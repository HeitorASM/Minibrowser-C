#include "search.h"
#include <string.h>
#include <ctype.h>

/* ── Tabela de provedores ─────────────────────────────────────────────────── */
const SearchEngineInfo SEARCH_ENGINES[SEARCH_ENGINE_COUNT] = {
    [SEARCH_ENGINE_DUCKDUCKGO] = {
        .name       = "DuckDuckGo",
        .short_name = "DDG",
        .base_url   = "https://duckduckgo.com/?q=%s",
        .icon       = "🦆"
    },
    [SEARCH_ENGINE_GOOGLE] = {
        .name       = "Google",
        .short_name = "Google",
        .base_url   = "https://www.google.com/search?q=%s",
        .icon       = "🔍"
    },
    [SEARCH_ENGINE_BING] = {
        .name       = "Bing",
        .short_name = "Bing",
        .base_url   = "https://www.bing.com/search?q=%s",
        .icon       = "🅱"
    },
    [SEARCH_ENGINE_BRAVE] = {
        .name       = "Brave Search",
        .short_name = "Brave",
        .base_url   = "https://search.brave.com/search?q=%s",
        .icon       = "🦁"
    },
};

/* ── Helpers internos ─────────────────────────────────────────────────────── */

/* Verifica se o texto já tem um scheme reconhecido */
static gboolean has_scheme(const gchar *s)
{
    return g_str_has_prefix(s, "http://")   ||
           g_str_has_prefix(s, "https://")  ||
           g_str_has_prefix(s, "file://")   ||
           g_str_has_prefix(s, "ftp://")    ||
           g_str_has_prefix(s, "about:")    ||
           g_str_has_prefix(s, "data:");
}

/*
 * Heurística simples: parece domínio se contém um ponto, não tem espaços
 * e o TLD depois do último ponto tem entre 2 e 6 letras.
 * Ex.: "google.com", "sub.exemplo.com.br", "localhost" (sem ponto → busca).
 */
static gboolean looks_like_domain(const gchar *s)
{
    if (strchr(s, ' '))  return FALSE;   /* espaços = frase de busca */
    if (strchr(s, '\n')) return FALSE;

    const gchar *dot = strrchr(s, '.');
    if (!dot || dot == s) return FALSE;

    const gchar *tld = dot + 1;
    gsize len = strlen(tld);
    if (len < 2 || len > 6) return FALSE;

    /* TLD deve ser só letras/dígitos */
    for (gsize i = 0; i < len; i++)
        if (!isalnum((unsigned char)tld[i])) return FALSE;

    return TRUE;
}

/* URI-encode a query (substitui espaços por + e codifica especiais) */
static gchar *uri_encode_query(const gchar *query)
{
    /* g_uri_escape_string preserva letras, dígitos e alguns chars seguros */
    gchar *encoded = g_uri_escape_string(query,
                                         G_URI_RESERVED_CHARS_ALLOWED_IN_PATH,
                                         FALSE);
    /* Substitui %20 por + (convenção de query string) */
    GString *result = g_string_new(NULL);
    for (gchar *p = encoded; *p; p++) {
        if (p[0] == '%' && p[1] == '2' && (p[2] == '0' || p[2] == '0')) {
            g_string_append_c(result, '+');
            p += 2;
        } else {
            g_string_append_c(result, *p);
        }
    }
    g_free(encoded);
    return g_string_free(result, FALSE);
}

/* ── API pública ─────────────────────────────────────────────────────────── */

gchar *search_resolve_input(const gchar *input, SearchEngine engine)
{
    if (!input || *input == '\0')
        return g_strdup("about:blank");

    /* Remove espaços nas pontas */
    gchar *trimmed = g_strstrip(g_strdup(input));

    /* Já tem scheme → devolve direto */
    if (has_scheme(trimmed))
        return trimmed;

    /* Parece domínio → adiciona https:// */
    if (looks_like_domain(trimmed)) {
        gchar *uri = g_strconcat("https://", trimmed, NULL);
        g_free(trimmed);
        return uri;
    }

    /* É uma busca → monta a URL do provedor */
    if (engine < 0 || engine >= SEARCH_ENGINE_COUNT)
        engine = SEARCH_ENGINE_DUCKDUCKGO;

    gchar *encoded = uri_encode_query(trimmed);
    gchar *uri     = g_strdup_printf(SEARCH_ENGINES[engine].base_url, encoded);
    g_free(encoded);
    g_free(trimmed);
    return uri;
}

gboolean search_is_search_url(const gchar *uri)
{
    if (!uri) return FALSE;
    for (int i = 0; i < SEARCH_ENGINE_COUNT; i++) {
        /* Extrai o host base da base_url para comparar */
        gchar *base = g_strdup(SEARCH_ENGINES[i].base_url);
        gchar *qs   = strchr(base, '?');
        if (qs) *qs = '\0';
        gboolean match = g_str_has_prefix(uri, base);
        g_free(base);
        if (match) return TRUE;
    }
    return FALSE;
}

gchar *search_extract_query(const gchar *uri)
{
    if (!uri) return NULL;

    /* Procura ?q= ou &q= */
    const gchar *q = strstr(uri, "?q=");
    if (!q) q = strstr(uri, "&q=");
    if (!q) return NULL;

    q += 3; /* pula "?q=" ou "&q=" */

    /* Copia até o próximo & ou fim */
    const gchar *end = strchr(q, '&');
    gchar *raw = end ? g_strndup(q, (gsize)(end - q)) : g_strdup(q);

    /* Decodifica %XX e + → espaço */
    gchar *decoded = g_uri_unescape_string(raw, NULL);
    g_free(raw);

    /* Substitui + por espaço */
    if (decoded)
        for (gchar *p = decoded; *p; p++)
            if (*p == '+') *p = ' ';

    return decoded;
}