#pragma once

#include "app.h"
#include "search.h"

/* Motor de busca atualmente selecionado (persistido em memória) */
extern SearchEngine current_engine;

/* ── Navegação ───────────────────────────────────────────────────────────── */
void browser_navigate   (AppState *app, const gchar *raw_input);
void browser_go_home    (AppState *app);
void browser_go_back    (AppState *app);
void browser_go_forward (AppState *app);
void browser_reload     (AppState *app);

/* ── Conexão de sinais ───────────────────────────────────────────────────── */
void browser_connect_signals(AppState *app);

/* Retorna o callback de navegação para uso na homepage GTK */
void (*browser_get_homepage_nav_cb(void))(const gchar *, gpointer);