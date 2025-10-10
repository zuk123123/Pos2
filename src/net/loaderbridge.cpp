#include "loaderbridge.h"
#include "apiclient.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslConfiguration>
#include <QUrl>
#include <QTimer>
#include "logging.h"

LoaderBridge::LoaderBridge(ApiClient* api, QObject* parent)
    : QObject(parent), m_api(api)
{
    m_minDelay.setInterval(800);
    m_minDelay.setSingleShot(true);
    connect(&m_minDelay, &QTimer::timeout, this, [this](){
        m_minPassed = true;
        LCORE() << "[Boot] min delay passed";
        maybeFinish();
    });
}

void LoaderBridge::start()
{
    LCORE() << "[Boot] start()";
    m_hasReply = m_minPassed = false; m_ok=false;
    m_minDelay.start();

    // ping через обычный GET
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkRequest req(m_api->baseUrl().resolved(QUrl("/ping")));
    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
    ssl.setProtocol(QSsl::TlsV1_2OrLater);
    req.setSslConfiguration(ssl);

    QNetworkReply* rp = nam->get(req);

    QTimer* to = new QTimer(rp); to->setInterval(1500); to->setSingleShot(true);
    connect(to, &QTimer::timeout, rp, [this, rp](){
        LCORE() << "[Boot] ping TIMEOUT";
        rp->abort(); m_ok=false; m_hasReply=true; maybeFinish();
    });
    to->start();

    connect(rp, &QNetworkReply::finished, this, [this, rp, to](){
        to->stop();
        m_ok = (rp->error() == QNetworkReply::NoError);
        LCORE() << "[Boot] ping finished, ok=" << m_ok;
        rp->deleteLater(); m_hasReply=true; maybeFinish();
    });
}

void LoaderBridge::maybeFinish()
{
    if (m_hasReply && m_minPassed) emit finished(m_ok);
}
