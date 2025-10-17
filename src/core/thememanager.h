#pragma once
#include <QObject>
#include <QString>
#include <optional>
#include "db.h"

struct ThemeRow {
    QString name;
    int     screen_width  = 0;
    int     screen_height = 0;
    double  ui_scale      = 1.0;
    int     font_small    = 12;
    int     font_base     = 14;
    int     font_large    = 18;
    int     spacing       = 8;
    int     radius        = 8;
    int     padding       = 10;
    QString primary_color;
    QString background;
    QString text_color;
    QString border_color;
    QString input_bg;
    QString button_bg;
    QString button_text;
};

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString current   READ current   NOTIFY changed)
    Q_PROPERTY(double  fontScale READ fontScale NOTIFY changed)
    Q_PROPERTY(QString bgColor   READ bgColor   NOTIFY changed)
    Q_PROPERTY(QString panelBg   READ panelBg   NOTIFY changed)
    Q_PROPERTY(QString textColor READ textColor NOTIFY changed)
    Q_PROPERTY(QString subtext   READ subtext   NOTIFY changed)
    Q_PROPERTY(QString primary   READ primary   NOTIFY changed)
    Q_PROPERTY(QString border    READ border    NOTIFY changed)
public:
    explicit ThemeManager(QObject* parent=nullptr);
    QString current()   const { return m_t.name; }
    double  fontScale() const { return m_t.ui_scale; }
    QString bgColor()   const { return m_t.background; }
    QString panelBg()   const { return m_t.input_bg; }
    QString textColor() const { return m_t.text_color; }
    QString subtext()   const { return QStringLiteral("#9AA0A6"); }
    QString primary()   const { return m_t.primary_color; }
    QString border()    const { return m_t.border_color; }
    Q_INVOKABLE void applyByName(const QString& name);
    Q_INVOKABLE void applyRow(const ThemeRow& row);
signals:
    void changed();
private:
    ThemeRow m_t;
};

