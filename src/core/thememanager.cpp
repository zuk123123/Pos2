#include "thememanager.h"
#include <QtSql>

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{}

void ThemeManager::applyByName(const QString& name)
{
    ThemeRow row;
    if (DbLayer::readThemeRowByName(name, &row)) {
        applyRow(row);
    }
}

void ThemeManager::applyRow(const ThemeRow& row)
{
    m_t = row;
    emit changed();
}
