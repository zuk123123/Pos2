#include "apiclient.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QSslConfiguration>

ApiClient::ApiClient(QObject *parent) : QObject(parent) {}

void ApiClient::addAuth(QNetworkRequest &req) {
    if (!m_jwt.isEmpty())
        req.setRawHeader("Authorization", QByteArray("Bearer ").append(m_jwt.toUtf8()));
}

void ApiClient::ping() {
    const QUrl url = m_base.resolved(QUrl("/ping"));
    qInfo() << "[API] GET" << url;

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "Pos2/1.0 (+Qt)");
    if (url.scheme().compare("https", Qt::CaseInsensitive) == 0) {
        QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ssl.setProtocol(QSsl::TlsV1_2OrLater);
#endif
        req.setSslConfiguration(ssl);
    }
    auto *reply = m_nam.get(req);

    connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError>& errs){
        for (const auto &e : errs) qWarning() << "[SSL]" << e.errorString();
    });
    connect(reply, &QNetworkReply::errorOccurred, this, [reply,this](QNetworkReply::NetworkError e){
        qWarning() << "[API][ERR]" << e << reply->errorString();
        emit networkError(reply->errorString());
    });
    connect(reply, &QNetworkReply::finished, this, [reply,this]{
        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        qInfo() << "[API][RESP]" << code << body.left(256);
        if (reply->error() == QNetworkReply::NoError && code>=200 && code<300)
            emit pingOk();
        reply->deleteLater();
    });
}

// login — как раньше, но мы теперь вытаскиваем token и кладём в m_jwt
static QList<QByteArray> s_loginPaths() { return { "/api/login" }; }

bool ApiClient::login(const QString &user, const QString &pass,
                      QString *themeOut, QString *errOut)
{
    if (errOut) errOut->clear();
    if (themeOut) themeOut->clear();

    QJsonObject payload{{"login", user}, {"password", pass}};
    const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkRequest req;
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setHeader(QNetworkRequest::UserAgentHeader, "Pos2/1.0 (+Qt)");

    auto doPost = [&](const QUrl &url)->std::pair<int,QByteArray> {
        QEventLoop loop;
        QTimer timer; timer.setSingleShot(true); timer.setInterval(10000);

        if (url.scheme().compare("https", Qt::CaseInsensitive) == 0) {
            QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            ssl.setProtocol(QSsl::TlsV1_2OrLater);
#endif
            req.setSslConfiguration(ssl);
        }
        req.setUrl(url);
        QNetworkReply *reply = m_nam.post(req, data);

        connect(&timer, &QTimer::timeout, &loop, [&]{ if (reply) reply->abort(); loop.quit(); });
        connect(reply, &QNetworkReply::finished, &loop, [&]{ loop.quit(); });

        timer.start();
        loop.exec();

        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const auto err = reply->error();
        reply->deleteLater();

        if (err != QNetworkReply::NoError) {
            qWarning() << "[API][login][ERR]" << url << "HTTP" << code << body.left(256);
            return {code?code:-1, body};
        }
        qInfo() << "[API][login][RESP]" << url << "HTTP" << code << body.left(256);
        return {code, body};
    };

    for (const auto &p : s_loginPaths()) {
        const QUrl url = m_base.resolved(QUrl(QString::fromUtf8(p)));
        auto [code, body] = doPost(url);
        if (code >= 200 && code < 300) {
            QJsonParseError jerr{};
            QJsonDocument doc = QJsonDocument::fromJson(body, &jerr);
            if (jerr.error == QJsonParseError::NoError && doc.isObject()) {
                const QJsonObject o = doc.object();
                const QString theme = o.value(QStringLiteral("themeName")).toString();
                const QString token = o.value(QStringLiteral("token")).toString();
                if (!token.isEmpty()) m_jwt = token;
                if (themeOut && !theme.isEmpty()) *themeOut = theme;
                const bool ok = o.value(QStringLiteral("ok")).toBool(true);
                if (ok) return true;
            }
            // если сервер вернул 2xx, но не JSON — пусть будет фейл
        }
    }
    if (errOut) *errOut = QStringLiteral("auth");
    return false;
}

bool ApiClient::setUserThemeOnServer(const QString &themeName, QString *errOut)
{
    if (errOut) *errOut = QString();
    const QUrl url = m_base.resolved(QUrl("/api/me/theme"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setHeader(QNetworkRequest::UserAgentHeader, "Pos2/1.0 (+Qt)");
    addAuth(req);

    const QJsonObject payload{{"themeName", themeName}};
    const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QEventLoop loop;
    QNetworkReply *reply = m_nam.put(req, data);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const auto err = reply->error();
    reply->deleteLater();

    if (err != QNetworkReply::NoError || code < 200 || code >= 300) {
        if (errOut) *errOut = QString::fromUtf8(body);
        qWarning() << "[API] setUserThemeOnServer fail" << code << body.left(200);
        return false;
    }
    qInfo() << "[API] setUserThemeOnServer OK";
    return true;
}

bool ApiClient::upsertThemeOnServer(const QJsonObject &themeDef, QString *errOut)
{
    if (errOut) *errOut = QString();
    const QUrl url = m_base.resolved(QUrl("/api/themes"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setHeader(QNetworkRequest::UserAgentHeader, "Pos2/1.0 (+Qt)");
    addAuth(req);

    const QByteArray data = QJsonDocument(themeDef).toJson(QJsonDocument::Compact);

    QEventLoop loop;
    QNetworkReply *reply = m_nam.post(req, data);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const auto err = reply->error();
    reply->deleteLater();

    if (err != QNetworkReply::NoError || code < 200 || code >= 300) {
        if (errOut) *errOut = QString::fromUtf8(body);
        qWarning() << "[API] upsertThemeOnServer fail" << code << body.left(200);
        return false;
    }
    qInfo() << "[API] upsertThemeOnServer OK";
    return true;
}
