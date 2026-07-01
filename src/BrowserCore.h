#pragma once

#include <QObject>
#include <QPointer>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include "SearchEngine.h"

class BrowserCore : public QObject {
    Q_OBJECT
public:
    explicit BrowserCore(QObject *parent = nullptr);
    ~BrowserCore();

    QWebEngineView *webView() const { return m_view; }
    QWebEngineProfile *profile() const { return m_profile; }

    void load(const QString &input);
    void loadHomepage();
    void goBack();
    void goForward();
    void reload();
    void stop();
    bool canGoBack() const;
    bool canGoForward() const;
    QString currentUrl() const;

    void setSearchEngine(SearchEngine engine) { m_currentEngine = engine; }
    SearchEngine searchEngine() const { return m_currentEngine; }

signals:
    void showHomepageRequested();
    void showWebViewRequested();
    void loadProgress(int percent);
    void titleChanged(const QString &title);
    void urlChanged(const QString &url);
    void loadFinished(bool ok);
    void loadStarted();

private slots:
    void onLoadStarted();
    void onLoadProgress(int progress);
    void onLoadFinished(bool ok);
    void onUrlChanged(const QUrl &url);
    void onTitleChanged(const QString &title);

private:
    void setupProfile();
    void prefetchDns();

    QWebEngineView *m_view = nullptr;
    QWebEngineProfile *m_profile = nullptr;
    SearchEngine m_currentEngine = SearchEngine::DuckDuckGo;
};