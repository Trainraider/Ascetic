#include "app_window.h"
#include "globals.h"
#include "webview.h"
#include "settings_page.h"
#include "uri.h"
#include "defer.h"
#include <adwaita.h>
#include <webkit/webkit.h>

struct _AsceticAppWindow {
        AdwApplicationWindow parent_instance;
        GtkStack*            stack_main;
        AdwTabOverview*      web_page;
        AsceticSettingsPage* settings_page;
        AdwTabView*          web_tab_view;
        AdwTabBar*           web_tab_bar;
        WebKitWebView*       active_web_view;
        GtkEntry*            url_entry;
        GtkRevealer*         revealer_main_toolbar;
        GtkButton*           open_settings_button;
        GtkButton*           upper_new_tab_button;
};

GObject* _builder_get_object(GtkBuilder* builder, char* name)
{
        GObject* obj = gtk_builder_get_object(builder, name);
#if DEBUG
        if (!obj) {
                g_printerr("Error: Failed to get %s from builder", name);
                g_application_quit(G_APPLICATION(app));
        }
#endif
        return obj;
}

#define builder_get_object(builder, TYPE, name) TYPE(_builder_get_object(builder, name))

G_DEFINE_TYPE(AsceticAppWindow, ascetic_app_window, ADW_TYPE_APPLICATION_WINDOW)

enum {
        BECAME_ACTIVE,
        WINDOW_CREATED,
        LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void ascetic_app_window_dispose(GObject* object);

void show_tab_overview(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        AsceticAppWindow* window = ASCETIC_APP_WINDOW(user_data);
        adw_tab_overview_set_open(window->web_page, TRUE);
}

void on_back_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        AsceticAppWindow* window          = ASCETIC_APP_WINDOW(user_data);
        WebKitWebView*    active_web_view = window->active_web_view;
        if (active_web_view)
                webkit_web_view_go_back(active_web_view);
}

void on_forward_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        AsceticAppWindow* window          = ASCETIC_APP_WINDOW(user_data);
        WebKitWebView*    active_web_view = window->active_web_view;
        if (active_web_view)
                webkit_web_view_go_forward(active_web_view);
}

void on_refresh_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        AsceticAppWindow* window          = ASCETIC_APP_WINDOW(user_data);
        WebKitWebView*    active_web_view = window->active_web_view;
        if (active_web_view)
                webkit_web_view_reload(active_web_view);
}

void on_open_settings_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        AsceticAppWindow* window        = ASCETIC_APP_WINDOW(user_data);
        GtkStack*         stack         = window->stack_main;
        GtkWidget*        settings_page = GTK_WIDGET(window->settings_page);
        gtk_stack_set_visible_child(stack, settings_page);
        gtk_widget_set_visible(GTK_WIDGET(window->web_tab_bar), FALSE);
}

void on_close_settings(AsceticSettingsPage* settings, gpointer user_data)
{
        (void)settings;
        AsceticAppWindow* window   = ASCETIC_APP_WINDOW(user_data);
        GtkStack*         stack    = window->stack_main;
        GtkWidget*        web_page = GTK_WIDGET(window->web_page);
        gtk_stack_set_visible_child(stack, web_page);
        gtk_widget_set_visible(GTK_WIDGET(window->web_tab_bar), TRUE);
}

void load_url_from_entry(GtkWidget* entry, gpointer user_data)
{
        S_
                AsceticAppWindow* window          = ASCETIC_APP_WINDOW(user_data);
                WebKitWebView*    active_web_view = window->active_web_view;
                if (!active_web_view)
                        return;
                const char* url        = gtk_editable_get_text(GTK_EDITABLE(entry));
                ParsedUri   parsed_uri = uri_parse(url);
                defer(dg_free, parsed_uri.str);
                gtk_editable_set_text(GTK_EDITABLE(entry), parsed_uri.str);
                if (parsed_uri.is_uri) {
                        webkit_web_view_load_uri(active_web_view, parsed_uri.str);
                } else {
                        S_
                                char* search_url = str_to_brave_search_url(parsed_uri.str);
                                defer(dg_free, search_url);
                                webkit_web_view_load_uri(active_web_view, search_url);
                        _S
                }
        _S
}

void on_webview_uri_changed(WebKitWebView* webview, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        AsceticAppWindow* window       = ASCETIC_APP_WINDOW(user_data);
        AdwTabView*       tab_view     = window->web_tab_view;
        GtkEntry*         url_entry    = window->url_entry;
        AdwTabPage*       tab          = adw_tab_view_get_page(tab_view, GTK_WIDGET(webview));
        AdwTabPage*       selected_tab = adw_tab_view_get_selected_page(tab_view);
        if (tab != selected_tab)
                return;
        if (gtk_widget_has_focus(GTK_WIDGET(url_entry)))
                return;
        const char*     uri          = webkit_web_view_get_uri(webview);
        GtkEntryBuffer* entry_buffer = gtk_entry_get_buffer(url_entry);
        if (uri) {
                gtk_entry_buffer_set_text(entry_buffer, uri, -1);
        } else {
                gtk_entry_buffer_set_text(entry_buffer, "", -1);
        }
}

void on_webview_enter_fullscreen(WebKitWebView* webview, gpointer user_data)
{
        (void)webview;
        AsceticAppWindow* window = ASCETIC_APP_WINDOW(user_data);
        gtk_revealer_set_reveal_child(window->revealer_main_toolbar, FALSE);
}

void on_webview_leave_fullscreen(WebKitWebView* webview, gpointer user_data)
{
        (void)webview;
        AsceticAppWindow* window = ASCETIC_APP_WINDOW(user_data);
        gtk_revealer_set_reveal_child(window->revealer_main_toolbar, TRUE);
}

WebKitWebView* on_create_web_view(WebKitWebView* webview, WebKitNavigationAction* navigation_action, gpointer user_data)
{
        (void)navigation_action;
        AsceticAppWindow* window = ASCETIC_APP_WINDOW(user_data);
        AdwTabPage*       tab    = new_tab(GTK_WIDGET(webview), window);
        return tab_get_webview(tab);
}

AdwTabPage* new_tab(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        AsceticAppWindow* window          = ASCETIC_APP_WINDOW(user_data);
        AdwTabView*       tab_view        = window->web_tab_view;
        GtkEntry*         url_entry       = window->url_entry;
        WebKitWebView*    related_webview = NULL;

        if (G_TYPE_CHECK_INSTANCE_TYPE(widget, WEBKIT_TYPE_WEB_VIEW)) {
                related_webview = WEBKIT_WEB_VIEW(widget);
        }
        WebKitWebView* webview = create_webview(related_webview);
        AdwTabPage*    tab     = adw_tab_view_append(tab_view, GTK_WIDGET(webview));
        adw_tab_page_set_title(tab, "New Tab");
        adw_tab_page_set_icon(tab, new_tab_icon);
        g_signal_connect(webview, "notify::title", G_CALLBACK(on_webview_title_changed), tab);
        g_signal_connect(webview, "notify::favicon", G_CALLBACK(on_webview_favicon_changed), tab);
        g_signal_connect(webview, "notify::uri", G_CALLBACK(on_webview_uri_changed), window);
        g_signal_connect(webview, "enter-fullscreen", G_CALLBACK(on_webview_enter_fullscreen), window);
        g_signal_connect(webview, "leave-fullscreen", G_CALLBACK(on_webview_leave_fullscreen), window);
        g_signal_connect(webview, "create", G_CALLBACK(on_create_web_view), window);
        if (G_TYPE_CHECK_INSTANCE_TYPE(widget, GTK_TYPE_BUTTON)) {
                gtk_widget_grab_focus(GTK_WIDGET(url_entry));
        }
        return tab;
}

void on_tab_changed(GObject* self, GParamSpec* pspec, gpointer user_data)
{
        (void)self;
        (void)pspec;
        AsceticAppWindow* window        = ASCETIC_APP_WINDOW(user_data);
        AdwTabPage*       selected_page = adw_tab_view_get_selected_page(window->web_tab_view);
        if (!selected_page) {
                window->active_web_view      = NULL;
                GtkEntryBuffer* entry_buffer = gtk_entry_get_buffer(window->url_entry);
                gtk_entry_buffer_set_text(entry_buffer, "", -1);
                return;
        }
        window->active_web_view = tab_get_webview(selected_page);
        // update url entry
        const char*     uri          = webkit_web_view_get_uri(window->active_web_view);
        GtkEntryBuffer* entry_buffer = gtk_entry_get_buffer(window->url_entry);
        if (uri) {
                gtk_entry_buffer_set_text(entry_buffer, uri, -1);
        } else {
                gtk_entry_buffer_set_text(entry_buffer, "", -1);
        }
}

void on_tab_page_attached(AdwTabView* self, AdwTabPage* page, gint position, gpointer user_data)
{
        (void)user_data;
        (void)position;
        adw_tab_view_set_selected_page(self, page);
}

AdwTabView* on_tab_create_window(AdwTabView* self, gpointer user_data)
{
        (void)self;
        (void)user_data;
        AsceticAppWindow* window = ascetic_app_window_new();
        gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
        gtk_window_present(GTK_WINDOW(window));
        g_signal_emit(ASCETIC_APP_WINDOW(gtk_widget_get_root(GTK_WIDGET(self))), signals[WINDOW_CREATED], 0, window);
        return window->web_tab_view;
}

void on_tab_bar_visibility_changed(AdwTabBar* tab_bar, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        GtkWidget* upper_new_tab_button = GTK_WIDGET(user_data);
        gboolean   tabs_revealed        = adw_tab_bar_get_tabs_revealed(tab_bar);
        if (tabs_revealed) {
                gtk_widget_set_opacity(upper_new_tab_button, 0.0);
                gtk_widget_set_sensitive(upper_new_tab_button, FALSE);
        } else {
                gtk_widget_set_opacity(upper_new_tab_button, 1.0);
                gtk_widget_set_sensitive(upper_new_tab_button, TRUE);
        }
}

static void ascetic_app_window_class_init(AsceticAppWindowClass* klass)
{
        G_OBJECT_CLASS(klass)->dispose = ascetic_app_window_dispose;
        signals[BECAME_ACTIVE]         = g_signal_new(
            "became-active",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            0);
        signals[WINDOW_CREATED]        = g_signal_new(
            "window-created",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_LAST,
            0,
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            1,
            ASCETIC_TYPE_APP_WINDOW
        );
        gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), APP_PREFIX "/app_window.ui");
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, stack_main);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, web_page);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, settings_page);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, web_tab_view);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, web_tab_bar);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, url_entry);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, revealer_main_toolbar);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, open_settings_button);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, upper_new_tab_button);
        GtkBuilderScope* scope = gtk_builder_cscope_new();
        gtk_builder_cscope_add_callback(scope, on_tab_bar_visibility_changed);
        gtk_builder_cscope_add_callback(scope, show_tab_overview);
        gtk_builder_cscope_add_callback(scope, on_back_button_clicked);
        gtk_builder_cscope_add_callback(scope, on_forward_button_clicked);
        gtk_builder_cscope_add_callback(scope, on_refresh_button_clicked);
        gtk_builder_cscope_add_callback(scope, load_url_from_entry);
        gtk_builder_cscope_add_callback(scope, on_open_settings_button_clicked);
        gtk_builder_cscope_add_callback(scope, on_close_settings);
        gtk_builder_cscope_add_callback(scope, on_webview_uri_changed);
        gtk_builder_cscope_add_callback(scope, on_webview_enter_fullscreen);
        gtk_builder_cscope_add_callback(scope, on_webview_leave_fullscreen);
        gtk_builder_cscope_add_callback(scope, on_create_web_view);
        gtk_builder_cscope_add_callback(scope, new_tab);
        gtk_builder_cscope_add_callback(scope, on_tab_page_attached);
        gtk_builder_cscope_add_callback(scope, on_tab_changed);
        gtk_builder_cscope_add_callback(scope, on_tab_create_window);
        gtk_widget_class_set_template_scope(GTK_WIDGET_CLASS(klass), scope);
        g_object_unref(scope);
}

void on_active_changed(AsceticAppWindow* self, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        (void)user_data;
        if (gtk_window_is_active(GTK_WINDOW(self))) {
                g_signal_emit(self, signals[BECAME_ACTIVE], 0);
        }
}

static void ascetic_app_window_init(AsceticAppWindow* self)
{
        gtk_widget_init_template(GTK_WIDGET(self));
        g_signal_connect(self, "notify::is-active", G_CALLBACK(on_active_changed), NULL);
}

static void ascetic_app_window_dispose(GObject* object)
{
        gtk_widget_dispose_template(GTK_WIDGET(object), ASCETIC_TYPE_APP_WINDOW);
        G_OBJECT_CLASS(ascetic_app_window_parent_class)->dispose(object);
}

AsceticAppWindow* ascetic_app_window_new(void)
{
        return g_object_new(ASCETIC_TYPE_APP_WINDOW, NULL);
}