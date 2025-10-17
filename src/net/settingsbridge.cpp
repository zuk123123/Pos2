
#include "settingsbridge.h"
#include "db.h"
#include "apiclient.h"
#include <QDebug>

SettingsBridge::SettingsBridge(ThemeManager* theme, ApiClient* api, QObject* parent)
    : QObject(parent), m_themeMgr(theme), m_api(api)
{
    refreshThemes();
}

void SettingsBridge::setCurrentUser(const QString& u) {
    if (m_user == u) return;
    m_user = u;
    emit changed();
}

bool SettingsBridge::refreshThemes() {
    m_themeNames = DbLayer::listThemeNames();
    emit changed();
    qInfo() << "[Settings] themes:" << m_themeNames;
    return true;
}

bool SettingsBridge::saveUserTheme(const QString& themeName) {
    if (m_user.isEmpty()) { emit error("Нет текущего пользователя"); return false; }
    if (!ensureThemeExists(themeName)) return false;

    // локально
    DbLayer::upsertLocalTheme(m_user, themeName);

    // применить
    ThemeRow row;
    if (DbLayer::readThemeRowByName(themeName, &row) && m_themeMgr)
        m_themeMgr->applyRow(row);
    else if (m_themeMgr)
        m_themeMgr->applyByName(themeName);

    // сервер (если есть токен внутри ApiClient)
    QString apiErr;
    if (m_api && !m_api->setUserThemeOnServer(themeName, &apiErr)) {
        qWarning() << "[Settings] server save theme failed:" << apiErr;
        // не падаем — локально уже применили
    } else {
        qInfo() << "[Settings] server theme saved";
    }
    return true;
}

bool SettingsBridge::createOrUpdateTheme(const QString& name,
                                         const QString& primary,
                                         const QString& background,
                                         const QString& text,
                                         const QString& border,
                                         const QString& panelBg)
{
    if (name.trimmed().isEmpty()) { emit error("Имя темы пусто"); return false; }
    ThemeRow t;
    t.name = name.trimmed();
    t.primary_color = primary;
    t.background    = background;
    t.text_color    = text;
    t.border_color  = border;
    t.input_bg      = panelBg;
    t.button_bg     = panelBg;
    t.button_text   = text;
    t.ui_scale      = 1.0;

    if (!DbLayer::upsertTheme(t)) {
        emit error("Не удалось сохранить тему локально");
        return false;
    }
    refreshThemes();

    // отправим на сервер
    if (m_api) {
        QJsonObject def{
            {"name", t.name},
            {"primary_color", t.primary_color},
            {"background", t.background},
            {"text_color", t.text_color},
            {"border_color", t.border_color},
            {"input_bg", t.input_bg},
            {"button_bg", t.button_bg},
            {"button_text", t.button_text},
            {"ui_scale", t.ui_scale},
            {"font_small", t.font_small},
            {"font_base", t.font_base},
            {"font_large", t.font_large},
            {"spacing", t.spacing},
            {"radius", t.radius},
            {"padding", t.padding}
        };
        QString err;
        if (!m_api->upsertThemeOnServer(def, &err)) {
            qWarning() << "[Settings] push theme to server failed:" << err;
        }
    }
    qInfo() << "[Settings] upsert custom theme:" << t.name;
    return true;
}

bool SettingsBridge::ensureThemeExists(const QString& name) {
    ThemeRow dummy;
    if (DbLayer::readThemeRowByName(name, &dummy)) return true;
    emit error(QStringLiteral("Тема '%1' отсутствует").arg(name));
    return false;
}
