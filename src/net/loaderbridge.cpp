#include "loaderbridge.h"
#include "apiclient.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QTimer>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

#ifndef POS2_API_BASE_DEFAULT
#  define POS2_API_BASE_DEFAULT "http://127.0.0.1:8080"
#endif

LoaderBridge::LoaderBridge(ApiClient* api, QObject* parent)
    : QObject(parent), m_api(api)
{
    m_minDelay.setInterval(800);
    m_minDelay.setSingleShot(true);
    connect(&m_minDelay, &QTimer::timeout, this, [this](){
        qInfo() << "[Boot] min delay passed";
        m_minPassed = true;
        maybeFinish();
    });
}

void LoaderBridge::buildCandidates(QStringList& out)
{
    // Ровно один адрес (никаких fallback/localhost)
    out.clear();
    out << QString::fromUtf8(POS2_API_BASE_DEFAULT);
    out.removeDuplicates();

    qInfo() << "[Boot] candidates =" << out;
}

void LoaderBridge::ping()
{
    m_minPassed = false;
    m_hasReply  = false;
    m_ok        = false;

    m_candidates.clear();
    buildCandidates(m_candidates);
    m_idx = -1;

    m_minDelay.start();
    tryNext();
}

void LoaderBridge::tryNext()
{
    m_idx++;
    if (m_idx >= m_candidates.size()) {
        qWarning() << "[Boot] no candidates worked";
        m_hasReply = true; m_ok = false; maybeFinish(); return;
    }
    const QUrl base(m_candidates.at(m_idx));
    qInfo() << "[Boot] try" << base.toString();
    pingBase(base);
}

void LoaderBridge::pingBase(const QUrl& base)
{
    const QNetworkRequest req = makeRequest("/ping", base);
    QNetworkReply* reply = m_nam.get(req);
    if (!reply) {
        qWarning() << "[Boot] cannot start request";
        tryNext();
        return;
    }

    QTimer* to = new QTimer(this);
    to->setSingleShot(true);
    to->setInterval(m_timeoutMs);
    connect(to, &QTimer::timeout, this, [reply](){
        if (reply->isRunning()) reply->abort();
    });
    to->start();

    connect(reply, &QNetworkReply::finished, this, [=](){
        const auto err = reply->error();
        const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool ok = (err == QNetworkReply::NoError) && http>=200 && http<300;

        if (ok) {
            qInfo() << "[Boot] OK at" << reply->url().toString() << "status" << http;
            m_apiBase = reply->url().adjusted(QUrl::RemovePath | QUrl::RemoveQuery | QUrl::StripTrailingSlash);
            if (m_api) m_api->setBaseUrl(m_apiBase);
            reply->deleteLater(); to->deleteLater();
            m_hasReply = true; m_ok = true; maybeFinish();
        } else {
            qWarning() << "[Boot] FAIL at" << reply->url().toString()
            << "status" << http << "err" << err << reply->errorString();
            reply->deleteLater(); to->deleteLater();
            tryNext();
        }
    });

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(reply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError e){
        qWarning() << "[Boot] errorOccurred:" << e << reply->errorString();
    });
#endif
}

QNetworkRequest LoaderBridge::makeRequest(const QString& path, const QUrl& base) const
{
    QUrl url = base.resolved(QUrl(path));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Pos2/1.0"));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    // HTTPS-настройки оставляем «по умолчанию»; для HTTP ничего не делаем
    if (url.scheme().compare("https", Qt::CaseInsensitive) == 0) {
        QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ssl.setProtocol(QSsl::TlsV1_2OrLater);
#endif
        req.setSslConfiguration(ssl);
    }
    return req;
}

void LoaderBridge::maybeFinish()
{
    if (m_minPassed && m_hasReply) {
        qInfo() << "[Boot] emit finished:" << m_ok << "base=" << m_apiBase.toString();
        emit finished(m_ok);
    } else {
        qInfo() << "[Boot] wait... minPassed=" << m_minPassed << "hasReply=" << m_hasReply;
    }
}
