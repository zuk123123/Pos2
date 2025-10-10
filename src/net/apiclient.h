#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

class ApiClient : public QObject {
    Q_OBJECT
public:
    explicit ApiClient(QObject* parent=nullptr);

    void setBaseUrl(const QUrl& url) { m_base = url; }
    QUrl  baseUrl() const { return m_base; }

    // sync login with small event loop (as было)
    bool login(const QString& user, const QString& pass, QString* outTheme, QString* outError);

signals:
    void sslIgnored(const QUrl&);

private:
    QNetworkAccessManager m_nam;
    QUrl m_base; // e.g. https://127.0.0.1:9443
};
