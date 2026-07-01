#include "MainWindow.h"
#include "ThemeManager.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QWebEngineView>
#include <QMessageBox>
#include <QStyle>
#include <QSettings>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_core = new BrowserCore(this);

    setupUi();
    connectSignals();
    loadSettings();

    ThemeManager::instance().applyTheme();

    m_core->loadHomepage();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("MiniBrowser");
    resize(1280, 800);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout *toolbarLayout = new QHBoxLayout;
    toolbarLayout->setContentsMargins(12, 6, 12, 6);
    toolbarLayout->setSpacing(6);

    m_btnBack = new QToolButton;
    m_btnBack->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    m_btnBack->setToolTip("Voltar (Alt+←)");
    m_btnBack->setAutoRaise(true);
    toolbarLayout->addWidget(m_btnBack);

    m_btnForward = new QToolButton;
    m_btnForward->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    m_btnForward->setToolTip("Avançar (Alt+→)");
    m_btnForward->setAutoRaise(true);
    toolbarLayout->addWidget(m_btnForward);

    m_btnReloadStop = new QToolButton;
    m_btnReloadStop->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    m_btnReloadStop->setToolTip("Recarregar (F5)");
    m_btnReloadStop->setAutoRaise(true);
    toolbarLayout->addWidget(m_btnReloadStop);

    m_btnHome = new QToolButton;
    // Usando texto Unicode em vez de ícone para garantir compatibilidade
    m_btnHome->setText("⌂");
    m_btnHome->setToolTip("Página inicial");
    m_btnHome->setAutoRaise(true);
    toolbarLayout->addWidget(m_btnHome);

    toolbarLayout->addSpacing(8);

    m_omnibox = new Omnibox;
    toolbarLayout->addWidget(m_omnibox, 1);

    m_btnSettings = new QToolButton;
    m_btnSettings->setText("⚙");
    m_btnSettings->setToolTip("Configurações");
    m_btnSettings->setAutoRaise(true);
    toolbarLayout->addWidget(m_btnSettings);

    QWidget *toolbar = new QWidget;
    toolbar->setObjectName("toolbar");
    toolbar->setLayout(toolbarLayout);
    mainLayout->addWidget(toolbar);

    m_stack = new QStackedWidget;
    mainLayout->addWidget(m_stack, 1);

    m_homepage = new HomepageWidget;
    m_stack->addWidget(m_homepage);

    QWidget *webContainer = new QWidget;
    QVBoxLayout *webLayout = new QVBoxLayout(webContainer);
    webLayout->setContentsMargins(0, 0, 0, 0);
    webLayout->addWidget(m_core->webView());
    m_stack->addWidget(webContainer);

    m_statusBar = statusBar();
    m_statusBar->setVisible(false);
}

void MainWindow::connectSignals() {
    connect(m_btnBack, &QToolButton::clicked, this, &MainWindow::onBackClicked);
    connect(m_btnForward, &QToolButton::clicked, this, &MainWindow::onForwardClicked);
    connect(m_btnReloadStop, &QToolButton::clicked, this, &MainWindow::onReloadStopClicked);
    connect(m_btnHome, &QToolButton::clicked, this, &MainWindow::onHomeClicked);
    connect(m_btnSettings, &QToolButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(m_omnibox, &Omnibox::returnPressed, this, &MainWindow::onOmniboxReturnPressed);

    connect(m_core, &BrowserCore::showHomepageRequested, this, &MainWindow::onShowHomepage);
    connect(m_core, &BrowserCore::showWebViewRequested, this, &MainWindow::onShowWebView);
    connect(m_core, &BrowserCore::loadStarted, this, &MainWindow::onLoadStarted);
    connect(m_core, &BrowserCore::loadProgress, this, &MainWindow::onLoadProgress);
    connect(m_core, &BrowserCore::loadFinished, this, &MainWindow::onLoadFinished);
    connect(m_core, &BrowserCore::urlChanged, this, &MainWindow::onUrlChanged);
    connect(m_core, &BrowserCore::titleChanged, this, &MainWindow::onTitleChanged);

    connect(m_homepage, &HomepageWidget::navigateRequested,
            m_core, &BrowserCore::load);

    // Atalhos de teclado (os tooltips já prometiam isso, mas não existiam)
    auto *backShortcut = new QShortcut(QKeySequence("Alt+Left"), this);
    connect(backShortcut, &QShortcut::activated, this, &MainWindow::onBackClicked);

    auto *forwardShortcut = new QShortcut(QKeySequence("Alt+Right"), this);
    connect(forwardShortcut, &QShortcut::activated, this, &MainWindow::onForwardClicked);

    auto *reloadShortcut = new QShortcut(QKeySequence("F5"), this);
    connect(reloadShortcut, &QShortcut::activated, this, &MainWindow::onReloadStopClicked);

    auto *stopShortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(stopShortcut, &QShortcut::activated, this, [this]() {
        if (m_isLoading) m_core->stop();
    });

    auto *focusOmniboxShortcut = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(focusOmniboxShortcut, &QShortcut::activated, this, [this]() {
        m_omnibox->setFocus();
    });
}

void MainWindow::loadSettings() {
    QSettings settings;
    int engineIndex = settings.value("searchEngine", static_cast<int>(SearchEngine::DuckDuckGo)).toInt();
    if (engineIndex >= 0 && engineIndex < static_cast<int>(SearchEngine::Count)) {
        SearchEngine eng = static_cast<SearchEngine>(engineIndex);
        m_core->setSearchEngine(eng);
        m_homepage->setSearchEngine(eng);
    }
}

void MainWindow::onLoadStarted() {
    m_isLoading = true;
    m_btnReloadStop->setIcon(style()->standardIcon(QStyle::SP_BrowserStop));
    m_btnReloadStop->setToolTip("Parar (Esc)");
    updateButtons();
    m_statusBar->showMessage("Carregando…");
    m_statusBar->setVisible(true);
}

void MainWindow::onLoadProgress(int percent) {
    m_statusBar->showMessage(QString("Carregando… %1%").arg(percent));
}

void MainWindow::onLoadFinished(bool ok) {
    m_isLoading = false;
    m_btnReloadStop->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    m_btnReloadStop->setToolTip("Recarregar (F5)");
    updateButtons();
    if (!ok) {
        m_statusBar->setVisible(true);
        m_statusBar->showMessage("Erro ao carregar a página", 5000);
    } else {
        m_statusBar->setVisible(true);
        m_statusBar->showMessage("Pronto", 1500);
        // Esconde a barra somente depois que a mensagem "Pronto" for exibida
        QTimer::singleShot(1500, this, [this]() {
            if (!m_isLoading)
                m_statusBar->setVisible(false);
        });
    }
}

void MainWindow::onUrlChanged(const QString &url) {
    m_omnibox->setText(url);
    updateLockIcon(url);
}

void MainWindow::onTitleChanged(const QString &title) {
    setWindowTitle(title.isEmpty() ? "MiniBrowser" : title + " — MiniBrowser");
}

void MainWindow::onShowHomepage() {
    m_stack->setCurrentIndex(0);
    m_omnibox->setText("");
    m_omnibox->clearLockIcon();
    m_btnBack->setEnabled(false);
    m_btnForward->setEnabled(false);
    m_btnReloadStop->setEnabled(false);
    setWindowTitle("MiniBrowser");
    m_statusBar->setVisible(false);
}

void MainWindow::onShowWebView() {
    m_stack->setCurrentIndex(1);
    m_btnReloadStop->setEnabled(true);
    updateButtons();
}

void MainWindow::onOmniboxReturnPressed() {
    QString text = m_omnibox->text().trimmed();
    if (text.isEmpty())
        m_core->loadHomepage();
    else
        m_core->load(text);
}

void MainWindow::onBackClicked() {
    m_core->goBack();
}

void MainWindow::onForwardClicked() {
    m_core->goForward();
}

void MainWindow::onReloadStopClicked() {
    if (m_isLoading)
        m_core->stop();
    else
        m_core->reload();
}

void MainWindow::onHomeClicked() {
    m_core->loadHomepage();
}

void MainWindow::onSettingsClicked() {
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        SearchEngine eng = dlg.selectedEngine();
        m_core->setSearchEngine(eng);
        m_homepage->setSearchEngine(eng);
        QSettings settings;
        settings.setValue("searchEngine", static_cast<int>(eng));

        ThemeManager::ThemeMode mode = dlg.themeMode();
        ThemeManager::instance().setThemeMode(mode);
    }
}

void MainWindow::updateButtons() {
    m_btnBack->setEnabled(m_core->canGoBack());
    m_btnForward->setEnabled(m_core->canGoForward());
}

void MainWindow::updateLockIcon(const QString &url) {
    if (url.startsWith("https://")) {
        m_omnibox->setLockIcon("🔒", "Conexão segura (HTTPS)");
    } else if (url.startsWith("http://")) {
        m_omnibox->setLockIcon("⛔", "Conexão não segura (HTTP)");
    } else {
        m_omnibox->clearLockIcon();
    }
}