#pragma once

#include <QString>
#include "SearchEngine.h"

class UrlResolver {
public:
    static QString resolve(const QString &input, SearchEngine engine);
    static bool isSearchUrl(const QString &url);
    static QString extractQuery(const QString &url);
};