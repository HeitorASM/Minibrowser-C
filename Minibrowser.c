#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ─── Página inicial embutida ─────────────────────────────────────────────── */
static const gchar *HOME_HTML =
    "<!DOCTYPE html>"
    "<html lang='pt-BR'>"
    "<head>"
    "  <meta charset='UTF-8'/>"
    "  <title>MiniBrowser</title>"
    "  <style>"
    "    *{margin:0;padding:0;box-sizing:border-box}"
    "    body{font-family:'Segoe UI',sans-serif;background:#0f1117;"
    "         color:#e2e8f0;display:flex;flex-direction:column;"
    "         align-items:center;justify-content:center;height:100vh;gap:24px}"
    "    h1{font-size:2.8rem;letter-spacing:-1px;"
    "       background:linear-gradient(135deg,#38bdf8,#818cf8);"
    "       -webkit-background-clip:text;-webkit-text-fill-color:transparent}"
    "    p{color:#94a3b8;font-size:1rem;text-align:center;"
    "      max-width:400px;line-height:1.7}"
    "    .badge{background:#1e293b;border:1px solid #334155;"
    "           padding:6px 14px;border-radius:999px;font-size:.8rem;"
    "           color:#64748b}"
    "  </style>"
    "</head>"
    "<body>"
    "  <h1>&#127760; MiniBrowser</h1>"
    "  <p>Digite uma URL na barra de endere&ccedil;os e pressione"
    "     <strong>Enter</strong> ou clique em <strong>Ir</strong>.</p>"
    "  <span class='badge'>GTK 3 &plus; WebKitGTK 4.1 &mdash; Feito em C</span>"
    "</body></html>";

/* ─── Estado central ──────────────────────────────────────────────────────── */
typedef struct {
    GtkWidget      *window;
    GtkWidget      *entry;
    GtkWidget      *btn_go;
    GtkWidget      *btn_back;
    GtkWidget      *btn_forward;
    GtkWidget      *btn_reload;
    GtkWidget      *btn_home;
    GtkWidget      *spinner;
    GtkWidget      *status_bar;
    WebKitWebView  *web_view;
} AppState;

/* ─── Utilitários ─────────────────────────────────────────────────────────── */
static gchar *normalize_uri(const gchar *raw)
{
    if (!raw || *raw == '\0')              return g_strdup("about:blank");
    if (g_str_has_prefix(raw, "http://")  ||
        g_str_has_prefix(raw, "https://") ||
        g_str_has_prefix(raw, "file://")  ||
        g_str_has_prefix(raw, "about:"))   return g_strdup(raw);
    return g_strconcat("https://", raw, NULL);
}

static void sync_entry(AppState *app, const gchar *uri)
{
    if (uri && g_strcmp0(uri, "about:blank") != 0)
        gtk_entry_set_text(GTK_ENTRY(app->entry), uri);
}

/* ─── Callbacks WebView ───────────────────────────────────────────────────── */
static void on_title_changed(WebKitWebView *wv, GParamSpec *ps G_GNUC_UNUSED,
                              gpointer data)
{
    AppState    *app   = (AppState *)data;
    const gchar *title = webkit_web_view_get_title(wv);
    gchar       *full  = g_strdup_printf("%s — MiniBrowser",
                             (title && *title) ? title : "Sem título");
    gtk_window_set_title(GTK_WINDOW(app->window), full);
    g_free(full);
}

static void on_uri_changed(WebKitWebView *wv, GParamSpec *ps G_GNUC_UNUSED,
                            gpointer data)
{
    sync_entry((AppState *)data, webkit_web_view_get_uri(wv));
}

static void on_load_changed(WebKitWebView *wv, WebKitLoadEvent ev,
                             gpointer data)
{
    AppState *app = (AppState *)data;
    switch (ev) {
    case WEBKIT_LOAD_STARTED:
    case WEBKIT_LOAD_REDIRECTED:
        gtk_spinner_start(GTK_SPINNER(app->spinner));
        gtk_widget_set_sensitive(app->btn_reload, FALSE);
        gtk_label_set_text(GTK_LABEL(app->status_bar), "Carregando…");
        break;
    case WEBKIT_LOAD_COMMITTED:
        sync_entry(app, webkit_web_view_get_uri(wv));
        break;
    case WEBKIT_LOAD_FINISHED:
        gtk_spinner_stop(GTK_SPINNER(app->spinner));
        gtk_widget_set_sensitive(app->btn_reload,  TRUE);
        gtk_widget_set_sensitive(app->btn_back,    webkit_web_view_can_go_back(wv));
        gtk_widget_set_sensitive(app->btn_forward, webkit_web_view_can_go_forward(wv));
        gtk_label_set_text(GTK_LABEL(app->status_bar), "Pronto.");
        break;
    }
}

static gboolean on_load_failed(WebKitWebView *wv G_GNUC_UNUSED,
                                WebKitLoadEvent ev G_GNUC_UNUSED,
                                gchar *uri, GError *err, gpointer data)
{
    AppState *app = (AppState *)data;
    gchar    *msg = g_strdup_printf("Erro: %s  (%s)", err->message, uri);
    gtk_label_set_text(GTK_LABEL(app->status_bar), msg);
    gtk_spinner_stop(GTK_SPINNER(app->spinner));
    g_free(msg);
    return TRUE;
}

static void on_progress_changed(WebKitWebView *wv,
                                 GParamSpec *ps G_GNUC_UNUSED, gpointer data)
{
    AppState *app  = (AppState *)data;
    gdouble   prog = webkit_web_view_get_estimated_load_progress(wv);
    if (prog < 1.0) {
        gchar *msg = g_strdup_printf("Carregando… %.0f%%", prog * 100.0);
        gtk_label_set_text(GTK_LABEL(app->status_bar), msg);
        g_free(msg);
    }
}

/* ─── Callbacks de navegação ──────────────────────────────────────────────── */
static void navigate_to(AppState *app, const gchar *raw)
{
    gchar *uri = normalize_uri(raw);
    webkit_web_view_load_uri(app->web_view, uri);
    g_free(uri);
}

static void on_go_clicked     (GtkWidget *w G_GNUC_UNUSED, gpointer d)
    { AppState *a = d; navigate_to(a, gtk_entry_get_text(GTK_ENTRY(a->entry))); }
static void on_entry_activate (GtkEntry  *e G_GNUC_UNUSED, gpointer d)
    { on_go_clicked(NULL, d); }
static void on_back_clicked   (GtkWidget *w G_GNUC_UNUSED, gpointer d)
    { AppState *a = d; if (webkit_web_view_can_go_back(a->web_view))    webkit_web_view_go_back(a->web_view); }
static void on_forward_clicked(GtkWidget *w G_GNUC_UNUSED, gpointer d)
    { AppState *a = d; if (webkit_web_view_can_go_forward(a->web_view)) webkit_web_view_go_forward(a->web_view); }
static void on_reload_clicked (GtkWidget *w G_GNUC_UNUSED, gpointer d)
    { AppState *a = d; webkit_web_view_reload(a->web_view); }
static void on_home_clicked   (GtkWidget *w G_GNUC_UNUSED, gpointer d)
    { AppState *a = d;
      webkit_web_view_load_html(a->web_view, HOME_HTML, "about:home");
      gtk_entry_set_text(GTK_ENTRY(a->entry), ""); }

/* ─── Construção da UI ────────────────────────────────────────────────────── */
static GtkWidget *flat_btn(const gchar *label, const gchar *tip)
{
    GtkWidget *b = gtk_button_new_with_label(label);
    gtk_widget_set_tooltip_text(b, tip);
    gtk_style_context_add_class(gtk_widget_get_style_context(b), "flat");
    return b;
}

static void build_ui(AppState *app)
{
    /* janela */
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "MiniBrowser");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 1280, 800);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* CSS */
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "window { background:#1e1e2e; }"
        ".toolbar { background:#181825; border-bottom:1px solid #313244; padding:4px 8px; }"
        "entry { background:#313244; color:#cdd6f4; border:1px solid #45475a;"
        "        border-radius:6px; padding:4px 10px; font-size:13px; }"
        "entry:focus { border-color:#89b4fa; }"
        "button.flat { background:none; border:none; color:#cdd6f4;"
        "              border-radius:6px; padding:4px 10px; font-size:15px; }"
        "button.flat:hover { background:#313244; }"
        "button.go { background:#89b4fa; color:#1e1e2e; border:none;"
        "            border-radius:6px; padding:4px 16px; font-weight:bold; }"
        "button.go:hover { background:#74c7ec; }"
        "label.status { color:#6c7086; font-size:11px; padding:2px 8px; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    /* layout */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);

    /* ── toolbar ── */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(toolbar), "toolbar");
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    app->btn_back    = flat_btn("◀", "Voltar");
    app->btn_forward = flat_btn("▶", "Avançar");
    app->btn_reload  = flat_btn("↺", "Recarregar");
    app->btn_home    = flat_btn("⌂", "Página inicial");
    gtk_widget_set_sensitive(app->btn_back,    FALSE);
    gtk_widget_set_sensitive(app->btn_forward, FALSE);

    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_back,    FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_forward, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_reload,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_home,    FALSE, FALSE, 0);

    app->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry), "  Digite uma URL…");
    gtk_widget_set_hexpand(app->entry, TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), app->entry, TRUE, TRUE, 4);

    app->btn_go = gtk_button_new_with_label("Ir");
    gtk_style_context_add_class(gtk_widget_get_style_context(app->btn_go), "go");
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_go, FALSE, FALSE, 0);

    app->spinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(toolbar), app->spinner, FALSE, FALSE, 4);

    /* ── WebView ── */
    app->web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(app->web_view));
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* ── status bar ── */
    app->status_bar = gtk_label_new("Pronto.");
    gtk_widget_set_halign(app->status_bar, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(app->status_bar), "status");
    GtkWidget *sbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(sbox), "toolbar");
    gtk_box_pack_start(GTK_BOX(sbox), app->status_bar, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), sbox, FALSE, FALSE, 0);

    /* ── sinais ── */
    g_signal_connect(app->btn_go,      "clicked",  G_CALLBACK(on_go_clicked),      app);
    g_signal_connect(app->btn_back,    "clicked",  G_CALLBACK(on_back_clicked),    app);
    g_signal_connect(app->btn_forward, "clicked",  G_CALLBACK(on_forward_clicked), app);
    g_signal_connect(app->btn_reload,  "clicked",  G_CALLBACK(on_reload_clicked),  app);
    g_signal_connect(app->btn_home,    "clicked",  G_CALLBACK(on_home_clicked),    app);
    g_signal_connect(app->entry,       "activate", G_CALLBACK(on_entry_activate),  app);

    g_signal_connect(app->web_view, "notify::title",
                     G_CALLBACK(on_title_changed),    app);
    g_signal_connect(app->web_view, "notify::uri",
                     G_CALLBACK(on_uri_changed),      app);
    g_signal_connect(app->web_view, "load-changed",
                     G_CALLBACK(on_load_changed),     app);
    g_signal_connect(app->web_view, "load-failed",
                     G_CALLBACK(on_load_failed),      app);
    g_signal_connect(app->web_view, "notify::estimated-load-progress",
                     G_CALLBACK(on_progress_changed), app);

    gtk_widget_show_all(app->window);
    webkit_web_view_load_html(app->web_view, HOME_HTML, "about:home");
}

/* ─── main ────────────────────────────────────────────────────────────────── */
int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    AppState app;
    memset(&app, 0, sizeof(app));
    build_ui(&app);

    if (argc > 1)
        navigate_to(&app, argv[1]);

    gtk_main();
    return EXIT_SUCCESS;
}