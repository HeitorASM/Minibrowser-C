#include "UrlResolver.h"
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QStringView>

static bool hasScheme(const QString &s) {
    return s.startsWith("http://")  || s.startsWith("https://") ||
           s.startsWith("file://")  || s.startsWith("ftp://")   ||
           s.startsWith("about:")   || s.startsWith("data:");
}

static bool looksLikeDomain(QStringView s) {
    if (s.contains(' ') || s.contains('\n') || s.contains('\r'))
        return false;
    int dot = s.lastIndexOf('.');
    if (dot <= 0 || dot == s.size() - 1)
        return false;
    QStringView tld = s.mid(dot + 1);
    if (tld.size() < 2 || tld.size() > 6)
        return false;
    for (QChar ch : tld)
        if (!ch.isLetterOrNumber())
            return false;
    return true;
}

static QString encodeQuery(const QString &query) {
    QByteArray encoded = QUrl::toPercentEncoding(query, QByteArray(), QByteArray(" "));
    encoded.replace(' ', '+');
    return QString::fromUtf8(encoded);
}

QString UrlResolver::resolve(const QString &input, SearchEngine engine) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty())
        return "about:blank";

    if (hasScheme(trimmed))
        return trimmed;

    if (looksLikeDomain(trimmed))
        return "https://" + trimmed;

    int idx = static_cast<int>(engine);
    if (idx < 0 || idx >= SEARCH_ENGINES.size())
        idx = 0;
    QString base = SEARCH_ENGINES[idx].baseUrl;
    QString encoded = encodeQuery(trimmed);
    return base.arg(encoded);
}

bool UrlResolver::isSearchUrl(const QString &url) {
    for (const auto &eng : SEARCH_ENGINES) {
        QString base = eng.baseUrl;
        int qPos = base.indexOf('?');
        if (qPos != -1)
            base.truncate(qPos);
        if (url.startsWith(base))
            return true;
    }
    return false;
}

QString UrlResolver::extractQuery(const QString &url) {
    QUrl qurl(url);
    if (!qurl.isValid())
        return QString();
    QUrlQuery query(qurl);
    if (query.hasQueryItem("q"))
        return query.queryItemValue("q");
    return QString();
}