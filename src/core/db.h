#pragma once
#include <QString>
#include <QStringList>
#include <optional>

struct ThemeRow;

namespace DbLayer {

// Открыть/создать БД по конкретному пути (должен вызываться из main.cpp)
bool openOrCreate(const QString& sqlitePath);

// Схема + seed
bool ensureSchemaAndSeed();

// Темы
bool readThemeRowByName(const QString& name, ThemeRow* out);
void writeThemeByName(const QString& name);

// НОВОЕ: список имён тем (для SettingsBridge/Settings.qml)
QStringList listThemeNames();

// НОВОЕ: upsert кастомной/переопределённой темы
bool upsertTheme(const ThemeRow& t);

// Пользователи
int  upsertUserLogin(const QString& login);            // возвращает user_id (локальный)
std::optional<int> getUserIdByLogin(const QString& login);

// Настройки пользователя
std::optional<QString> getUserTheme(int userId);
bool setUserTheme(int userId, const QString& themeName);

// Локальный кэш маппинга login->theme (старый интерфейс, чтобы не ломать текущую логику)
std::optional<QString> readLocalTheme(const QString& login);
void upsertLocalTheme(const QString& login, const QString& theme_name);

// Начальная тема
ThemeRow readInitialTheme();

// Инфо о экране
void upsertScreenInfo(int width, int height, double dpi, double dpr, double scale);

} // namespace DbLayer
