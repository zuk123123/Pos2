#pragma once
#include <QObject>
#include <QStringList>
#include <QJsonObject>
#include "thememanager.h"

class ApiClient;

class SettingsBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList themeNames READ themeNames NOTIFY changed)
    Q_PROPERTY(QString currentUser READ currentUser WRITE setCurrentUser NOTIFY changed)
public:
    explicit SettingsBridge(ThemeManager* theme, ApiClient* api, QObject* parent=nullptr);

    QStringList themeNames() const { return m_themeNames; }
    QString currentUser() const { return m_user; }
    void setCurrentUser(const QString& u);

    Q_INVOKABLE bool refreshThemes();
    Q_INVOKABLE bool saveUserTheme(const QString& themeName); // локально + сервер
    Q_INVOKABLE bool createOrUpdateTheme(const QString& name,
                                         const QString& primary,
                                         const QString& background,
                                         const QString& text,
                                         const QString& border,
                                         const QString& panelBg);

signals:
    void changed();
    void error(QString message);

private:
    bool ensureThemeExists(const QString& name);
    ThemeManager* m_themeMgr = nullptr;
    ApiClient*    m_api      = nullptr;
    QStringList   m_themeNames;
    QString       m_user;
};

