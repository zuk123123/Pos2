#pragma once
#include <QString>
#include <optional>

struct ThemeRow {
    QString name;
    int     screenWidth  = 0;
    int     screenHeight = 0;
    double  uiScale      = 1.0;

    int fontSmall = 12;
    int fontBase  = 14;
    int fontLarge = 18;

    int spacing = 8;
    int radius  = 8;
    int padding = 10;

    QString primaryColor;
    QString background;
    QString text;
    QString border;
    QString inputBg;
    QString buttonBg;
    QString buttonText;
};

class DbLayer {
public:
    static bool openOrCreate(QString* outPath = nullptr);     // открывает/создаёт localdb.sqlite
    static bool ensureSchemaAndSeed();                         // создаёт таблицы и базовые темы при необходимости
    static std::optional<ThemeRow> readThemeByName(const QString& name);
    static ThemeRow readInitialTheme();
    static bool updateThemeScreen(const QString& themeName, int w, int h);

    // локальный кэш предпочтений пользователя
    static bool upsertLocalTheme(const QString& login, const QString& themeName);
    static std::optional<QString> readLocalTheme(const QString& login);
};
