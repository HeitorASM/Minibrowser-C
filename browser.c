#include "browser.h"
#include "homepage.h"
#include "search.h"
#include <string.h>

/* Motor de busca global (alterável via UI) */
SearchEngine current_engine = SEARCH_ENGINE_DUCKDUCKGO;

/* ── Helpers internos ──────────────────────────────────────────────────────── */

/* Mostra a homepage GTK (esconde o WebView) */
static void show_homepage(AppState *app)
{
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "home");
    gtk_entry_set_text(GTK_ENTRY(app->entry), "");
    gtk_widget_set_sensitive(app->btn_back,    FALSE);
    gtk_widget_set_sensitive(app->btn_forward, FALSE);
    gtk_widget_set_sensitive(app->btn_reload,  FALSE);
    gtk_label_set_text(GTK_LABEL(app->status_label), "Pronto.");
    gtk_label_set_text(GTK_LABEL(app->tls_icon), "");
    app->on_homepage = TRUE;
}

/* Mostra o WebView (esconde a homepage) */
static void show_webview(AppState *app)
{
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "web");
    app->on_homepage = FALSE;
}

static void sync_entry(AppState *app, const gchar *uri)
{
    if (!uri || app->on_homepage) return;

    gchar *new_text = NULL;
    gchar *query    = search_extract_query(uri);
    new_text = query ? query : g_strdup(uri);

    const gchar *current = gtk_entry_get_text(GTK_ENTRY(app->entry));
    if (g_strcmp0(current, new_text) != 0)
        gtk_entry_set_text(GTK_ENTRY(app->entry), new_text);

    g_free(new_text);
}

static void update_tls(AppState *app, const gchar *uri)
{
    if (!uri) { gtk_label_set_text(GTK_LABEL(app->tls_icon), ""); return; }
    if (g_str_has_prefix(uri, "https://"))
        gtk_label_set_markup(GTK_LABEL(app->tls_icon),
            "<span foreground='#4ade80' font_desc='Sans 11'>🔒</span>");
    else
        gtk_label_set_text(GTK_LABEL(app->tls_icon), "");
}

static void set_status(AppState *app, const gchar *msg)
{
    gtk_label_set_text(GTK_LABEL(app->status_label), msg);
}

/* Throttle de progresso: atualiza no máximo a cada 120 ms */
static guint     progress_timeout_id = 0;
static AppState *progress_app        = NULL;

static gboolean flush_progress(gpointer data G_GNUC_UNUSED)
{
    if (progress_app && !progress_app->on_homepage) {
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
    gchar *full = g_strdup_printf("%s — MiniBrowser",
                      (title && *title) ? title : "Sem título");
    gtk_window_set_title(GTK_WINDOW(app->window), full);
    g_free(full);
}

static void cb_uri_changed(WebKitWebView *wv,
                            GParamSpec   *ps G_GNUC_UNUSED,
                            gpointer      data)
{
    AppState    *app = (AppState *)data;
    const gchar *uri = webkit_web_view_get_uri(wv);
    sync_entry(app, uri);
    update_tls(app, uri);
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
        gtk_widget_set_sensitive(app->btn_reload, TRUE);
        gtk_button_set_label(GTK_BUTTON(app->btn_reload), "✕");
        gtk_widget_set_tooltip_text(app->btn_reload, "Parar (Esc)");
        set_status(app, "Conectando…");
        break;

    case WEBKIT_LOAD_COMMITTED:
        set_status(app, "Carregando página…");
        break;

    case WEBKIT_LOAD_FINISHED:
        gtk_spinner_stop(GTK_SPINNER(app->spinner));
        gtk_button_set_label(GTK_BUTTON(app->btn_reload), "↺");
        gtk_widget_set_tooltip_text(app->btn_reload, "Recarregar (F5)");
        gtk_widget_set_sensitive(app->btn_reload,  TRUE);
        gtk_widget_set_sensitive(app->btn_back,
            webkit_web_view_can_go_back(wv));
        gtk_widget_set_sensitive(app->btn_forward,
            webkit_web_view_can_go_forward(wv));
        set_status(app, "Pronto.");
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
    if (g_error_matches(error, WEBKIT_NETWORK_ERROR, WEBKIT_NETWORK_ERROR_CANCELLED))
        return TRUE;

    gchar *msg = g_strdup_printf("Erro ao carregar %s: %s", uri, error->message);
    set_status(app, msg);
    g_free(msg);
    gtk_spinner_stop(GTK_SPINNER(app->spinner));
    gtk_button_set_label(GTK_BUTTON(app->btn_reload), "↺");
    return TRUE;
}

static void cb_progress_changed(WebKitWebView *wv G_GNUC_UNUSED,
                                 GParamSpec   *ps G_GNUC_UNUSED,
                                 gpointer      data)
{
    progress_app = (AppState *)data;
    if (progress_timeout_id == 0)
        progress_timeout_id = g_timeout_add(120, flush_progress, NULL);
}

/* ── Callbacks de botões/entry ─────────────────────────────────────────────── */

static void cb_go_clicked(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    AppState    *app = (AppState *)data;
    const gchar *txt = gtk_entry_get_text(GTK_ENTRY(app->entry));
    if (!txt || *txt == '\0')
        browser_go_home(app);
    else
        browser_navigate(app, txt);
}

static void cb_entry_activate(GtkEntry *e G_GNUC_UNUSED, gpointer data)
{ cb_go_clicked(NULL, data); }

static gboolean cb_entry_focus(GtkWidget *w,
                                GdkEventFocus *ev G_GNUC_UNUSED,
                                gpointer data G_GNUC_UNUSED)
{
    gtk_editable_select_region(GTK_EDITABLE(w), 0, -1);
    return FALSE;
}

static void cb_back_clicked   (GtkWidget *w G_GNUC_UNUSED, gpointer d)
{ browser_go_back((AppState *)d); }

static void cb_forward_clicked(GtkWidget *w G_GNUC_UNUSED, gpointer d)
{ browser_go_forward((AppState *)d); }

static void cb_reload_clicked (GtkWidget *w G_GNUC_UNUSED, gpointer d)
{
    AppState *app = (AppState *)d;
    gdouble p = webkit_web_view_get_estimated_load_progress(app->web_view);
    if (p > 0.0 && p < 1.0)
        webkit_web_view_stop_loading(app->web_view);
    else
        browser_reload(app);
}

static void cb_home_clicked   (GtkWidget *w G_GNUC_UNUSED, gpointer d)
{ browser_go_home((AppState *)d); }

static gboolean cb_key_press(GtkWidget *w G_GNUC_UNUSED,
                              GdkEventKey *ev,
                              gpointer data)
{
    AppState *app = (AppState *)data;
    switch (ev->keyval) {
    case GDK_KEY_F5:
        browser_reload(app);
        return TRUE;
    case GDK_KEY_Escape:
        webkit_web_view_stop_loading(app->web_view);
        return TRUE;
    case GDK_KEY_l:
        if (ev->state & GDK_CONTROL_MASK) {
            gtk_widget_grab_focus(app->entry);
            gtk_editable_select_region(GTK_EDITABLE(app->entry), 0, -1);
            return TRUE;
        }
        break;
    case GDK_KEY_Left:
        if (ev->state & GDK_MOD1_MASK) { browser_go_back(app);    return TRUE; }
        break;
    case GDK_KEY_Right:
        if (ev->state & GDK_MOD1_MASK) { browser_go_forward(app); return TRUE; }
        break;
    }
    return FALSE;
}

static void homepage_navigate_cb(const gchar *url, gpointer data)
{
    browser_navigate((AppState *)data, url);
}

/* ── API pública ───────────────────────────────────────────────────────────── */

void browser_navigate(AppState *app, const gchar *raw_input)
{
    if (!raw_input || *raw_input == '\0') {
        browser_go_home(app);
        return;
    }
    gchar *uri = search_resolve_input(raw_input, current_engine);
    show_webview(app);
    webkit_web_view_load_uri(app->web_view, uri);
    gtk_widget_grab_focus(GTK_WIDGET(app->web_view)); /* única melhoria mantida */
    g_free(uri);
}

void browser_go_home(AppState *app)
{
    homepage_widget_update_engine(app->homepage, current_engine);
    gtk_window_set_title(GTK_WINDOW(app->window), "MiniBrowser");
    show_homepage(app);
}

void browser_go_back(AppState *app)
{
    if (app->on_homepage) return;
    if (webkit_web_view_can_go_back(app->web_view))
        webkit_web_view_go_back(app->web_view);
}

void browser_go_forward(AppState *app)
{
    if (app->on_homepage) return;
    if (webkit_web_view_can_go_forward(app->web_view))
        webkit_web_view_go_forward(app->web_view);
}

void browser_reload(AppState *app)
{
    if (app->on_homepage) return;
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

    g_signal_connect(app->btn_go,      "clicked",     G_CALLBACK(cb_go_clicked),      app);
    g_signal_connect(app->btn_back,    "clicked",     G_CALLBACK(cb_back_clicked),    app);
    g_signal_connect(app->btn_forward, "clicked",     G_CALLBACK(cb_forward_clicked), app);
    g_signal_connect(app->btn_reload,  "clicked",     G_CALLBACK(cb_reload_clicked),  app);
    g_signal_connect(app->btn_home,    "clicked",     G_CALLBACK(cb_home_clicked),    app);
    g_signal_connect(app->entry,       "activate",    G_CALLBACK(cb_entry_activate),  app);
    g_signal_connect(app->entry,       "focus-in-event", G_CALLBACK(cb_entry_focus),  NULL);
    g_signal_connect(app->window, "key-press-event", G_CALLBACK(cb_key_press), app);
}

void (*browser_get_homepage_nav_cb(void))(const gchar *, gpointer)
{
    return homepage_navigate_cb;
}