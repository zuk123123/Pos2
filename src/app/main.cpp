#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "thememanager.h"
#include "db.h"

#include "apiclient.h"
#include "authbridge.h"
#include "loaderbridge.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 1) БД: открыть/создать и, при необходимости, накатить схему/сид
    if (!DbLayer::openOrCreate(nullptr))   // <-- ВАЖНО: без ""
        return 1;
    DbLayer::ensureSchemaAndSeed();

    // 2) Тема из локальной БД
    ThemeManager theme;
    const ThemeRow initial = DbLayer::readInitialTheme();
    theme.apply(initial);

    // 3) Сетевой слой и бриджи
    ApiClient api;                   // по умолчанию https://127.0.0.1:9443
    LoaderBridge loader(&api);
    AuthBridge auth(&theme, &api);   // конструктор: (ThemeManager*, ApiClient*, QObject*)

    // 4) QML
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("AppTheme"), &theme);
    engine.rootContext()->setContextProperty(QStringLiteral("LoginCtl"),  &auth);
    engine.rootContext()->setContextProperty(QStringLiteral("BootCtl"),   &loader);

    const QUrl url(u"qrc:/qml/Splash.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     },
                     Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
