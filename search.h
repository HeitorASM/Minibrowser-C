#pragma once

#include <glib.h>

/* Provedores de busca disponíveis */
typedef enum {
    SEARCH_ENGINE_DUCKDUCKGO = 0,  /* padrão */
    SEARCH_ENGINE_GOOGLE,
    SEARCH_ENGINE_BING,
    SEARCH_ENGINE_BRAVE,
    SEARCH_ENGINE_COUNT
} SearchEngine;

/* Informações de um provedor */
typedef struct {
    const char *name;
    const char *short_name;   /* exibido na UI  */
    const char *base_url;     /* %s = query URI-encoded */
    const char *icon;         /* emoji / texto  */
} SearchEngineInfo;

/* Tabela pública (para a UI listar os provedores) */
extern const SearchEngineInfo SEARCH_ENGINES[SEARCH_ENGINE_COUNT];

/* ── API pública ─────────────────────────────────────────────────────────── */

/**
 * search_resolve_input:
 * @input:  texto bruto da barra de endereços
 * @engine: provedor a usar quando for uma busca
 *
 * Retorna uma URI alocada com g_malloc que o chamador deve liberar com g_free.
 *  - Se @input parece URL (tem scheme ou domínio com ponto) → normaliza e devolve.
 *  - Caso contrário → monta URL de busca no provedor escolhido.
 */
gchar *search_resolve_input(const gchar *input, SearchEngine engine);

/**
 * search_is_search_url:
 * Retorna TRUE se a URI pertence a um dos provedores conhecidos
 * (útil para limpar a barra de endereços após uma busca).
 */
gboolean search_is_search_url(const gchar *uri);

/**
 * search_extract_query:
 * Se @uri for URL de busca de um provedor conhecido, extrai e retorna
 * a query decodificada. Retorna NULL caso contrário.
 * O chamador deve liberar com g_free.
 */
gchar *search_extract_query(const gchar *uri);