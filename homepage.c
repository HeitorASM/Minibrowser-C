#include "homepage.h"
#include <time.h>
#include <string.h>

/* ── Dados dos atalhos ────────────────────────────────────────────────────── */
typedef struct { const gchar *label; const gchar *url; const gchar *icon; } Shortcut;

static const Shortcut SHORTCUTS[] = {
    { "GitHub",       "https://github.com",           "󰊤" },
    { "Wikipedia",    "https://wikipedia.org",        "W"  },
    { "YouTube",      "https://youtube.com",          "▶"  },
    { "Hacker News",  "https://news.ycombinator.com", "Y"  },
    { "Reddit",       "https://reddit.com",           "🅁"  },
    { "OpenStreetMap","https://openstreetmap.org",    "🗺"  },
};
static const int N_SHORTCUTS = (int)G_N_ELEMENTS(SHORTCUTS);

/* ── Estado interno do widget ─────────────────────────────────────────────── */
typedef struct {
    GtkWidget  *clock_label;
    GtkWidget  *date_label;
    GtkWidget  *engine_label;
    guint       tick_id;
} HomepageData;

/* Libera HomepageData quando o widget é destruído */
static void on_homepage_destroy(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    HomepageData *hd = (HomepageData *)data;
    if (hd->tick_id)
        g_source_remove(hd->tick_id);
    g_free(hd);
}

/* ── Relógio ──────────────────────────────────────────────────────────────── */
static void update_clock(HomepageData *hd)
{
    time_t     now = time(NULL);
    struct tm *tm  = localtime(&now);
    char clock_str[16], date_str[64];

    strftime(clock_str, sizeof(clock_str), "%H:%M:%S", tm);
    strftime(date_str,  sizeof(date_str),
             "%A, %d de %B de %Y", tm);

    /* Relógio em markup grande */
    gchar *markup = g_strdup_printf(
        "<span font_desc='Monospace Bold 48' foreground='#ffffff'>%s</span>",
        clock_str);
    gtk_label_set_markup(GTK_LABEL(hd->clock_label), markup);
    g_free(markup);

    /* Data em texto menor */
    gchar *date_markup = g_strdup_printf(
        "<span font_desc='Monospace 9' foreground='#4a5068' letter_spacing='1024'>%s</span>",
        date_str);
    gtk_label_set_markup(GTK_LABEL(hd->date_label), date_markup);
    g_free(date_markup);
}

gboolean homepage_widget_tick(gpointer data)
{
    HomepageData *hd = (HomepageData *)g_object_get_data(G_OBJECT(data), "hp_data");
    if (hd) update_clock(hd);
    return G_SOURCE_CONTINUE;
}

/* ── Shortcut button ──────────────────────────────────────────────────────── */
typedef struct { void (*cb)(const gchar*, gpointer); gpointer ud; const gchar *url; } NavClosure;

static void on_shortcut_clicked(GtkWidget *w G_GNUC_UNUSED, gpointer data)
{
    NavClosure *nc = (NavClosure *)data;
    nc->cb(nc->url, nc->ud);
}

static GtkWidget *make_shortcut_btn(const Shortcut *sc,
                                    void (*navigate_cb)(const gchar*, gpointer),
                                    gpointer user_data)
{
    GtkWidget *btn  = gtk_button_new();
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

    /* Ícone */
    GtkWidget *icon_lbl = gtk_label_new(sc->icon);
    gchar *icon_markup = g_strdup_printf(
        "<span font_desc='Sans 18' foreground='#c9d1e0'>%s</span>", sc->icon);
    gtk_label_set_markup(GTK_LABEL(icon_lbl), icon_markup);
    g_free(icon_markup);

    /* Nome */
    GtkWidget *name_lbl = gtk_label_new(NULL);
    gchar *name_markup = g_strdup_printf(
        "<span font_desc='Monospace 8' foreground='#4a5068'>%s</span>", sc->label);
    gtk_label_set_markup(GTK_LABEL(name_lbl), name_markup);
    g_free(name_markup);

    gtk_box_pack_start(GTK_BOX(vbox), icon_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), name_lbl, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn), vbox);

    /* Estilo do botão */
    GtkStyleContext *ctx = gtk_widget_get_style_context(btn);
    gtk_style_context_add_class(ctx, "shortcut-btn");

    /* Closure: alocada e liberada com o botão */
    NavClosure *nc = g_new(NavClosure, 1);
    nc->cb  = navigate_cb;
    nc->ud  = user_data;
    nc->url = sc->url;  
    g_object_set_data_full(G_OBJECT(btn), "nav_closure", nc, g_free);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_shortcut_clicked), nc);

    return btn;
}

/* ── CSS do tema ──────────────────────────────────────────────────────────── */
static void apply_homepage_css(void)
{
    static gboolean done = FALSE;
    if (done) return;
    done = TRUE;

    const gchar *css =
        /* Área da homepage */
        ".homepage-root {"
        "  background-color: #0b0c10;"
        "}"
        /* Badge do motor de busca */
        ".engine-badge {"
        "  background: #13151c;"
        "  border-radius: 999px;"
        "  border: 1px solid #1e2130;"
        "  padding: 4px 14px;"
        "}"
        ".engine-badge label {"
        "  color: #5e9bff;"
        "  font-size: 11px;"
        "}"
        /* Botões de atalho */
        ".shortcut-btn {"
        "  background: #13151c;"
        "  border: 1px solid #1e2130;"
        "  border-radius: 12px;"
        "  padding: 8px;"
        "  min-width: 100px;"
        "  min-height: 70px;"
        "}"
        ".shortcut-btn:hover {"
        "  background: #1e2133;"
        "  border-color: #5e9bff;"
        "}"
        ".shortcut-btn:active {"
        "  background: #161926;"
        "}"
        /* Rodapé */
        ".hp-footer {"
        "  color: #2e3350;"
        "  font-size: 10px;"
        "}";

    GtkCssProvider *prov = gtk_css_provider_new();
    gtk_css_provider_load_from_data(prov, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(prov),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(prov);
}

/* ── API pública ──────────────────────────────────────────────────────────── */

GtkWidget *homepage_widget_new(void (*navigate_cb)(const gchar *url, gpointer data),
                               gpointer user_data)
{
    apply_homepage_css();

    /* Raiz com fundo escuro */
    GtkWidget *root = gtk_event_box_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(root), "homepage-root");

    /* Coluna principal centralizada via halign/valign no EventBox */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(vbox,    40);
    gtk_widget_set_margin_bottom(vbox, 40);
    gtk_widget_set_margin_start(vbox,  40);
    gtk_widget_set_margin_end(vbox,    40);
    gtk_container_add(GTK_CONTAINER(root), vbox);

    /* Relógio */
    GtkWidget *clock_label = gtk_label_new(NULL);
    gtk_widget_set_halign(clock_label, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), clock_label, FALSE, FALSE, 0);

    /* Data */
    GtkWidget *date_label = gtk_label_new(NULL);
    gtk_widget_set_halign(date_label, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), date_label, FALSE, FALSE, 0);

    /* Badge do motor de busca */
    GtkWidget *badge_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(badge_box, GTK_ALIGN_CENTER);
    gtk_style_context_add_class(gtk_widget_get_style_context(badge_box), "engine-badge");

    GtkWidget *engine_label = gtk_label_new("🦆 DuckDuckGo ativo");
    gtk_box_pack_start(GTK_BOX(badge_box), engine_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), badge_box, FALSE, FALSE, 0);

    /* Grade de atalhos 3×2 */
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);

    for (int i = 0; i < N_SHORTCUTS; i++) {
        GtkWidget *btn = make_shortcut_btn(&SHORTCUTS[i], navigate_cb, user_data);
        gtk_grid_attach(GTK_GRID(grid), btn, i % 3, i / 3, 1, 1);
    }
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 0);


    /* Estado interno */
    HomepageData *hd    = g_new0(HomepageData, 1);
    hd->clock_label     = clock_label;
    hd->date_label      = date_label;
    hd->engine_label    = engine_label;

    g_object_set_data(G_OBJECT(root), "hp_data", hd);
    g_signal_connect(root, "destroy", G_CALLBACK(on_homepage_destroy), hd);

    /* Primeira atualização imediata */
    update_clock(hd);

    /* Timer de 1 s */
    hd->tick_id = g_timeout_add_seconds(1, homepage_widget_tick, root);

    gtk_widget_show_all(root);
    return root;
}

void homepage_widget_update_engine(GtkWidget *homepage, SearchEngine engine)
{
    HomepageData *hd = (HomepageData *)g_object_get_data(G_OBJECT(homepage), "hp_data");
    if (!hd) return;

    const SearchEngineInfo *eng = &SEARCH_ENGINES[engine];
    gchar *text = g_strdup_printf("%s %s ativo", eng->icon, eng->name);
    gtk_label_set_text(GTK_LABEL(hd->engine_label), text);
    g_free(text);
}