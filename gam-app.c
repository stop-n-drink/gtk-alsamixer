/*
 *  (gtk-alsamixer) An ALSA mixer for GTK
 *
 *  Copyright (C) 2001-2005 Derrick J Houy <djhouy@paw.za.org>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtklabel.h>
#include <gtk/gtknotebook.h>
#include <glib/gi18n.h>

#include "gam-app.h"
#include "gam-mixer.h"
#include "gam-prefs-dlg.h"

#define GAM_APP_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GAM_TYPE_APP, GamAppPrivate))

typedef struct _GamAppPrivate GamAppPrivate;

struct _GamAppPrivate
{
    GtkWidget      *status_bar;
    GtkWidget      *notebook;
    GtkUIManager   *ui_manager;
    GtkAccelGroup  *ui_accel_group;
    GtkActionGroup *main_action_group;
    gint            num_cards;
    guint           tip_message_cid;
};

static void      gam_app_class_init                    (GamAppClass           *klass);
static void      gam_app_init                          (GamApp                *gam_app);
static gboolean  gam_app_delete                        (GtkWidget             *widget,
                                                        gpointer               user_data);
static void      gam_app_destroy                       (GtkObject             *object);
static GObject  *gam_app_constructor                   (GType                  type,
                                                        guint                  n_construct_properties,
                                                        GObjectConstructParam *construct_params);
static void      gam_app_load_prefs                    (GamApp                *gam_app);
static void      gam_app_save_prefs                    (GamApp                *gam_app);
static GamMixer *gam_app_get_current_mixer             (GamApp                *gam_app);
static void      gam_app_quit_cb                       (GtkWidget             *widget,
                                                        GamApp                *gam_app);
static void      gam_app_about_cb                      (GtkWidget             *widget,
                                                        gpointer               data);
static void      gam_app_mixer_display_name_changed_cb (GamMixer              *gam_mixer,
                                                        GamApp                *gam_app);
static void      gam_app_mixer_visibility_changed_cb   (GamMixer              *gam_mixer,
                                                        GamApp                *gam_app);
static void      gam_app_ui_connect_proxy_cb           (GtkUIManager          *manager,
                                                        GtkAction             *action,
                                                        GtkWidget             *proxy,
                                                        GamApp                *gam_app);
static void      gam_app_ui_disconnect_proxy_cb        (GtkUIManager          *manager,
                                                        GtkAction             *action,
                                                        GtkWidget             *proxy,
                                                        GamApp                *gam_app);

static gpointer parent_class;


GType
gam_app_get_type (void)
{
    static GType gam_app_type = 0;

    if (!gam_app_type) {
        static const GTypeInfo gam_app_info =
        {
            sizeof (GamAppClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gam_app_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (GamApp),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gam_app_init,
        };

        gam_app_type = g_type_register_static (GTK_TYPE_WINDOW, "GamApp",
                                               &gam_app_info, (GTypeFlags)0);
    }

    return gam_app_type;
}

static void
gam_app_class_init (GamAppClass *klass)
{
    GObjectClass *gobject_class;
    GtkObjectClass *object_class;

    gobject_class = G_OBJECT_CLASS (klass);
    object_class = GTK_OBJECT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    gobject_class->constructor = gam_app_constructor;

    object_class->destroy = gam_app_destroy;

    g_type_class_add_private (gobject_class, sizeof (GamAppPrivate));
}

static void
gam_app_init (GamApp *gam_app)
{
    GamAppPrivate *priv;

    g_return_if_fail (GAM_IS_APP (gam_app));

    priv = GAM_APP_GET_PRIVATE (gam_app);

//    priv->ui_manager = gtk_ui_manager_new ();
//    priv->ui_accel_group = gtk_ui_manager_get_accel_group (priv->ui_manager);

    priv->main_action_group = gtk_action_group_new ("MainActions");

#ifdef ENABLE_NLS
    gtk_action_group_set_translation_domain (priv->main_action_group, GETTEXT_PACKAGE);
#endif

//    priv->status_bar = gtk_statusbar_new ();
    priv->tip_message_cid = gtk_statusbar_get_context_id (GTK_STATUSBAR (priv->status_bar),
                                                          "GamAppToolTips");

    priv->notebook = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (priv->notebook), TRUE);
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (priv->notebook), GTK_POS_TOP);
}

static gboolean
gam_app_delete (GtkWidget *widget, gpointer user_data)
{
#ifdef DEBUG
    g_message ("%s - %d: gam_app_delete", __FILE__, __LINE__);
#endif

    GamApp *gam_app;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GAM_IS_APP (widget), FALSE);

    gam_app = GAM_APP (widget);

    gam_app_save_prefs (gam_app);

    return FALSE;
}

static void
gam_app_destroy (GtkObject *object)
{
#ifdef DEBUG
    g_message ("%s - %d: gam_app_destroy", __FILE__, __LINE__);
#endif

    GamApp *gam_app;
    GamAppPrivate *priv;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GAM_IS_APP (object));

    gam_app = GAM_APP (object);

    priv = GAM_APP_GET_PRIVATE (gam_app);

    gtk_main_quit ();

    priv->status_bar = NULL;
    priv->notebook = NULL;

    gtk_container_foreach (GTK_CONTAINER (gam_app), (GtkCallback) gtk_widget_destroy, NULL);
}

static GObject *
gam_app_constructor (GType                  type,
                     guint                  n_construct_properties,
                     GObjectConstructParam *construct_params)
{
    GObject *object;
    GamApp *gam_app;
    GamAppPrivate *priv;
    GtkWidget *main_box, *mixer, *label;
    GError *error;
    snd_ctl_t *ctl_handle;
    gint result, index = 0;
    gchar *card;

    object = (* G_OBJECT_CLASS (parent_class)->constructor) (type,
                                                             n_construct_properties,
                                                             construct_params);

    gam_app = GAM_APP (object);

    priv = GAM_APP_GET_PRIVATE (gam_app);

    g_signal_connect (G_OBJECT (gam_app), "delete_event",
                      G_CALLBACK (gam_app_delete), NULL);

    // Build the main menu and toolbar
/*    gtk_action_group_add_actions (priv->main_action_group, action_entries,
                                  G_N_ELEMENTS (action_entries), gam_app); */

//    gtk_ui_manager_insert_action_group (priv->ui_manager, priv->main_action_group, 0);

    error = NULL;
/*
    g_signal_connect (G_OBJECT (priv->ui_manager), "connect_proxy",
                      G_CALLBACK (gam_app_ui_connect_proxy_cb), gam_app);
    g_signal_connect (G_OBJECT (priv->ui_manager), "disconnect_proxy",
                      G_CALLBACK (gam_app_ui_disconnect_proxy_cb), gam_app);
*/
    do {
        card = g_strdup_printf ("hw:%d", index++);

        result = snd_ctl_open (&ctl_handle, card, 0);

        if (result == 0) {
            snd_ctl_close(ctl_handle);

            mixer = gam_mixer_new (gam_app, card);

            if (gam_mixer_get_visible (GAM_MIXER (mixer)))
                gtk_widget_show (mixer);

            g_signal_connect (G_OBJECT (mixer), "display_name_changed",
                              G_CALLBACK (gam_app_mixer_display_name_changed_cb), gam_app);

            g_signal_connect (G_OBJECT (mixer), "visibility_changed",
                              G_CALLBACK (gam_app_mixer_visibility_changed_cb), gam_app);

            label = gtk_label_new (gam_mixer_get_display_name (GAM_MIXER (mixer)));
            gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

            gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), mixer, label);
        }

        g_free (card);
    } while (result == 0);

    priv->num_cards = index - 1;

    // Pack widgets into window
    main_box = gtk_vbox_new (FALSE, 0);

    gtk_container_add (GTK_CONTAINER (gam_app), main_box);
/*    gtk_box_pack_start (GTK_BOX (main_box), gtk_ui_manager_get_widget (priv->ui_manager, "/MainMenu"),
                        FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (main_box), priv->status_bar,
                      FALSE, FALSE, 0); */

    gtk_widget_show_all (GTK_WIDGET (main_box));

    gtk_box_pack_start (GTK_BOX (main_box), priv->notebook, TRUE, TRUE, 0);

    gtk_widget_show (priv->notebook);

    gam_app_load_prefs (gam_app);

    return object;
}

static void
gam_app_load_prefs (GamApp *gam_app)
{
    GamAppPrivate *priv;
    gint height, width;

    g_return_if_fail (GAM_IS_APP (gam_app));

    priv = GAM_APP_GET_PRIVATE (gam_app);

    if ((height != 0) && (width != 0))
        gtk_window_resize (GTK_WINDOW (gam_app), width, height);
    else /* This is really pedantic, since it is very unlikely to ever happen */
        gtk_window_set_default_size (GTK_WINDOW (gam_app), 480, 350);
}

static void
gam_app_save_prefs (GamApp *gam_app)
{
    GamAppPrivate *priv;
    gint height, width;

    g_return_if_fail (GAM_IS_APP (gam_app));

    priv = GAM_APP_GET_PRIVATE (gam_app);

    gdk_window_get_geometry (GDK_WINDOW (GTK_WIDGET (gam_app)->window), NULL, NULL, &width, &height, NULL);

}

static GamMixer *
gam_app_get_current_mixer (GamApp *gam_app)
{
    GamAppPrivate *priv;
    GtkWidget *mixer;

    g_return_val_if_fail (GAM_IS_APP (gam_app), NULL);

    priv = GAM_APP_GET_PRIVATE (gam_app);

    mixer = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook),
              gtk_notebook_get_current_page (GTK_NOTEBOOK (priv->notebook)));

    return (GAM_MIXER (mixer));
}

static void
gam_app_quit_cb (GtkWidget *widget, GamApp *gam_app)
{
#ifdef DEBUG
    g_message ("%s - %d: gam_app_quit_cb", __FILE__, __LINE__);
#endif

    g_return_if_fail (GAM_IS_APP (gam_app));

    if (!gam_app_delete (GTK_WIDGET (gam_app), NULL)) {
#ifdef DEBUG
    g_message ("%s - %d: gam_app deleted, calling gtk_widget_destroy", __FILE__, __LINE__);
#endif
        gtk_widget_destroy (GTK_WIDGET (gam_app));
    }
}

static void
gam_app_about_cb (GtkWidget *widget, gpointer data)
{
}

static void
gam_app_preferences_cb (GtkMenuItem *menuitem, GamApp *gam_app)
{
    static GtkWidget *dialog = NULL;

    if (dialog != NULL) {
        gtk_window_present (GTK_WINDOW (dialog));
        gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                      GTK_WINDOW (gam_app));

        return;
    }

/*    dialog = gam_prefs_dlg_new (GTK_WINDOW (gam_app));

    g_signal_connect (G_OBJECT (dialog), "destroy",
                      G_CALLBACK (gtk_widget_destroyed), &dialog);

    gtk_widget_show (dialog); */
}

static void
gam_app_properties_cb (GtkMenuItem *menuitem, GamApp *gam_app)
{
    gam_mixer_show_props_dialog (gam_app_get_current_mixer (gam_app));
}

static void
gam_app_mixer_display_name_changed_cb (GamMixer *gam_mixer, GamApp *gam_app)
{
    GamAppPrivate *priv;

    g_return_if_fail (GAM_IS_APP (gam_app));
    g_return_if_fail (GAM_IS_MIXER (gam_mixer));

    priv = GAM_APP_GET_PRIVATE (gam_app);

    gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (priv->notebook),
                                     GTK_WIDGET (gam_mixer),
                                     gam_mixer_get_display_name (gam_mixer));
}

static void
gam_app_mixer_visibility_changed_cb (GamMixer *gam_mixer, GamApp *gam_app)
{
    if (gam_mixer_get_visible (gam_mixer))
        gtk_widget_show (GTK_WIDGET (gam_mixer));
    else
        gtk_widget_hide (GTK_WIDGET (gam_mixer));
}

static void
gam_app_menu_item_select_cb (GtkMenuItem *proxy, GamApp *gam_app)
{
    GamAppPrivate *priv;
    GtkAction *action;
    gchar *message;

/*    g_return_if_fail (GAM_IS_APP (gam_app));

    priv = GAM_APP_GET_PRIVATE (gam_app);

    action = g_object_get_data (G_OBJECT (proxy),  "gtk-action");

    g_return_if_fail (action != NULL);

    g_object_get (G_OBJECT (action), "tooltip", &message, NULL);

    if (message) {
        gtk_statusbar_push (GTK_STATUSBAR (priv->status_bar),
                            priv->tip_message_cid,
                            message);
        g_free (message);
    } */
}

static void
gam_app_menu_item_deselect_cb (GtkMenuItem *proxy, GamApp *gam_app)
{
    GamAppPrivate *priv;

    g_return_if_fail (GAM_IS_APP (gam_app));

    priv = GAM_APP_GET_PRIVATE (gam_app);

/*    gtk_statusbar_pop (GTK_STATUSBAR (priv->status_bar),
                       priv->tip_message_cid); */
}

static void
gam_app_ui_connect_proxy_cb (GtkUIManager *manager, GtkAction *action,
                             GtkWidget    *proxy,   GamApp    *gam_app)
{
    if (GTK_IS_MENU_ITEM (proxy)) {
        g_signal_connect (G_OBJECT (proxy), "select",
            G_CALLBACK (gam_app_menu_item_select_cb), gam_app);
        g_signal_connect (G_OBJECT (proxy), "deselect",
            G_CALLBACK (gam_app_menu_item_deselect_cb), gam_app);
    }
}

static void
gam_app_ui_disconnect_proxy_cb (GtkUIManager *manager, GtkAction *action,
                                GtkWidget    *proxy,   GamApp    *gam_app)
{
    if (GTK_IS_MENU_ITEM (proxy)) {
/*        g_signal_handlers_disconnect_by_func (G_OBJECT (proxy),
            G_CALLBACK (gam_app_menu_item_select_cb), gam_app); 
        g_signal_handlers_disconnect_by_func (G_OBJECT (proxy),
            G_CALLBACK (gam_app_menu_item_deselect_cb), gam_app); */
    }
}


GtkWidget *
gam_app_new (void)
{
    return g_object_new (GAM_TYPE_APP,
                         "title", _("GTK ALSA Mixer"),
                         NULL);
}

void
gam_app_run (GamApp *gam_app)
{
    gtk_widget_show (GTK_WIDGET (gam_app));
    gtk_main ();
}

gint
gam_app_get_num_cards (GamApp *gam_app)
{
    GamAppPrivate *priv;

    g_return_val_if_fail (GAM_IS_APP (gam_app), 0);

    priv = GAM_APP_GET_PRIVATE (gam_app);

    return priv->num_cards;
}

GamMixer *
gam_app_get_mixer (GamApp *gam_app, gint index)
{
    GamAppPrivate *priv;
    GtkWidget *mixer;

    g_return_val_if_fail (GAM_IS_APP (gam_app), NULL);

    priv = GAM_APP_GET_PRIVATE (gam_app);

    g_return_val_if_fail ((index >= 0) && (index <= priv->num_cards), NULL);

    mixer = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), index);

    return GAM_MIXER (mixer);
}

gint
gam_app_get_mixer_slider_style (GamApp *gam_app)
{
    gint style = 0;

    return style;
}

void
gam_app_set_mixer_slider_style (GamApp *gam_app, gint style)
{

}

gint
gam_app_get_slider_toggle_style (GamApp *gam_app)
{
    gint style = 1;


    return style;
}

void
gam_app_set_slider_toggle_style (GamApp *gam_app, gint style)
{

}
