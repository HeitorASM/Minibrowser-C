#include "ui.h"
#include "browser.h"
#include <string.h>

/* ── CSS do tema ─────────────────────────────────────────────────────────── */
static const gchar *APP_CSS =
    "window { background:#0b0c10; }"

    ".toolbar {"
    "  background: #13151c;"
    "  border-bottom: 1px solid #1e2130;"
    "  padding: 5px 10px;"
    "}"
    ".statusbar {"
    "  background: #0f1117;"
    "  border-top: 1px solid #1e2130;"
    "  padding: 3px 10px;"
    "}"
    "entry {"
    "  background: #1a1d2b;"
    "  color: #c9d1e0;"
    "  border: 1px solid #1e2130;"
    "  border-radius: 8px;"
    "  padding: 5px 12px;"
    "  font-size: 13px;"
    "  caret-color: #5e9bff;"
    "}"
    "entry:focus {"
    "  border-color: #5e9bff;"
    "  background: #1e2133;"
    "}"
    "button.flat {"
    "  background: none;"
    "  border: none;"
    "  color: #4a5068;"
    "  border-radius: 8px;"
    "  padding: 5px 10px;"
    "  font-size: 16px;"
    "  min-width: 0;"
    "}"
    "button.flat:hover    { background:#1e2130; color:#c9d1e0; }"
    "button.flat:disabled { opacity:.3; }"
    "button.go {"
    "  background: #5e9bff;"
    "  color: #0b0c10;"
    "  border: none;"
    "  border-radius: 8px;"
    "  padding: 5px 18px;"
    "  font-weight: bold;"
    "  font-size: 13px;"
    "}"
    "button.go:hover { background: #74c7ec; }"
    "label.status {"
    "  color: #2e3350;"
    "  font-size: 11px;"
    "}"
    "popover { background:#13151c; border:1px solid #1e2130; border-radius:10px; }"
    "popover modelbutton {"
    "  color:#c9d1e0; padding:6px 14px; border-radius:6px; font-size:13px;"
    "}"
    "popover modelbutton:hover { background:#1e2130; }";

/* ── Seletor de motor de busca ────────────────────────────────────────────── */

typedef struct {
    AppState     *app;
    SearchEngine  engine;
} EngineChoice;

static void on_engine_selected(GtkWidget *item G_GNUC_UNUSED, gpointer data)
{
    EngineChoice *ch = (EngineChoice *)data;
    current_engine = ch->engine;
    ui_set_engine_label(ch->app, ch->engine);
}

static GtkWidget *build_engine_selector(AppState *app)
{
    GtkWidget *btn = gtk_menu_button_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "flat");
    gtk_widget_set_tooltip_text(btn, "Selecionar motor de busca");

    GtkWidget *lbl = gtk_label_new(NULL);
    g_object_set_data(G_OBJECT(btn), "label_widget", lbl);

    const SearchEngineInfo *eng = &SEARCH_ENGINES[current_engine];
    gchar *markup = g_strdup_printf("<small>%s %s ▾</small>",
                                    eng->icon, eng->short_name);
    gtk_label_set_markup(GTK_LABEL(lbl), markup);
    g_free(markup);
    gtk_container_add(GTK_CONTAINER(btn), lbl);

    GtkWidget *popover = gtk_popover_new(btn);
    GtkWidget *vbox    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

    for (int i = 0; i < SEARCH_ENGINE_COUNT; i++) {
        gchar     *label = g_strdup_printf("%s  %s",
                               SEARCH_ENGINES[i].icon, SEARCH_ENGINES[i].name);
        GtkWidget *item  = gtk_model_button_new();
        g_object_set(item, "text", label, NULL);
        g_free(label);

        EngineChoice *ch = g_new(EngineChoice, 1);
        ch->app    = app;
        ch->engine = (SearchEngine)i;
        g_object_set_data_full(G_OBJECT(item), "choice", ch, g_free);
        g_signal_connect(item, "clicked", G_CALLBACK(on_engine_selected), ch);
        gtk_box_pack_start(GTK_BOX(vbox), item, FALSE, FALSE, 0);
    }

    gtk_widget_show_all(vbox);
    gtk_container_add(GTK_CONTAINER(popover), vbox);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(btn), popover);
    return btn;
}

/* ── Helper ───────────────────────────────────────────────────────────────── */
static GtkWidget *flat_btn(const gchar *label, const gchar *tooltip)
{
    GtkWidget *b = gtk_button_new_with_label(label);
    gtk_widget_set_tooltip_text(b, tooltip);
    gtk_style_context_add_class(gtk_widget_get_style_context(b), "flat");
    return b;
}

/* ── API pública ─────────────────────────────────────────────────────────── */

void ui_set_engine_label(AppState *app, SearchEngine engine)
{
    GtkWidget *btn = GTK_WIDGET(
        g_object_get_data(G_OBJECT(app->window), "engine_btn"));
    if (!btn) return;

    GtkWidget *lbl = GTK_WIDGET(
        g_object_get_data(G_OBJECT(btn), "label_widget"));
    if (!lbl) return;

    const SearchEngineInfo *eng = &SEARCH_ENGINES[engine];
    gchar *markup = g_strdup_printf("<small>%s %s ▾</small>",
                                    eng->icon, eng->short_name);
    gtk_label_set_markup(GTK_LABEL(lbl), markup);
    g_free(markup);
}

void ui_build(AppState *app)
{
    /* janela */
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "MiniBrowser");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 1280, 800);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* CSS */
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css, APP_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    /* layout */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);

    /* toolbar */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(toolbar), "toolbar");
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    app->btn_back    = flat_btn("◀", "Voltar (Alt+←)");
    app->btn_forward = flat_btn("▶", "Avançar (Alt+→)");
    app->btn_reload  = flat_btn("↺", "Recarregar (F5)");
    app->btn_home    = flat_btn("⌂", "Página inicial");
    gtk_widget_set_sensitive(app->btn_back,    FALSE);
    gtk_widget_set_sensitive(app->btn_forward, FALSE);

    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_back,    FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_forward, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_reload,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_home,    FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(toolbar),
        gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 2);

    GtkWidget *engine_btn = build_engine_selector(app);
    g_object_set_data(G_OBJECT(app->window), "engine_btn", engine_btn);
    gtk_box_pack_start(GTK_BOX(toolbar), engine_btn, FALSE, FALSE, 0);

    app->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry),
                                   "  Pesquisar ou digitar endereço…");
    gtk_widget_set_hexpand(app->entry, TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), app->entry, TRUE, TRUE, 6);

    app->btn_go = gtk_button_new_with_label("Ir");
    gtk_style_context_add_class(gtk_widget_get_style_context(app->btn_go), "go");
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_go, FALSE, FALSE, 0);

    app->spinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(toolbar), app->spinner, FALSE, FALSE, 6);

    /* WebView */
    app->web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

    /* ── Configurações WebKit (apenas APIs não-deprecated no 4.1) ── */
    WebKitSettings *settings = webkit_web_view_get_settings(app->web_view);
    webkit_settings_set_enable_webgl(settings, TRUE);
    webkit_settings_set_enable_media_stream(settings, TRUE);
    webkit_settings_set_enable_developer_extras(settings, FALSE);
    webkit_settings_set_enable_resizable_text_areas(settings, FALSE);
    webkit_settings_set_javascript_can_access_clipboard(settings, FALSE);
    webkit_settings_set_enable_webaudio(settings, FALSE);
    /* hardware acceleration — aceita valores do enum WebKitHardwareAccelerationPolicy */
    webkit_settings_set_hardware_acceleration_policy(
        settings, WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS);

    /* modelo de cache */
    WebKitWebContext *context = webkit_web_view_get_context(app->web_view);
    webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_WEB_BROWSER);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(app->web_view));
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* status bar */
    GtkWidget *sbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(sbox), "statusbar");

    app->status_label = gtk_label_new("Pronto.");
    gtk_widget_set_halign(app->status_label, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(app->status_label), "status");

    gtk_box_pack_start(GTK_BOX(sbox), app->status_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), sbox, FALSE, FALSE, 0);

    gtk_widget_show_all(app->window);
}