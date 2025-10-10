#pragma once
#include <QObject>
#include <QString>
#include "db.h"

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString current READ current NOTIFY changed)
    Q_PROPERTY(double fontScale READ fontScale NOTIFY changed)
    Q_PROPERTY(QString bgColor READ bgColor NOTIFY changed)
    Q_PROPERTY(QString panelBg READ panelBg NOTIFY changed)
    Q_PROPERTY(QString textColor READ textColor NOTIFY changed)
    Q_PROPERTY(QString subtext READ subtext NOTIFY changed)
    Q_PROPERTY(QString primary READ primary NOTIFY changed)
    Q_PROPERTY(QString border READ border NOTIFY changed)
public:
    explicit ThemeManager(QObject* parent=nullptr);

    QString current() const { return m_t.name; }
    double  fontScale() const { return m_t.uiScale; }
    QString bgColor() const { return m_t.background; }
    QString panelBg() const { return m_t.inputBg; }
    QString textColor() const { return m_t.text; }
    QString subtext() const { return "#9AA0A6"; }
    QString primary() const { return m_t.primaryColor; }
    QString border() const { return m_t.border; }

    Q_INVOKABLE void applyByName(const QString& name);

signals:
    void changed();

private:
    ThemeRow m_t;
};
