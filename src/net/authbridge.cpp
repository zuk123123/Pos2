#include <QDebug>
#include "authbridge.h"
#include "thememanager.h"
#include "apiclient.h"
#include "db.h"

AuthBridge::AuthBridge(ThemeManager* theme, ApiClient* api, QObject* parent)
    : QObject(parent), m_theme(theme), m_api(api)
{}

// authbridge.cpp (исправленный фрагмент)
bool AuthBridge::login(const QString& user, const QString& pass)
{
    qInfo() << "[Auth] login() user=" << user << " len(pass)=" << pass.size();
    setError(QString());
    setBusy(true);

    QString themeFromServer;
    QString err;
    const bool ok = m_api && m_api->login(user, pass, &themeFromServer, &err);

    setBusy(false);

    if (!ok) {
        // Строго: при неуспехе НИКУДА не пускаем
        if (err == "auth") setError(QStringLiteral("Неверный логин или пароль"));
        else               setError(QStringLiteral("Ошибка сети"));
        return false;
    }

    // Успешный логин → применяем тему и пускаем внутрь
    DbLayer::upsertLocalTheme(user, themeFromServer);
    if (m_theme) {
        ThemeRow row;
        if (DbLayer::readThemeRowByName(themeFromServer, &row))
            m_theme->applyRow(row);
        else
            m_theme->applyByName(themeFromServer);
    }
    emit loginSuccess();
    return true;
}


