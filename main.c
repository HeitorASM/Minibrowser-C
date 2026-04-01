/*
 * main.c — Ponto de entrada do MiniBrowser
 *
 * Responsabilidade: inicializar GTK, construir a UI, conectar os sinais
 * e carregar a homepage (ou URL passada por argumento).
 *
 * Toda a lógica real está nos módulos:
 *   ui.c      → widgets GTK e CSS
 *   browser.c → navegação e callbacks WebKit
 *   search.c  → resolução de URL/query
 *   homepage.c→ HTML da tela inicial
 */
#include <gtk/gtk.h>
#include <string.h>

#include "app.h"
#include "ui.h"
#include "browser.h"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    AppState app;
    memset(&app, 0, sizeof(app));

    /* 1. Constrói todos os widgets */
    ui_build(&app);

    /* 2. Conecta sinais de navegação e WebKit */
    browser_connect_signals(&app);

    /* 3. Carrega a página inicial (ou URL de argumento) */
    if (argc > 1)
        browser_navigate(&app, argv[1]);
    else
        browser_go_home(&app);

    gtk_main();
    return 0;
}
