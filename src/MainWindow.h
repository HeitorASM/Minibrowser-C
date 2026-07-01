#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolButton>
#include <QStatusBar>
#include <QPointer>
#include "BrowserCore.h"
#include "Omnibox.h"
#include "HomepageWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadStarted();
    void onLoadProgress(int percent);
    void onLoadFinished(bool ok);
    void onUrlChanged(const QString &url);
    void onTitleChanged(const QString &title);
    void onShowHomepage();
    void onShowWebView();
    void onOmniboxReturnPressed();
    void onBackClicked();
    void onForwardClicked();
    void onReloadStopClicked();
    void onHomeClicked();
    void onSettingsClicked();   // novo

private:
    void setupUi();
    void connectSignals();
    void updateButtons();
    void updateLockIcon(const QString &url);
    void loadSettings();        // carrega preferências salvas

    BrowserCore *m_core;
    QStackedWidget *m_stack;
    HomepageWidget *m_homepage;
    Omnibox *m_omnibox;
    QToolButton *m_btnBack;
    QToolButton *m_btnForward;
    QToolButton *m_btnReloadStop;
    QToolButton *m_btnHome;
    QToolButton *m_btnSettings; // novo
    QStatusBar *m_statusBar;
    bool m_isLoading = false;
};