/*
 * homepage.h — Tela inicial do MiniBrowser
 *
 * Gera o HTML da página inicial em tempo de execução, permitindo
 * injetar informações dinâmicas (hora, provedor de busca ativo, etc.)
 */
#pragma once

#include <glib.h>
#include "search.h"

/**
 * homepage_get_html:
 * Retorna o HTML completo da tela inicial (versão estática) alocado com g_malloc.
 * O chamador deve liberar com g_free.
 */
gchar *homepage_get_html(void);

/**
 * homepage_generate_dynamic_js:
 * Retorna uma string JavaScript que atualiza a data e o motor de busca
 * exibidos na página inicial. Deve ser executada após o carregamento da página.
 */
gchar *homepage_generate_dynamic_js(void);

/** URI interna usada para identificar a homepage */
#define HOMEPAGE_URI "minibrowser://home"