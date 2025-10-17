#include "db.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QVariant>
#include <QDebug>
#include "thememanager.h"

static QSqlDatabase& db()
{
    static bool inited = false;
    static QSqlDatabase d = QSqlDatabase::addDatabase("QSQLITE");
    if (!inited) {
        inited = true;
    }
    return d;
}

namespace DbLayer {

bool openOrCreate(const QString& sqlitePath)
{
    auto& d = db();
    if (!d.isValid()) return false;

    if (d.databaseName().isEmpty()) {
        d.setDatabaseName(sqlitePath);
        qInfo() << "[DB] databaseName =" << d.databaseName();
    }

    if (!d.isOpen()) {
        if (!d.open()) {
            qCritical() << "[DB] open failed:" << d.lastError().text();
            return false;
        }
    }
    return true;
}

static bool execOrWarn(QSqlQuery& q, const char* where)
{
    if (!q.exec()) {
        qWarning() << "[DB]" << where << "failed:" << q.lastError().text();
        return false;
    }
    return true;
}

bool ensureSchemaAndSeed()
{
    QSqlQuery q(db());

    // users
    q.prepare(R"SQL(
        CREATE TABLE IF NOT EXISTS users (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            login     TEXT    NOT NULL UNIQUE,
            server_id INTEGER
        )
    )SQL");
    if (!execOrWarn(q, "create users")) return false;

    // user_settings
    q.prepare(R"SQL(
        CREATE TABLE IF NOT EXISTS user_settings (
            user_id INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
            theme   TEXT NOT NULL REFERENCES themes(name)
        )
    )SQL");
    if (!execOrWarn(q, "create user_settings")) return false;

    // themes
    q.prepare(R"SQL(
        CREATE TABLE IF NOT EXISTS themes(
            name           TEXT    PRIMARY KEY,
            screen_width   INTEGER NOT NULL DEFAULT 1280,
            screen_height  INTEGER NOT NULL DEFAULT 800,
            ui_scale       REAL    NOT NULL DEFAULT 1.0,
            font_small     INTEGER NOT NULL DEFAULT 12,
            font_base      INTEGER NOT NULL DEFAULT 14,
            font_large     INTEGER NOT NULL DEFAULT 18,
            spacing        INTEGER NOT NULL DEFAULT 8,
            radius         INTEGER NOT NULL DEFAULT 8,
            padding        INTEGER NOT NULL DEFAULT 10,
            primary_color  TEXT    NOT NULL DEFAULT '#2563EB',
            background     TEXT    NOT NULL DEFAULT '#111827',
            text_color     TEXT    NOT NULL DEFAULT '#F9FAFB',
            border_color   TEXT    NOT NULL DEFAULT '#333333',
            input_bg       TEXT    NOT NULL DEFAULT '#1F2937',
            button_bg      TEXT    NOT NULL DEFAULT '#374151',
            button_text    TEXT    NOT NULL DEFAULT '#F9FAFB'
        )
    )SQL");
    if (!execOrWarn(q, "create themes")) return false;

    // app_info
    q.prepare(R"SQL(
        CREATE TABLE IF NOT EXISTS app_info(
            key   TEXT PRIMARY KEY,
            value TEXT NOT NULL
        )
    )SQL");
    if (!execOrWarn(q, "create app_info")) return false;

    // seed themes
    q.prepare(R"SQL(
        INSERT OR IGNORE INTO themes
            (name, screen_width, screen_height, ui_scale, font_small, font_base, font_large,
             spacing, radius, padding, primary_color, background, text_color, border_color,
             input_bg, button_bg, button_text)
        VALUES
            ('Dark', 1280, 800, 1.0, 12,14,18, 8,8,10,
             '#3B82F6','#111827','#F9FAFB','#333333','#1F2937','#374151','#F9FAFB'),
            ('Light', 1280, 800, 1.0, 12,14,18, 8,8,10,
             '#2563EB','#F3F4F6','#111827','#D1D5DB','#FFFFFF','#3B82F6','#FFFFFF')
    )SQL");
    if (!execOrWarn(q, "seed themes")) return false;

    qInfo() << "[DB] schema ok; themes seeded";
    return true;
}

static ThemeRow fromRecord(const QSqlRecord& r)
{
    ThemeRow t;
    t.name           = r.value("name").toString();
    t.screen_width   = r.value("screen_width").toInt();
    t.screen_height  = r.value("screen_height").toInt();
    t.ui_scale       = r.value("ui_scale").toDouble();
    t.font_small     = r.value("font_small").toInt();
    t.font_base      = r.value("font_base").toInt();
    t.font_large     = r.value("font_large").toInt();
    t.spacing        = r.value("spacing").toInt();
    t.radius         = r.value("radius").toInt();
    t.padding        = r.value("padding").toInt();
    t.primary_color  = r.value("primary_color").toString();
    t.background     = r.value("background").toString();
    t.text_color     = r.value("text_color").toString();
    t.border_color   = r.value("border_color").toString();
    t.input_bg       = r.value("input_bg").toString();
    t.button_bg      = r.value("button_bg").toString();
    t.button_text    = r.value("button_text").toString();
    return t;
}

bool readThemeRowByName(const QString& name, ThemeRow* out)
{
    if (!out) return false;
    QSqlQuery q(db());
    q.prepare("SELECT * FROM themes WHERE name = :name");
    q.bindValue(":name", name);
    if (!q.exec()) return false;
    if (!q.next()) return false;
    *out = fromRecord(q.record());
    return true;
}

void writeThemeByName(const QString& name)
{
    QSqlQuery q(db());
    q.prepare("INSERT OR IGNORE INTO themes(name) VALUES(?)");
    q.addBindValue(name);
    q.exec();
}

// НОВОЕ: список имён тем
QStringList listThemeNames()
{
    QStringList out;
    QSqlQuery q(db());
    if (!q.exec("SELECT name FROM themes ORDER BY name COLLATE NOCASE ASC")) {
        qWarning() << "[DB] listThemeNames failed:" << q.lastError().text();
        return out;
    }
    while (q.next())
        out << q.value(0).toString();
    return out;
}

// НОВОЕ: upsert темы целиком
bool upsertTheme(const ThemeRow& t)
{
    QSqlQuery q(db());
    q.prepare(R"SQL(
        INSERT INTO themes
          (name, screen_width, screen_height, ui_scale,
           font_small, font_base, font_large,
           spacing, radius, padding,
           primary_color, background, text_color, border_color,
           input_bg, button_bg, button_text)
        VALUES
          (:name, :sw, :sh, :scale,
           :fs, :fb, :fl,
           :sp, :rd, :pd,
           :pc, :bg, :txt, :brd,
           :inbg, :btnbg, :btntxt)
        ON CONFLICT(name) DO UPDATE SET
           screen_width  = excluded.screen_width,
           screen_height = excluded.screen_height,
           ui_scale      = excluded.ui_scale,
           font_small    = excluded.font_small,
           font_base     = excluded.font_base,
           font_large    = excluded.font_large,
           spacing       = excluded.spacing,
           radius        = excluded.radius,
           padding       = excluded.padding,
           primary_color = excluded.primary_color,
           background    = excluded.background,
           text_color    = excluded.text_color,
           border_color  = excluded.border_color,
           input_bg      = excluded.input_bg,
           button_bg     = excluded.button_bg,
           button_text   = excluded.button_text
    )SQL");

    q.bindValue(":name",  t.name);
    q.bindValue(":sw",    t.screen_width);
    q.bindValue(":sh",    t.screen_height);
    q.bindValue(":scale", t.ui_scale);
    q.bindValue(":fs",    t.font_small);
    q.bindValue(":fb",    t.font_base);
    q.bindValue(":fl",    t.font_large);
    q.bindValue(":sp",    t.spacing);
    q.bindValue(":rd",    t.radius);
    q.bindValue(":pd",    t.padding);
    q.bindValue(":pc",    t.primary_color);
    q.bindValue(":bg",    t.background);
    q.bindValue(":txt",   t.text_color);
    q.bindValue(":brd",   t.border_color);
    q.bindValue(":inbg",  t.input_bg);
    q.bindValue(":btnbg", t.button_bg);
    q.bindValue(":btntxt",t.button_text);

    if (!q.exec()) {
        qWarning() << "[DB] upsertTheme failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// --- Пользователи / настройки ---

int upsertUserLogin(const QString& login)
{
    {   QSqlQuery q(db());
        q.prepare("INSERT OR IGNORE INTO users(login) VALUES(?)");
        q.addBindValue(login);
        q.exec();
    }
    QSqlQuery q(db());
    q.prepare("SELECT id FROM users WHERE login=?");
    q.addBindValue(login);
    if (!q.exec() || !q.next()) return -1;
    return q.value(0).toInt();
}

std::optional<int> getUserIdByLogin(const QString& login)
{
    QSqlQuery q(db());
    q.prepare("SELECT id FROM users WHERE login=?");
    q.addBindValue(login);
    if (!q.exec() || !q.next()) return std::nullopt;
    return q.value(0).toInt();
}

std::optional<QString> getUserTheme(int userId)
{
    QSqlQuery q(db());
    q.prepare("SELECT theme FROM user_settings WHERE user_id=?");
    q.addBindValue(userId);
    if (!q.exec() || !q.next()) return std::nullopt;
    return q.value(0).toString();
}

bool setUserTheme(int userId, const QString& themeName)
{
    QSqlQuery q(db());
    q.prepare(R"SQL(
        INSERT INTO user_settings(user_id, theme)
        VALUES(:uid, :t)
        ON CONFLICT(user_id) DO UPDATE SET theme=excluded.theme
    )SQL");
    q.bindValue(":uid", userId);
    q.bindValue(":t", themeName);
    return q.exec();
}

// Старые совместимые функции (login -> theme)
std::optional<QString> readLocalTheme(const QString& login)
{
    if (auto uid = getUserIdByLogin(login)) {
        return getUserTheme(*uid);
    }
    return std::nullopt;
}

void upsertLocalTheme(const QString& login, const QString& theme_name)
{
    const int uid = upsertUserLogin(login);
    if (uid > 0) (void)setUserTheme(uid, theme_name);
}

ThemeRow readInitialTheme()
{
    ThemeRow t;
    if (!readThemeRowByName(QStringLiteral("Dark"), &t)) {
        t.name = QStringLiteral("Dark");
        t.background = "#111827";
        t.text_color = "#F9FAFB";
        t.border_color = "#333333";
        t.input_bg = "#1F2937";
        t.button_bg = "#374151";
        t.button_text = "#F9FAFB";
        t.primary_color = "#3B82F6";
        t.ui_scale = 1.0;
        t.font_small = 12; t.font_base = 14; t.font_large = 18;
        t.spacing = 8; t.radius = 8; t.padding = 10;
        t.screen_width = 1280; t.screen_height = 800;
    }
    return t;
}

void upsertScreenInfo(int width, int height, double dpi, double dpr, double scale)
{
    auto put = [&](const char* key, const QString& val) {
        QSqlQuery q2(db());
        q2.prepare("INSERT INTO app_info(key,value) VALUES(?,?) "
                   "ON CONFLICT(key) DO UPDATE SET value=excluded.value");
        q2.addBindValue(QString::fromUtf8(key));
        q2.addBindValue(val);
        if (!q2.exec())
            qWarning() << "[DB] upsert app_info failed:" << q2.lastError().text();
    };
    put("screen.width",  QString::number(width));
    put("screen.height", QString::number(height));
    put("screen.dpi",    QString::number(dpi, 'f', 2));
    put("screen.dpr",    QString::number(dpr, 'f', 2));
    put("screen.scale",  QString::number(scale, 'f', 3));
}

} // namespace DbLayer
