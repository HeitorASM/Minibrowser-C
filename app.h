/*
 * app.h — Estrutura central compartilhada entre todos os módulos
 */
#pragma once

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

/* ── Estado global da aplicação ─────────────────────────────────────────── */
typedef struct {
    /* janela */
    GtkWidget     *window;

    /* toolbar */
    GtkWidget     *entry;        /* barra de endereços / busca       */
    GtkWidget     *btn_go;
    GtkWidget     *btn_back;
    GtkWidget     *btn_forward;
    GtkWidget     *btn_reload;
    GtkWidget     *btn_home;
    GtkWidget     *spinner;

    /* conteúdo */
    WebKitWebView *web_view;

    /* status */
    GtkWidget     *status_label;
} AppState;
