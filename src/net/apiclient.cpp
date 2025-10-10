#include "apiclient.h"
#include "logging.h"
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimer>

ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::sslErrors,
            this, [this](QNetworkReply* r, const QList<QSslError>& errs){
                const auto u = r->request().url();
                if (u.host() == QLatin1String("127.0.0.1")) {
                    r->ignoreSslErrors();
                    emit sslIgnored(u);
                    LNET() << "SSL errors ignored for" << u.toString() << errs.size();
                }
            });
}

bool ApiClient::login(const QString& user, const QString& pass, QString* outTheme, QString* outError)
{
    if (outTheme) *outTheme = QString();
    if (outError) *outError = QString();

    const QUrl url = m_base.resolved(QUrl("/api/login"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
    ssl.setProtocol(QSsl::TlsV1_2OrLater);
    req.setSslConfiguration(ssl);

    QJsonObject payload{{"login", user}, {"password", pass}};
    QNetworkReply* rp = m_nam.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    QEventLoop loop;
    QTimer to; to.setInterval(3000); to.setSingleShot(true);
    QObject::connect(&to, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(rp, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    to.start(); loop.exec();

    if (rp->isRunning()) { rp->abort(); rp->deleteLater(); if(outError)*outError="timeout"; return false; }

    const auto err = rp->error();
    const QByteArray body = rp->readAll();
    rp->deleteLater();

    if (err != QNetworkReply::NoError) { if(outError)*outError="network"; return false; }

    QJsonParseError pe{};
    const auto jd = QJsonDocument::fromJson(body, &pe);
    if (pe.error != QJsonParseError::NoError || !jd.isObject()) { if(outError)*outError="bad json"; return false; }

    const auto obj = jd.object();
    if (!obj.value("ok").toBool(false)) { if(outError)*outError="auth"; return false; }
    if (outTheme) *outTheme = obj.value("themeName").toString("Dark");
    return true;
}
