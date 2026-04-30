/*
 * homepage.h — Tela inicial do MiniBrowser (GTK puro, sem WebView)
 *
 * A homepage é um widget GTK nativo, exibido diretamente no lugar
 * do WebView quando o usuário está na tela inicial. Isso elimina o
 * overhead de carregar um motor HTML/JS para uma página simples.
 */
#pragma once

#include <gtk/gtk.h>
#include "search.h"
#include "app.h"

/** URI interna usada para identificar a homepage */
#define HOMEPAGE_URI "minibrowser://home"

/**
 * @navigate_cb: callback chamado quando o usuário clica em um atalho.
 * @user_data:   ponteiro passado ao callback (normalmente AppState*).
 */
GtkWidget *homepage_widget_new(void (*navigate_cb)(const gchar *url, gpointer data),
                               gpointer user_data);

void homepage_widget_update_engine(GtkWidget *homepage, SearchEngine engine);

gboolean homepage_widget_tick(gpointer homepage_widget);