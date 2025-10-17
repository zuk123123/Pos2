#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QUrl>
#include <QStringList>

class ApiClient;

class LoaderBridge : public QObject {
    Q_OBJECT
public:
    explicit LoaderBridge(ApiClient* api, QObject* parent=nullptr);

    Q_INVOKABLE void ping();                // вызывает подбор рабочей базы
    QUrl currentBase() const { return m_apiBase; }

signals:
    void finished(bool ok);                 // готово: найден рабочий бэк или нет

private:
    void buildCandidates(QStringList& out); // формирование списка адресов
    void tryNext();                         // идём последовательно по кандидатам
    void pingBase(const QUrl& base);        // GET /ping
    QNetworkRequest makeRequest(const QString& path, const QUrl& base) const;
    void maybeFinish();                     // эмитит finished, когда готовы и таймер истёк

private:
    ApiClient* m_api = nullptr;
    QNetworkAccessManager m_nam;

    QStringList m_candidates;
    int   m_idx = -1;

    QTimer m_minDelay;          // минимальная «красота» задержки сплеша
    bool   m_minPassed = false;
    bool   m_hasReply  = false;
    bool   m_ok        = false;

    int    m_timeoutMs = 3000;

    QUrl   m_apiBase;
};
