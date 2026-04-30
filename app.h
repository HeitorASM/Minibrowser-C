#pragma once

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

/* ── Estado global da aplicação ─────────────────────────────────────────── */
typedef struct {
    /* janela */
    GtkWidget     *window;

    /* toolbar */
    GtkWidget     *entry;
    GtkWidget     *btn_go;
    GtkWidget     *btn_back;
    GtkWidget     *btn_forward;
    GtkWidget     *btn_reload;
    GtkWidget     *btn_home;
    GtkWidget     *spinner;

    /* Stack: alterna entre homepage GTK e WebView */
    GtkWidget     *stack;
    GtkWidget     *homepage;   /* widget GTK nativo da homepage */
    WebKitWebView *web_view;

    /* status */
    GtkWidget     *status_label;
    GtkWidget     *tls_icon;   /* cadeado HTTPS */

    gboolean       on_homepage;
} AppState;