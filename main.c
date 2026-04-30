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