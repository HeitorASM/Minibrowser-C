#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Configurações");
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Mecanismo de busca
    QHBoxLayout *engineLayout = new QHBoxLayout;
    QLabel *engineLabel = new QLabel("Mecanismo de busca:", this);
    m_engineCombo = new QComboBox(this);
    for (const auto &eng : SEARCH_ENGINES) {
        m_engineCombo->addItem(eng.icon + " " + eng.name);
    }
    engineLayout->addWidget(engineLabel);
    engineLayout->addWidget(m_engineCombo, 1);
    mainLayout->addLayout(engineLayout);

    // Tema
    QHBoxLayout *themeLayout = new QHBoxLayout;
    QLabel *themeLabel = new QLabel("Tema:", this);
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem("Automático (sistema)");
    m_themeCombo->addItem("Claro");
    m_themeCombo->addItem("Escuro");
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(m_themeCombo, 1);
    mainLayout->addLayout(themeLayout);

    // Botões
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *okBtn = new QPushButton("OK", this);
    QPushButton *cancelBtn = new QPushButton("Cancelar", this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    loadSettings();
}

void SettingsDialog::loadSettings() {
    QSettings settings;
    int engineIdx = settings.value("searchEngine", static_cast<int>(SearchEngine::DuckDuckGo)).toInt();
    m_engineCombo->setCurrentIndex(engineIdx);

    int themeIdx = settings.value("themeMode", static_cast<int>(ThemeManager::System)).toInt();
    m_themeCombo->setCurrentIndex(themeIdx);
}

SearchEngine SettingsDialog::selectedEngine() const {
    return static_cast<SearchEngine>(m_engineCombo->currentIndex());
}

ThemeManager::ThemeMode SettingsDialog::themeMode() const {
    return static_cast<ThemeManager::ThemeMode>(m_themeCombo->currentIndex());
}