#pragma once
#include <QObject>
#include <QTimer>

class ApiClient;

class LoaderBridge : public QObject {
    Q_OBJECT
public:
    explicit LoaderBridge(ApiClient* api, QObject* parent=nullptr);
    Q_INVOKABLE void start();

signals:
    void finished(bool ok);

private:
    ApiClient* m_api = nullptr;
    QTimer     m_minDelay;
    bool m_hasReply=false, m_minPassed=false, m_ok=false;

    void maybeFinish();
};
