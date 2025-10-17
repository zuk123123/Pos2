#pragma once
#include <QObject>
#include <QString>

class ThemeManager;
class ApiClient;

class AuthBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)
public:
    explicit AuthBridge(ThemeManager* theme, ApiClient* api, QObject* parent=nullptr);
    Q_INVOKABLE bool login(const QString& user, const QString& pass);
    bool busy() const { return m_busy; }
    QString errorText() const { return m_error; }
signals:
    void busyChanged();
    void errorTextChanged();
    void loginSuccess();
private:
    void setBusy(bool b) { if (m_busy==b) return; m_busy=b; emit busyChanged(); }
    void setError(const QString& e) { if (m_error==e) return; m_error=e; emit errorTextChanged(); }
    ThemeManager* m_theme = nullptr;
    ApiClient*    m_api   = nullptr;
    bool m_busy = false;
    QString m_error;
};

