#include "homepage.h"
#include <time.h>
#include <stdio.h>

/* HTML estático da homepage (gerado uma vez) */
/*Tinha um erro que ele gerava a mesma coisa milhares de vezes em loop kkk*/
static gchar *static_html = NULL;

static void ensure_static_html(void)
{
    if (static_html)
        return;

    const char *shortcuts[][3] = {
        /*
        a ideia e que teria uma icone mais ele não funciona por causa da biblioteca do GTK
        */
        { "GitHub",     "https://github.com",           "" },
        { "Wikipedia",  "https://wikipedia.org",        "" },
        { "YouTube",    "https://youtube.com",          "" },
        { "HackerNews", "https://news.ycombinator.com", "" },
        { "Reddit",     "https://reddit.com",           "" },
        { "OpenStreetMap","https://openstreetmap.org",  "" },
    };
    const int N = (int)(sizeof(shortcuts) / sizeof(shortcuts[0]));

    GString *shortcuts_html = g_string_new(NULL);
    for (int i = 0; i < N; i++) {
        g_string_append_printf(shortcuts_html,
            "<a href='%s' class='shortcut'>"
            "  <span class='sc-icon'>%s</span>"
            "  <span class='sc-label'>%s</span>"
            "</a>",
            shortcuts[i][1], shortcuts[i][2], shortcuts[i][0]);
    }

    static_html = g_strdup_printf(
/*Homepage e em html e exibida usando o GTK e a API*/
"<!DOCTYPE html>"
"<html lang='pt-BR'>"
"<head>"
"<meta charset='UTF-8'/>"
"<title>MiniBrowser — Início</title>"
"<style>"
"  @import url('https://fonts.googleapis.com/css2?family=DM+Serif+Display:ital@0;1&family=DM+Mono:wght@400;500&display=swap');"

"  :root {"
"    --bg:      #0b0c10;"
"    --surface: #13151c;"
"    --border:  #1e2130;"
"    --text:    #c9d1e0;"
"    --muted:   #4a5068;"
"    --accent:  #5e9bff;"
"    --accent2: #ff7eb3;"
"    --green:   #4ade80;"
"  }"

"  * { margin:0; padding:0; box-sizing:border-box; }"

"  body {"
"    background: var(--bg);"
"    color: var(--text);"
"    font-family: 'DM Mono', monospace;"
"    min-height: 100vh;"
"    display: flex;"
"    flex-direction: column;"
"    align-items: center;"
"    justify-content: center;"
"    gap: 40px;"
"    padding: 40px 20px;"
"    overflow-x: hidden;"
"  }"

"  body::before {"
"    content: '';"
"    position: fixed; inset: 0;"
"    background-image:"
"      linear-gradient(var(--border) 1px, transparent 1px),"
"      linear-gradient(90deg, var(--border) 1px, transparent 1px);"
"    background-size: 40px 40px;"
"    opacity: 0.35;"
"    pointer-events: none;"
"    z-index: 0;"
"  }"

"  .content { position: relative; z-index: 1; width:100%%; max-width:680px;"
"             display:flex; flex-direction:column; align-items:center; gap:32px; }"

"  #clock {"
"    font-family: 'DM Serif Display', serif;"
"    font-size: clamp(3.5rem, 10vw, 6rem);"
"    letter-spacing: -3px;"
"    color: #fff;"
"    line-height: 1;"
"    text-shadow: 0 0 60px rgba(94,155,255,0.3);"
"    animation: fadeUp .6s ease both;"
"  }"

"  #date {"
"    font-size: .75rem;"
"    color: var(--muted);"
"    letter-spacing: .15em;"
"    text-transform: uppercase;"
"    animation: fadeUp .6s .1s ease both;"
"  }"

"  .engine-badge {"
"    display: inline-flex; align-items: center; gap: 6px;"
"    background: var(--surface);"
"    border: 1px solid var(--border);"
"    border-radius: 999px;"
"    padding: 5px 14px;"
"    font-size: .75rem;"
"    color: var(--accent);"
"    animation: fadeUp .6s .2s ease both;"
"  }"

"  .shortcuts {"
"    display: grid;"
"    grid-template-columns: repeat(3, 1fr);"
"    gap: 10px;"
"    width: 100%%;"
"    animation: fadeUp .6s .3s ease both;"
"  }"

"  .shortcut {"
"    display: flex; flex-direction: column; align-items: center; gap: 6px;"
"    background: var(--surface);"
"    border: 1px solid var(--border);"
"    border-radius: 12px;"
"    padding: 16px 10px;"
"    text-decoration: none;"
"    color: var(--text);"
"    transition: border-color .2s, transform .2s, box-shadow .2s;"
"  }"
"  .shortcut:hover {"
"    border-color: var(--accent);"
"    transform: translateY(-3px);"
"    box-shadow: 0 8px 24px rgba(94,155,255,0.12);"
"  }"
"  .sc-icon  { font-size: 1.5rem; }"
"  .sc-label { font-size: .7rem; color: var(--muted); letter-spacing:.05em; }"

"  footer {"
"    font-size: .65rem;"
"    color: var(--muted);"
"    letter-spacing: .1em;"
"    animation: fadeUp .6s .4s ease both;"
"  }"
"  footer span { color: var(--accent2); }"

"  @keyframes fadeUp {"
"    from { opacity:0; transform:translateY(14px); }"
"    to   { opacity:1; transform:translateY(0);    }"
"  }"
"</style>"
"</head>"
"<body>"
"<div class='content'>"

"  <div id='clock'>00:00:00</div>"
"  <div id='date'></div>"

"  <div class='engine-badge' id='engine-badge'></div>"

"  <div class='shortcuts'>%s</div>"

"  <footer>MiniBrowser &mdash; GTK&nbsp;3 + WebKitGTK &mdash; Feito em <span>C</span></footer>"

"</div>"

"<script>"
"(function tick() {"
"  const now = new Date();"
"  const pad = n => String(n).padStart(2,'0');"
"  document.getElementById('clock').textContent ="
"    pad(now.getHours()) + ':' + pad(now.getMinutes()) + ':' + pad(now.getSeconds());"
"  setTimeout(tick, 1000);"
"})();"

"function setDate(dateStr) {"
"  document.getElementById('date').textContent = dateStr;"
"}"

"function setEngine(icon, name) {"
"  document.getElementById('engine-badge').innerHTML = icon + ' ' + name + ' ativo';"
"}"
"</script>"

"</body></html>",
        shortcuts_html->str
    );

    g_string_free(shortcuts_html, TRUE);
}

gchar *homepage_get_html(void)
{
    ensure_static_html();
    return g_strdup(static_html);
}

gchar *homepage_generate_dynamic_js(void)
{
    time_t     now = time(NULL);
    struct tm *tm  = localtime(&now);
    char date_str[64];
    strftime(date_str, sizeof(date_str), "%A, %d de %B de %Y", tm);

    /* O motor de busca ativo é acessado via variável global current_engine */
    extern SearchEngine current_engine;
    const SearchEngineInfo *eng = &SEARCH_ENGINES[current_engine];

    return g_strdup_printf("setDate('%s'); setEngine('%s', '%s');",
                           date_str, eng->icon, eng->name);
}