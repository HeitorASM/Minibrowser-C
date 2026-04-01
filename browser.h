/*
 * browser.h — Lógica de navegação e callbacks do WebKitWebView
 */
#pragma once

#include "app.h"
#include "search.h"

/* Motor de busca atualmente selecionado (persistido em memória) */
extern SearchEngine current_engine;

/* ── Navegação ───────────────────────────────────────────────────────────── */

/** Resolve o input (URL ou query), cria a URI e carrega no WebView */
void browser_navigate(AppState *app, const gchar *raw_input);

/** Carrega a homepage */
void browser_go_home(AppState *app);

/** Volta, avança, recarrega */
void browser_go_back   (AppState *app);
void browser_go_forward(AppState *app);
void browser_reload    (AppState *app);

/* ── Conexão de sinais ───────────────────────────────────────────────────── */

/**
 * browser_connect_signals:
 * Conecta todos os sinais do WebKitWebView e dos botões da toolbar.
 * Deve ser chamado após build_ui().
 */
void browser_connect_signals(AppState *app);