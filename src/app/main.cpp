#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QScreen>
#include <QDebug>

#include <QFontDatabase>
#include <QFont>
#include <QStandardPaths>
#include <QDir>

#include "thememanager.h"
#include "db.h"
#include "apiclient.h"
#include "loaderbridge.h"
#include "authbridge.h"

// Один адрес из CMake
#ifndef POS2_API_BASE_DEFAULT
#define POS2_API_BASE_DEFAULT "http://127.0.0.1:8080"
#endif

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QtCore/qnativeinterface.h>
static void forceLandscapeAndroid()
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid()) return;
    const jint sensorLandscape =
        QJniObject::getStaticField<jint>("android/content/pm/ActivityInfo",
                                         "SCREEN_ORIENTATION_SENSOR_LANDSCAPE");
    activity.callMethod<void>("setRequestedOrientation", "(I)V", sensorLandscape);
}
#endif

static void captureAndStoreScreenInfo()
{
    QScreen* s = QGuiApplication::primaryScreen();
    if (!s) { qWarning() << "[App] No primary screen"; return; }
    const QSize  sz   = s->size();
    const qreal  dpr  = s->devicePixelRatio();
    const qreal  dpi  = s->logicalDotsPerInch();
    const qreal  scale= dpi / 96.0;
    qInfo() << "[App] Screen:" << "size=" << sz << "dpi=" << dpi << "dpr=" << dpr << "scale=" << scale;
    DbLayer::upsertScreenInfo(sz.width(), sz.height(), dpi, dpr, scale);
}

static void loadUiFonts(QGuiApplication& app)
{
    int interId = QFontDatabase::addApplicationFont(":/fonts/Inter.ttf");
    QString interFamily;
    if (interId >= 0) {
        const auto fams = QFontDatabase::applicationFontFamilies(interId);
        if (!fams.isEmpty()) interFamily = fams.first();
    } else {
        qWarning() << "[Font] Inter.ttf missing";
    }
    QFontDatabase::addApplicationFont(":/fonts/Inter-Italic.ttf");

    int emojiId = QFontDatabase::addApplicationFont(":/fonts/NotoColorEmoji.ttf");
    QString emojiFamily;
    if (emojiId >= 0) {
        const auto fams = QFontDatabase::applicationFontFamilies(emojiId);
        if (!fams.isEmpty()) emojiFamily = fams.first();
    } else {
        qWarning() << "[Font] NotoColorEmoji.ttf missing";
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QStringList chain;
    if (!interFamily.isEmpty()) chain << interFamily;
    if (!emojiFamily.isEmpty()) chain << emojiFamily;
    if (!chain.isEmpty()) {
        QFont f = app.font();
        f.setFamilies(chain);
        app.setFont(f);
        qInfo() << "[Font] UI font chain =" << chain;
    }
#endif
}

int main(int argc, char *argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    QGuiApplication app(argc, argv);

#ifdef Q_OS_ANDROID
    forceLandscapeAndroid();
#endif

    loadUiFonts(app);

    // === БД путь (рядом с бинарём на desktop, AppDataLocation на Android) ===
    QString dbPath;
#ifdef Q_OS_ANDROID
    dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    dbPath += "/localdb.sqlite";
#else
    dbPath = QCoreApplication::applicationDirPath() + "/localdb.sqlite";
#endif

    if (!DbLayer::openOrCreate(dbPath))  { qCritical("[ERR] DB open failed"); return 1; }
    if (!DbLayer::ensureSchemaAndSeed()) { qCritical("[ERR] DB schema/seed failed"); return 1; }

    captureAndStoreScreenInfo();

    // Тема
    ThemeManager theme;
    const ThemeRow initial = DbLayer::readInitialTheme();
    theme.applyByName(initial.name);

    // Сеть / мосты
    ApiClient api;
    const QUrl base(QString::fromUtf8(POS2_API_BASE_DEFAULT));
    qInfo() << "[URL] base =" << base;
    api.setBaseUrl(base);
    QObject::connect(&api, &ApiClient::networkError, [](const QString &err){ qWarning() << "[API][NET-ERR]" << err; });
    QObject::connect(&api, &ApiClient::pingOk, [](){ qInfo() << "[API] ping OK"; });

    LoaderBridge loader(&api);
    AuthBridge   auth(&theme, &api);

    // QML
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("AppTheme",  &theme);
    engine.rootContext()->setContextProperty("LoginCtl",  &auth);
    engine.rootContext()->setContextProperty("LoaderCtl", &loader);
    engine.rootContext()->setContextProperty("Boot",      &loader);
    engine.rootContext()->setContextProperty("Api",       &api);
    engine.rootContext()->setContextProperty("LogPathPublic", QString());

    const QUrl url(QStringLiteral("qrc:/Splash.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl) QCoreApplication::exit(-1);
                     },
                     Qt::QueuedConnection);

    engine.load(url);

    // Первичный пинг
    api.ping();
    loader.ping();

    return app.exec();
}
