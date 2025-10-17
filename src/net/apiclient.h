#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

class ApiClient : public QObject {
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);
    void setBaseUrl(const QUrl &url) { m_base = url; }
    Q_INVOKABLE void ping();

    // login: парсит token и сохраняет его внутри клиента
    bool login(const QString &user, const QString &pass,
               QString *themeOut, QString *errOut);

    // серверные настройки темы (требуют JWT внутри)
    bool setUserThemeOnServer(const QString &themeName, QString *errOut = nullptr);
    bool upsertThemeOnServer(const QJsonObject &themeDef, QString *errOut = nullptr);

signals:
    void networkError(QString);
    void pingOk();

private:
    QNetworkAccessManager m_nam;
    QUrl m_base;
    QString m_jwt; // <- хранится тут

    void addAuth(QNetworkRequest &req);
};
