#pragma once

#include <QObject>
#include <QString>

class ThemeManager : public QObject {
    Q_OBJECT
public:
    enum ThemeMode { System, Light, Dark };

    static ThemeManager &instance();

    void applyTheme();
    void setThemeMode(ThemeMode mode);
    ThemeMode themeMode() const;

private:
    ThemeManager() = default;
    QString loadStyleSheet(const QString &filename) const;
    bool isDarkMode() const;

    ThemeMode m_mode = System;
};