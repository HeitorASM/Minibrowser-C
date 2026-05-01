#include "ui.h"
#include "browser.h"
#include "homepage.h"
#include <string.h>

/* ── CSS do tema ─────────────────────────────────────────────────────────── */
static const gchar *APP_CSS =
    "window { background:#0b0c10; }"

    ".toolbar {"
    "  background: #13151c;"
    "  border-bottom: 1px solid #1e2130;"
    "  padding: 5px 10px;"
    "  min-height: 44px;"
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
    "  transition: border-color 120ms, background 120ms;"
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
    "  transition: background 100ms, color 100ms;"
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
    "  transition: background 100ms;"
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

/* ── Seletor de motor de busca ───────────────────────────────────────────── */

typedef struct {
    AppState     *app;
    SearchEngine  engine;
} EngineChoice;

static void on_engine_selected(GtkWidget *item G_GNUC_UNUSED, gpointer data)
{
    EngineChoice *ch = (EngineChoice *)data;
    current_engine = ch->engine;
    ui_set_engine_label(ch->app, ch->engine);
    homepage_widget_update_engine(ch->app->homepage, ch->engine);
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
static void
apply_webkit_performance_tweaks (WebKitWebView *web_view)
{
    WebKitSettings   *settings = webkit_web_view_get_settings   (web_view);
    WebKitWebContext *context  = webkit_web_view_get_context    (web_view);

    /*Aceleração de hardware forçada*/
    webkit_settings_set_hardware_acceleration_policy (
        settings,
        WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS);

    /* Cache */
    webkit_web_context_set_cache_model (context,
        WEBKIT_CACHE_MODEL_WEB_BROWSER);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gchar *cache_dir = g_build_filename (g_get_user_cache_dir(),
                                         "minibrowser", NULL);
    webkit_web_context_set_disk_cache_directory (context, cache_dir);
    g_free (cache_dir);
#pragma GCC diagnostic pop

    /* Multiprocesso */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#if WEBKIT_CHECK_VERSION(2, 30, 0)
    webkit_web_context_set_process_model (
        context,
        WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);
#elif WEBKIT_CHECK_VERSION(2, 26, 0)
    webkit_web_context_set_process_model (
        context,
        WEBKIT_PROCESS_MODEL_SECONDARY_PROCESS);
#endif
#pragma GCC diagnostic pop

    /* ── Desabilita recursos pesados desnecessários ────────────────── */
    webkit_settings_set_enable_webgl             (settings, FALSE);
    webkit_settings_set_enable_webaudio          (settings, FALSE);
    webkit_settings_set_enable_media_stream      (settings, FALSE);
    webkit_settings_set_enable_html5_database    (settings, FALSE);
    webkit_settings_set_enable_write_console_messages_to_stdout (settings, FALSE);
    webkit_settings_set_enable_site_specific_quirks (settings, FALSE);
    webkit_settings_set_enable_resizable_text_areas (settings, FALSE);
    webkit_settings_set_javascript_can_access_clipboard (settings, FALSE);

    /* ── Ativa recursos que melhoram a fluidez ──────────────────── */
    webkit_settings_set_enable_smooth_scrolling  (settings, TRUE);
    webkit_settings_set_enable_html5_local_storage (settings, TRUE);

    /* ── Pré‑conexão DNS para domínios frequentes (atalhos da homepage) ─ */
    const gchar *domains[] = {
        "github.com",
        "wikipedia.org",
        "youtube.com",
        "news.ycombinator.com",
        "reddit.com",
        "openstreetmap.org",
        "duckduckgo.com",
        "google.com",
        "bing.com",
        "search.brave.com",
        NULL
    };
    for (const gchar **d = domains; *d; d++) {
        webkit_web_context_prefetch_dns (context, *d);
    }
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
    /* ── Janela principal ── */
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "MiniBrowser");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 1280, 800);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* CSS global */
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css, APP_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    /* Layout raiz */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);

    /* ── Toolbar ── */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(toolbar), "toolbar");
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    app->btn_back    = flat_btn("◀", "Voltar (Alt+←)");
    app->btn_forward = flat_btn("▶", "Avançar (Alt+→)");
    app->btn_reload  = flat_btn("↺", "Recarregar (F5)");
    app->btn_home    = flat_btn("⌂", "Página inicial");
    gtk_widget_set_sensitive(app->btn_back,    FALSE);
    gtk_widget_set_sensitive(app->btn_forward, FALSE);
    gtk_widget_set_sensitive(app->btn_reload,  FALSE);

    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_back,    FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_forward, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_reload,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_home,    FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(toolbar),
        gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 2);

    /* Seletor de motor de busca */
    GtkWidget *engine_btn = build_engine_selector(app);
    g_object_set_data(G_OBJECT(app->window), "engine_btn", engine_btn);
    gtk_box_pack_start(GTK_BOX(toolbar), engine_btn, FALSE, FALSE, 0);

    /* Ícone de TLS */
    app->tls_icon = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(toolbar), app->tls_icon, FALSE, FALSE, 2);

    /* Barra de endereço */
    app->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry),
                                   "Pesquisar ou digitar endereço…");
    gtk_widget_set_hexpand(app->entry, TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), app->entry, TRUE, TRUE, 6);

    /* Botão "Ir" */
    app->btn_go = gtk_button_new_with_label("Ir");
    gtk_style_context_add_class(gtk_widget_get_style_context(app->btn_go), "go");
    gtk_box_pack_start(GTK_BOX(toolbar), app->btn_go, FALSE, FALSE, 0);

    /* Spinner */
    app->spinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(toolbar), app->spinner, FALSE, FALSE, 6);

    
    app->web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    apply_webkit_performance_tweaks (app->web_view);

    /* ── Stack: homepage ↔ WebView ── */
    app->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app->stack),
                                  GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(app->stack), 120);
    gtk_widget_set_vexpand(app->stack, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), app->stack, TRUE, TRUE, 0);

    /* Homepage GTK nativa */
    app->homepage = homepage_widget_new(browser_get_homepage_nav_cb(), app);
    gtk_stack_add_named(GTK_STACK(app->stack), app->homepage, "home");

    /* WebView dentro de ScrolledWindow */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(app->web_view));
    gtk_stack_add_named(GTK_STACK(app->stack), scroll, "web");

    /* Começa mostrando a homepage */
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "home");
    app->on_homepage = TRUE;

    /* ── Status bar ── */
    GtkWidget *sbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(sbox), "statusbar");
    gtk_container_set_border_width(GTK_CONTAINER(sbox), 0);

    app->status_label = gtk_label_new("Pronto.");
    gtk_widget_set_halign(app->status_label, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(app->status_label), "status");

    gtk_box_pack_start(GTK_BOX(sbox), app->status_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), sbox, FALSE, FALSE, 0);

    gtk_widget_show_all(app->window);
}