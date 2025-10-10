#include "db.h"
#include "logging.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

Q_LOGGING_CATEGORY(logCore, "pos.core")
Q_LOGGING_CATEGORY(logNet,  "pos.net")

static const char* kConn = "pos2_sqlite";

static QString appDataDir()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base;
}

static QString dbPath()
{
    return appDataDir() + "/localdb.sqlite";
}

static ThemeRow rowFromQuery(const QSqlQuery& q)
{
    ThemeRow t;
    t.name         = q.value(0).toString();
    t.screenWidth  = q.value(1).toInt();
    t.screenHeight = q.value(2).toInt();
    t.uiScale      = q.value(3).toDouble();
    t.fontSmall    = q.value(4).toInt();
    t.fontBase     = q.value(5).toInt();
    t.fontLarge    = q.value(6).toInt();
    t.spacing      = q.value(7).toInt();
    t.radius       = q.value(8).toInt();
    t.padding      = q.value(9).toInt();
    t.primaryColor = q.value(10).toString();
    t.background   = q.value(11).toString();
    t.text         = q.value(12).toString();
    t.border       = q.value(13).toString();
    t.inputBg      = q.value(14).toString();
    t.buttonBg     = q.value(15).toString();
    t.buttonText   = q.value(16).toString();
    return t;
}

bool DbLayer::openOrCreate(QString* outPath)
{
    const QString path = dbPath();
    if (outPath) *outPath = path;

    // первый запуск: положим дефолтную БД из ресурсов (если она зашита в qrc)
    if (!QFileInfo::exists(path)) {
        const QString res = QStringLiteral(":/db/localdb.sqlite");
        if (QFile::exists(res)) {
            if (!QFile::copy(res, path)) {
                LERR() << "failed to copy seed DB from resource to" << path;
            } else {
                QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner);
                LCORE().noquote() << "[INFO] DB seeded from resource to" << path;
            }
        }
    }

    if (QSqlDatabase::contains(kConn))
        return true;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", kConn);
    db.setDatabaseName(path);
    if (!db.open()) {
        LERR() << "SQLite open failed:" << db.lastError().text();
        return false;
    }
    LCORE().noquote() << "[INFO] DB opened at" << path;
    return true;
}

bool DbLayer::ensureSchemaAndSeed()
{
    QSqlDatabase db = QSqlDatabase::database(kConn);
    if (!db.isOpen()) return false;

    QSqlQuery q(db);

    // themes
    if (!q.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS themes(
          name TEXT PRIMARY KEY,
          screen_width  INTEGER DEFAULT 0,
          screen_height INTEGER DEFAULT 0,
          ui_scale REAL DEFAULT 1.0,
          font_small INTEGER, font_base INTEGER, font_large INTEGER,
          spacing INTEGER, radius INTEGER, padding INTEGER,
          primary_color TEXT, background TEXT, text_color TEXT,
          border_color TEXT, input_bg TEXT, button_bg TEXT, button_text TEXT
        );
    )SQL")) {
        LERR() << "create themes:" << q.lastError().text();
        return false;
    }

    // local_prefs
    if (!q.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS local_prefs(
          login TEXT PRIMARY KEY,
          theme_name TEXT
        );
    )SQL")) {
        LERR() << "create local_prefs:" << q.lastError().text();
        return false;
    }

    // seed two themes if empty
    if (!q.exec("SELECT COUNT(*) FROM themes")) return false;
    int cnt = 0; if (q.next()) cnt = q.value(0).toInt();
    if (cnt == 0) {
        QSqlQuery ins(db);
        ins.prepare(R"SQL(
            INSERT INTO themes(
                name, ui_scale, font_small, font_base, font_large,
                spacing, radius, padding,
                primary_color, background, text_color, border_color,
                input_bg, button_bg, button_text
            )
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )SQL");

        auto add = [&](const ThemeRow& t){
            ins.addBindValue(t.name);
            ins.addBindValue(t.uiScale);
            ins.addBindValue(t.fontSmall);
            ins.addBindValue(t.fontBase);
            ins.addBindValue(t.fontLarge);
            ins.addBindValue(t.spacing);
            ins.addBindValue(t.radius);
            ins.addBindValue(t.padding);
            ins.addBindValue(t.primaryColor);
            ins.addBindValue(t.background);
            ins.addBindValue(t.text);
            ins.addBindValue(t.border);
            ins.addBindValue(t.inputBg);
            ins.addBindValue(t.buttonBg);
            ins.addBindValue(t.buttonText);
            ins.exec();
        };

        add(ThemeRow{
            .name="Dark", .uiScale=1.0, .fontSmall=12, .fontBase=14, .fontLarge=18,
            .spacing=8, .radius=8, .padding=10,
            .primaryColor="#3B82F6", .background="#111827", .text="#F9FAFB",
            .border="#333333", .inputBg="#1F2937", .buttonBg="#2563EB", .buttonText="#FFFFFF"
        });
        add(ThemeRow{
            .name="Light", .uiScale=1.0, .fontSmall=12, .fontBase=14, .fontLarge=18,
            .spacing=8, .radius=8, .padding=10,
            .primaryColor="#2563EB", .background="#FFFFFF", .text="#111827",
            .border="#D1D5DB", .inputBg="#F3F4F6", .buttonBg="#2563EB", .buttonText="#FFFFFF"
        });

        LCORE() << "seeded themes: 2";
    }
    return true;
}

std::optional<ThemeRow> DbLayer::readThemeByName(const QString& name)
{
    QSqlDatabase db = QSqlDatabase::database(kConn);
    if (!db.isOpen()) return std::nullopt;

    QSqlQuery q(db);
    q.prepare(R"SQL(
        SELECT name, screen_width, screen_height, ui_scale,
               font_small, font_base, font_large, spacing, radius, padding,
               primary_color, background, text_color, border_color, input_bg, button_bg, button_text
        FROM themes WHERE name = ? LIMIT 1
    )SQL");
    q.addBindValue(name);
    if (q.exec() && q.next()) return rowFromQuery(q);
    return std::nullopt;
}

ThemeRow DbLayer::readInitialTheme()
{
    return readThemeByName("Dark").value_or(ThemeRow{.name="Dark"});
}

bool DbLayer::updateThemeScreen(const QString& themeName, int w, int h)
{
    QSqlDatabase db = QSqlDatabase::database(kConn);
    if (!db.isOpen()) return false;
    QSqlQuery q(db);
    q.prepare("UPDATE themes SET screen_width=?, screen_height=? WHERE name=?");
    q.addBindValue(w);
    q.addBindValue(h);
    q.addBindValue(themeName);
    return q.exec();
}

bool DbLayer::upsertLocalTheme(const QString& login, const QString& themeName)
{
    QSqlDatabase db = QSqlDatabase::database(kConn);
    if (!db.isOpen()) return false;
    QSqlQuery q(db);
    q.prepare(R"SQL(
        INSERT INTO local_prefs(login, theme_name) VALUES(?, ?)
        ON CONFLICT(login) DO UPDATE SET theme_name=excluded.theme_name
    )SQL");
    q.addBindValue(login);
    q.addBindValue(themeName);
    return q.exec();
}

std::optional<QString> DbLayer::readLocalTheme(const QString& login)
{
    QSqlDatabase db = QSqlDatabase::database(kConn);
    if (!db.isOpen()) return std::nullopt;
    QSqlQuery q(db);
    q.prepare("SELECT theme_name FROM local_prefs WHERE login=? LIMIT 1");
    q.addBindValue(login);
    if (q.exec() && q.next()) return q.value(0).toString();
    return std::nullopt;
}
