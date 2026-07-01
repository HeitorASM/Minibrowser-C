#pragma once

#include <QDialog>
#include <QComboBox>
#include "SearchEngine.h"
#include "ThemeManager.h"

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    SearchEngine selectedEngine() const;
    ThemeManager::ThemeMode themeMode() const;

private:
    void loadSettings();

    QComboBox *m_engineCombo;
    QComboBox *m_themeCombo;
};