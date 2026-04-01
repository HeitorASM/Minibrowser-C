#include "browser.h"
#include "homepage.h"
#include "search.h"
#include <string.h>

/* Motor de busca global (alterável via UI) */
SearchEngine current_engine = SEARCH_ENGINE_DUCKDUCKGO;

/* ── Helpers internos ──────────────────────────────────────────────────────── */

static void sync_entry(AppState *app, const gchar *uri)
{
    if (!uri) return;

    const gchar *current = gtk_entry_get_text(GTK_ENTRY(app->entry));
    gchar *new_text = NULL;

    if (g_str_has_prefix(uri, "about:") ||
        g_strcmp0(uri, HOMEPAGE_URI) == 0) {
        new_text = g_strdup("");
    } else {
        gchar *query = search_extract_query(uri);
        new_text = query ? query : g_strdup(uri);
    }

    if (g_strcmp0(current, new_text) != 0)
        gtk_entry_set_text(GTK_ENTRY(app->entry), new_text);

    g_free(new_text);
}

static void set_status(AppState *app, const gchar *msg)
{
    gtk_label_set_text(GTK_LABEL(app->status_label), msg);
}

/* Throttle de progresso: atualiza no máximo a cada 150 ms */
static guint  progress_timeout_id = 0;
static AppState *progress_app     = NULL;

static gboolean flush_progress(gpointer data G_GNUC_UNUSED)
{
    if (progress_app) {
        gdouble p = webkit_web_view_get_estimated_load_progress(
                        progress_app->web_view);
        if (p > 0.0 && p < 1.0) {
            gchar *msg = g_strdup_printf("Carregando… %.0f%%", p * 100.0);
            set_status(progress_app, msg);
            g_free(msg);
        }
    }
    progress_timeout_id = 0;
    return G_SOURCE_REMOVE;
}

/* ── Callbacks WebKit ──────────────────────────────────────────────────────── */

static void cb_title_changed(WebKitWebView *wv,
                              GParamSpec   *ps G_GNUC_UNUSED,
                              gpointer      data)
{
    AppState    *app   = (AppState *)data;
    const gchar *title = webkit_web_view_get_title(wv);
    gchar       *full  = g_strdup_printf("%s — MiniBrowser",
                             (title && *title) ? title : "Sem título");
    gtk_window_set_title(GTK_WINDOW(app->window), full);
    g_free(full);
}

static void cb_uri_changed(WebKitWebView *wv,
                            GParamSpec   *ps G_GNUC_UNUSED,
                            gpointer      data)
{
    sync_entry((AppState *)data, webkit_web_view_get_uri(wv));
}

static void cb_load_changed(WebKitWebView  *wv,
                             WebKitLoadEvent event,
                             gpointer        data)
{
    AppState *app = (AppState *)data;

    switch (event) {
    case WEBKIT_LOAD_STARTED:
    case WEBKIT_LOAD_REDIRECTED:
        gtk_spinner_start(GTK_SPINNER(app->spinner));
        gtk_widget_set_sensitive(app->btn_reload, FALSE);
        set_status(app, "Carregando…");
        break;

    case WEBKIT_LOAD_COMMITTED:
        /* notify::uri já sincroniza a entry */
        break;

    case WEBKIT_LOAD_FINISHED:
        gtk_spinner_stop(GTK_SPINNER(app->spinner));
        gtk_widget_set_sensitive(app->btn_reload,  TRUE);
        gtk_widget_set_sensitive(app->btn_back,
            webkit_web_view_can_go_back(wv));
        gtk_widget_set_sensitive(app->btn_forward,
            webkit_web_view_can_go_forward(wv));
        set_status(app, "Pronto.");

        /* Injeta dados dinâmicos na homepage (data e motor de busca) */
        {
            const gchar *uri = webkit_web_view_get_uri(wv);
            if (uri && g_strcmp0(uri, HOMEPAGE_URI) == 0) {
                gchar *js = homepage_generate_dynamic_js();
                /* 8 args: webview, script, len, world, source_uri, cancel, cb, userdata */
                webkit_web_view_evaluate_javascript(
                    wv, js, -1, NULL, NULL, NULL, NULL, NULL);
                g_free(js);
            }
        }
        break;
    }
}

static gboolean cb_load_failed(WebKitWebView  *wv G_GNUC_UNUSED,
                                WebKitLoadEvent ev G_GNUC_UNUSED,
                                gchar          *uri,
                                GError         *error,
                                gpointer        data)
{
    AppState *app = (AppState *)data;
    gchar    *msg = g_strdup_printf("Erro ao carregar %s: %s",
                                    uri, error->message);
    set_status(app, msg);
    gtk_spinner_stop(GTK_SPINNER(app->spinner));
    g_free(msg);
    return TRUE;
}

static void cb_progress_changed(WebKitWebView *wv G_GNUC_UNUSED,
                                 GParamSpec   *ps G_GNUC_UNUSED,
                                 gpointer      data)
{
    progress_app = (AppState *)data;
    if (progress_timeout_id == 0)
        progress_timeout_id = g_timeout_add(150, flush_progress, NULL);
}

/* ── Callbacks de botões/entry ─────────────────────────────────────────────── */

static void cb_go_clicked(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    AppState    *app = (AppState *)data;
    const gchar *txt = gtk_entry_get_text(GTK_ENTRY(app->entry));
    browser_navigate(app, txt);
}

static void cb_entry_activate(GtkEntry *e G_GNUC_UNUSED, gpointer data)
{ cb_go_clicked(NULL, data); }

static void cb_back_clicked   (GtkWidget *w G_GNUC_UNUSED, gpointer d)
{ browser_go_back((AppState *)d); }

static void cb_forward_clicked(GtkWidget *w G_GNUC_UNUSED, gpointer d)
{ browser_go_forward((AppState *)d); }

static void cb_reload_clicked (GtkWidget *w G_GNUC_UNUSED, gpointer d)
{ browser_reload((AppState *)d); }

static void cb_home_clicked   (GtkWidget *w G_GNUC_UNUSED, gpointer d)
{ browser_go_home((AppState *)d); }

/* ── API pública ───────────────────────────────────────────────────────────── */

void browser_navigate(AppState *app, const gchar *raw_input)
{
    if (!raw_input || *raw_input == '\0') {
        browser_go_home(app);
        return;
    }
    gchar *uri = search_resolve_input(raw_input, current_engine);
    webkit_web_view_load_uri(app->web_view, uri);
    g_free(uri);
}

void browser_go_home(AppState *app)
{
    gchar *html = homepage_get_html();
    webkit_web_view_load_html(app->web_view, html, HOMEPAGE_URI);
    gtk_entry_set_text(GTK_ENTRY(app->entry), "");
    g_free(html);
}

void browser_go_back(AppState *app)
{
    if (webkit_web_view_can_go_back(app->web_view))
        webkit_web_view_go_back(app->web_view);
}

void browser_go_forward(AppState *app)
{
    if (webkit_web_view_can_go_forward(app->web_view))
        webkit_web_view_go_forward(app->web_view);
}

void browser_reload(AppState *app)
{
    webkit_web_view_reload(app->web_view);
}

void browser_connect_signals(AppState *app)
{
    g_signal_connect(app->web_view, "notify::title",
                     G_CALLBACK(cb_title_changed),    app);
    g_signal_connect(app->web_view, "notify::uri",
                     G_CALLBACK(cb_uri_changed),      app);
    g_signal_connect(app->web_view, "load-changed",
                     G_CALLBACK(cb_load_changed),     app);
    g_signal_connect(app->web_view, "load-failed",
                     G_CALLBACK(cb_load_failed),      app);
    g_signal_connect(app->web_view, "notify::estimated-load-progress",
                     G_CALLBACK(cb_progress_changed), app);

    g_signal_connect(app->btn_go,      "clicked",  G_CALLBACK(cb_go_clicked),      app);
    g_signal_connect(app->btn_back,    "clicked",  G_CALLBACK(cb_back_clicked),    app);
    g_signal_connect(app->btn_forward, "clicked",  G_CALLBACK(cb_forward_clicked), app);
    g_signal_connect(app->btn_reload,  "clicked",  G_CALLBACK(cb_reload_clicked),  app);
    g_signal_connect(app->btn_home,    "clicked",  G_CALLBACK(cb_home_clicked),    app);
    g_signal_connect(app->entry,       "activate", G_CALLBACK(cb_entry_activate),  app);
}