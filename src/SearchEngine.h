#pragma once

#include <QString>
#include <array>

enum class SearchEngine {
    DuckDuckGo,
    Google,
    Bing,
    Brave,
    Count
};

struct SearchEngineInfo {
    QString name;
    QString shortName;
    QString baseUrl;   
    QString icon;
};

inline const std::array<SearchEngineInfo, 4> SEARCH_ENGINES = {{
    { "DuckDuckGo", "DDG",   "https://duckduckgo.com/?q=%1", "🦆" },
    { "Google",     "Google","https://www.google.com/search?q=%1", "🔍" },
    { "Bing",       "Bing",  "https://www.bing.com/search?q=%1", "🅱" },
    { "Brave",      "Brave", "https://search.brave.com/search?q=%1", "🦁" }
}};