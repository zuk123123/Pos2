#include "authbridge.h"
#include "thememanager.h"
#include "apiclient.h"
#include "db.h"
#include "logging.h"

AuthBridge::AuthBridge(ThemeManager* theme, ApiClient* api, QObject* parent)
    : QObject(parent), m_theme(theme), m_api(api)
{}

bool AuthBridge::login(const QString& user, const QString& pass)
{
    LNET().noquote() << "[Auth] login() user=" << user << " len(pass)=" << pass.size();
    setError(QString());
    setBusy(true);

    QString theme, err;
    const bool ok = m_api && m_api->login(user, pass, &theme, &err);

    if (!ok) {
        if (err == "auth") setError("Неверный логин или пароль");
        else setError("Ошибка сети");
        setBusy(false);

        if (auto cached = DbLayer::readLocalTheme(user)) {
            if (m_theme) m_theme->applyByName(*cached);
            emit loginSuccess();
            return true; // оффлайн вход по теме
        }
        return false;
    }

    if (m_theme) m_theme->applyByName(theme);
    DbLayer::upsertLocalTheme(user, theme);
    setBusy(false);
    emit loginSuccess();
    return true;
}

