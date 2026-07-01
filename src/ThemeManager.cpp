#include "ThemeManager.h"
#include <QApplication>
#include <QPalette>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QSettings>

ThemeManager &ThemeManager::instance() {
    static ThemeManager inst;
    return inst;
}

bool ThemeManager::isDarkMode() const {
    QPalette pal = qApp->palette();
    QColor bg = pal.color(QPalette::Window);
    return bg.lightness() < 128;
}

QString ThemeManager::loadStyleSheet(const QString &filename) const {
    // Carrega do diretório resources/ ao lado do executável
    QString path = QCoreApplication::applicationDirPath() + "/resources/" + filename;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream stream(&file);
    return stream.readAll();
}

void ThemeManager::setThemeMode(ThemeMode mode) {
    m_mode = mode;
    QSettings settings;
    settings.setValue("themeMode", static_cast<int>(mode));
    applyTheme();
}

ThemeManager::ThemeMode ThemeManager::themeMode() const {
    return m_mode;
}

void ThemeManager::applyTheme() {
    // Carrega o modo salvo (se não foi definido ainda)
    QSettings settings;
    if (settings.contains("themeMode")) {
        m_mode = static_cast<ThemeMode>(settings.value("themeMode").toInt());
    }

    bool dark = false;
    switch (m_mode) {
        case System:
            dark = isDarkMode();
            break;
        case Light:
            dark = false;
            break;
        case Dark:
            dark = true;
            break;
    }

    QString css;
    if (dark) {
        css = loadStyleSheet("style_dark.qss");
    } else {
        css = loadStyleSheet("style_light.qss");
    }
    if (!css.isEmpty())
        qApp->setStyleSheet(css);
}