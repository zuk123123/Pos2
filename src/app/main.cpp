#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "thememanager.h"
#include "apiclient.h"
#include "authbridge.h"
#include "loaderbridge.h"
#include "db.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Локальная БД (создать/открыть и накатить схему/сид)
    DbLayer::openOrCreate();
    DbLayer::ensureSchemaAndSeed();

    ThemeManager theme;
    ApiClient api;
    api.setBaseUrl(QUrl(QStringLiteral("https://127.0.0.1:9443")));

    // ВАЖНО: передаём и тему, и api в конструктор
    AuthBridge auth(&theme, &api);
    LoaderBridge loader(&api);

    QQmlApplicationEngine eng;
    eng.rootContext()->setContextProperty(QStringLiteral("AppTheme"),  &theme);
    eng.rootContext()->setContextProperty(QStringLiteral("LoginCtl"),  &auth);
    eng.rootContext()->setContextProperty(QStringLiteral("LoaderCtl"), &loader);

    const QUrl qmlUrl(QStringLiteral("qrc:/qml/Login.qml"));
    QObject::connect(&eng, &QQmlApplicationEngine::objectCreated,
                     &app, [qmlUrl](QObject *obj, const QUrl &objUrl) {
                         if (!obj && qmlUrl == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    eng.load(qmlUrl);
    return app.exec();
}
