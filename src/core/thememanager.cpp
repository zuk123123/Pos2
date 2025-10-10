#include "thememanager.h"
#include "logging.h"

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    DbLayer::openOrCreate();
    DbLayer::ensureSchemaAndSeed();
    m_t = DbLayer::readInitialTheme();
}

void ThemeManager::applyByName(const QString& name)
{
    if (name == m_t.name) return;
    auto maybe = DbLayer::readThemeByName(name);
    if (!maybe) {
        LWARN() << "theme not found:" << name;
        return;
    }
    m_t = *maybe;
    emit changed();
}
