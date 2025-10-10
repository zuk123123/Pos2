#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include "thememanager.h"
#include "db.h"
#include "apiclient.h"
#include "loaderbridge.h"
#include "authbridge.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 1) База: открыть/создать и убедиться, что есть схема/сид
    if (!DbLayer::openOrCreate()) {
        qCritical("[ERR] DB open failed");
        return 1;
    }
    if (!DbLayer::ensureSchemaAndSeed()) {
        qCritical("[ERR] DB schema/seed failed");
        return 1;
    }

    // 2) Тема: читаем «стартовую» из БД и применяем по имени
    ThemeManager theme;
    const ThemeRow initial = DbLayer::readInitialTheme();
    theme.applyByName(initial.name);   // <-- было theme.apply(initial);

    // 3) Сеть/бриджи
    ApiClient api;                      // базовый сетевой клиент
    LoaderBridge loader(&api);          // /ping
    AuthBridge auth(&theme, &api);      // /api/login — нужен ThemeManager и ApiClient

    // 4) QML
    QQmlApplicationEngine engine;

    // Экспорт C++ объектов в QML
    engine.rootContext()->setContextProperty("AppTheme", &theme);
    engine.rootContext()->setContextProperty("LoginCtl", &auth);
    engine.rootContext()->setContextProperty("LoaderCtl", &loader);

    const QUrl url(QStringLiteral("qrc:/qml/Splash.qml")); // без _qs -> убираем warning
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     },
                     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
