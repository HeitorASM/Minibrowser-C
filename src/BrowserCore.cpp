#include "BrowserCore.h"
#include "UrlResolver.h"
#include <QHostInfo>
#include <QStandardPaths>
#include <QDir>
#include <QWebEngineHistory>

BrowserCore::BrowserCore(QObject *parent) : QObject(parent) {
    m_profile = new QWebEngineProfile(this);
    setupProfile();

    m_view = new QWebEngineView;
    m_view->setPage(new QWebEnginePage(m_profile, m_view));

    connect(m_view, &QWebEngineView::loadStarted, this, &BrowserCore::onLoadStarted);
    connect(m_view, &QWebEngineView::loadProgress, this, &BrowserCore::onLoadProgress);
    connect(m_view, &QWebEngineView::loadFinished, this, &BrowserCore::onLoadFinished);
    connect(m_view, &QWebEngineView::urlChanged, this, &BrowserCore::onUrlChanged);
    connect(m_view, &QWebEngineView::titleChanged, this, &BrowserCore::onTitleChanged);

    prefetchDns();
}

BrowserCore::~BrowserCore() = default;

void BrowserCore::setupProfile() {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dataPath);
    m_profile->setPersistentStoragePath(dataPath);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    m_profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    m_profile->setHttpCacheMaximumSize(100 * 1024 * 1024);
}

void BrowserCore::prefetchDns() {
    const QStringList domains = {
        "github.com", "wikipedia.org", "youtube.com",
        "news.ycombinator.com", "reddit.com", "openstreetmap.org",
        "duckduckgo.com", "google.com", "bing.com", "search.brave.com"
    };
    for (const QString &domain : domains)
        QHostInfo::lookupHost(domain, this, [](const QHostInfo &) {});
}

void BrowserCore::load(const QString &input) {
    QString uri = UrlResolver::resolve(input, m_currentEngine);
    if (uri == "minibrowser://home") {
        loadHomepage();
        return;
    }
    m_view->load(QUrl(uri));
    emit showWebViewRequested();
}

void BrowserCore::loadHomepage() {
    emit showHomepageRequested();
}

void BrowserCore::goBack() {
    if (canGoBack()) m_view->back();
}

void BrowserCore::goForward() {
    if (canGoForward()) m_view->forward();
}

void BrowserCore::reload() {
    m_view->reload();
}

void BrowserCore::stop() {
    m_view->stop();
}

bool BrowserCore::canGoBack() const {
    return m_view->history()->canGoBack();
}

bool BrowserCore::canGoForward() const {
    return m_view->history()->canGoForward();
}

QString BrowserCore::currentUrl() const {
    return m_view->url().toString();
}

void BrowserCore::onLoadStarted() {
    emit loadStarted();
}

void BrowserCore::onLoadProgress(int progress) {
    emit loadProgress(progress);
}

void BrowserCore::onLoadFinished(bool ok) {
    emit loadFinished(ok);
}

void BrowserCore::onUrlChanged(const QUrl &url) {
    emit urlChanged(url.toString());
}

void BrowserCore::onTitleChanged(const QString &title) {
    emit titleChanged(title);
}