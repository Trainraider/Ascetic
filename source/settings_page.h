#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TEMPLATE_APP_TYPE_SETTINGS_PAGE template_app_settings_page_get_type()
G_DECLARE_FINAL_TYPE(TemplateAppSettingsPage, template_app_settings_page, TEMPLATE_APP, SETTINGS_PAGE, GtkWidget)

TemplateAppSettingsPage* template_app_settings_page_new(void);

GtkWidget* template_app_settings_page_get_back_button(TemplateAppSettingsPage* self);

G_END_DECLS

#endif // SETTINGS_PAGE_H