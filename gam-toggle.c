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

#include <glib/gi18n.h>

#include "gam-toggle.h"

enum {
    PROP_0,
    PROP_ELEM,
    PROP_MIXER,
    PROP_APP
};

#define GAM_TOGGLE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GAM_TYPE_TOGGLE, GamTogglePrivate))

typedef struct _GamTogglePrivate GamTogglePrivate;

struct _GamTogglePrivate
{
    snd_mixer_elem_t *elem;
    gpointer          app;
    gpointer          mixer;
    gchar            *name_config;
};

static void     gam_toggle_class_init   (GamToggleClass        *klass);
static void     gam_toggle_init         (GamToggle             *gam_toggle);
static void     gam_toggle_finalize     (GObject               *object);
static GObject *gam_toggle_constructor  (GType                  type,
                                         guint                  n_construct_properties,
                                         GObjectConstructParam *construct_params);
static void     gam_toggle_set_property (GObject               *object,
                                         guint                  prop_id,
                                         const GValue          *value,
                                         GParamSpec            *pspec);
static void     gam_toggle_get_property (GObject               *object,
                                         guint                  prop_id,
                                         GValue                *value,
                                         GParamSpec            *pspec);
static void     gam_toggle_set_elem     (GamToggle             *gam_slider,
                                         snd_mixer_elem_t      *elem);
static gint     gam_toggle_toggled_cb   (GtkWidget             *widget,
                                         GamToggle             *gam_toggle);
static gint     gam_toggle_refresh      (snd_mixer_elem_t      *elem,
                                         guint                  mask);

static gpointer parent_class;

GType
gam_toggle_get_type (void)
{
    static GType gam_toggle_type = 0;

    if (!gam_toggle_type) {
        static const GTypeInfo gam_toggle_info =
        {
            sizeof (GamToggleClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gam_toggle_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (GamToggle),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gam_toggle_init,
        };

        gam_toggle_type = g_type_register_static (GTK_TYPE_CHECK_BUTTON, "GamToggle",
                                                  &gam_toggle_info, (GTypeFlags)0);
    }

    return gam_toggle_type;
}

static void
gam_toggle_class_init (GamToggleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = gam_toggle_finalize;
    gobject_class->constructor = gam_toggle_constructor;
    gobject_class->set_property = gam_toggle_set_property;
    gobject_class->get_property = gam_toggle_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_ELEM,
                                     g_param_spec_pointer ("elem",
                                                           _("Element"),
                                                           _("ALSA mixer element"),
                                                           (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_MIXER,
                                     g_param_spec_pointer ("mixer",
                                                           _("Mixer"),
                                                           _("Mixer"),
                                                           (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_APP,
                                     g_param_spec_pointer ("app",
                                                           _("Main Application"),
                                                           _("Main Application"),
                                                           (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_type_class_add_private (gobject_class, sizeof (GamTogglePrivate));
}

static void
gam_toggle_init (GamToggle *gam_toggle)
{
    GamTogglePrivate *priv;

    g_return_if_fail (GAM_IS_TOGGLE (gam_toggle));

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    priv->elem = NULL;
    priv->name_config = NULL;
    priv->app = NULL;
    priv->mixer = NULL;
}

static void
gam_toggle_finalize (GObject *object)
{
    GamToggle *gam_toggle;
    GamTogglePrivate *priv;

    g_return_if_fail (GAM_IS_TOGGLE (object));

    gam_toggle = GAM_TOGGLE (object);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    snd_mixer_elem_set_callback (priv->elem, NULL);

    g_free (priv->name_config);

    priv->name_config = NULL;
    priv->elem = NULL;
    priv->mixer = NULL;
    priv->app = NULL;

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GObject*
gam_toggle_constructor (GType                  type,
                        guint                  n_construct_properties,
                        GObjectConstructParam *construct_params)
{
    GObject *object;
    GamToggle *gam_toggle;

    object = (* G_OBJECT_CLASS (parent_class)->constructor) (type,
                                                             n_construct_properties,
                                                             construct_params);

    gam_toggle = GAM_TOGGLE (object);

    gtk_button_set_label (GTK_BUTTON (gam_toggle), gam_toggle_get_display_name (gam_toggle));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gam_toggle),
                                  gam_toggle_get_state (gam_toggle));

    g_signal_connect (G_OBJECT (gam_toggle), "toggled",
                      G_CALLBACK (gam_toggle_toggled_cb), gam_toggle);

    return object;
}

static void
gam_toggle_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    GamToggle *gam_toggle;
    GamTogglePrivate *priv;

    gam_toggle = GAM_TOGGLE (object);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    switch (prop_id) {
        case PROP_ELEM:
            gam_toggle_set_elem (gam_toggle, g_value_get_pointer (value));
            break;
        case PROP_MIXER:
            priv->mixer = g_value_get_pointer (value);
            g_object_notify (G_OBJECT (gam_toggle), "mixer");
            break;
        case PROP_APP:
            priv->app = g_value_get_pointer (value);
            g_object_notify (G_OBJECT (gam_toggle), "app");
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gam_toggle_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    GamToggle *gam_toggle;
    GamTogglePrivate *priv;

    gam_toggle = GAM_TOGGLE (object);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    switch (prop_id) {
        case PROP_ELEM:
            g_value_set_pointer (value, priv->elem);
            break;
        case PROP_MIXER:
            g_value_set_pointer (value, priv->mixer);
            break;
        case PROP_APP:
            g_value_set_pointer (value, priv->app);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static gint
gam_toggle_toggled_cb (GtkWidget *widget, GamToggle *gam_toggle)
{
    gam_toggle_set_state (gam_toggle, 
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));

    return TRUE;
}

static gint
gam_toggle_refresh (snd_mixer_elem_t *elem, guint mask)
{
    GamToggle * const gam_toggle = GAM_TOGGLE (snd_mixer_elem_get_callback_private (elem));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gam_toggle),
                                  gam_toggle_get_state (gam_toggle));

    return 0;
}

GtkWidget *
gam_toggle_new (gpointer elem, GamMixer *gam_mixer, GamApp *gam_app)
{
    g_return_val_if_fail (GAM_IS_MIXER (gam_mixer), NULL);

    return g_object_new (GAM_TYPE_TOGGLE,
                         "elem", elem,
                         "mixer", gam_mixer,
                         "app", gam_app,
                         "use_underline", TRUE,
                         NULL);
}

snd_mixer_elem_t *
gam_toggle_get_elem (GamToggle *gam_toggle)
{
    GamTogglePrivate *priv;

    g_return_val_if_fail (GAM_IS_TOGGLE (gam_toggle), NULL);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    return priv->elem;
}

void
gam_toggle_set_elem (GamToggle *gam_toggle, snd_mixer_elem_t *elem)
{
    GamTogglePrivate *priv;

    g_return_if_fail (GAM_IS_TOGGLE (gam_toggle));

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    if (priv->elem)
        snd_mixer_elem_set_callback (priv->elem, NULL);

    if (elem) {
        snd_mixer_elem_set_callback_private (elem, gam_toggle);
        snd_mixer_elem_set_callback (elem, gam_toggle_refresh);
    }

    priv->elem = elem;

    g_object_notify (G_OBJECT (gam_toggle), "elem");
}

gboolean
gam_toggle_get_state (GamToggle *gam_toggle)
{
    GamTogglePrivate *priv;
    gint value = 0;

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    if (snd_mixer_selem_has_playback_switch (priv->elem)) {
        snd_mixer_selem_get_playback_switch (priv->elem, 0, &value);
    } else if (snd_mixer_selem_has_capture_switch (priv->elem)) {
        snd_mixer_selem_get_capture_switch (priv->elem, 0, &value);
    } else {
        g_warning ("%s (). No idea what to do for mixer element \"%s\"!",
                   __FUNCTION__, snd_mixer_selem_get_name (priv->elem));
    }

    return value;
}

void
gam_toggle_set_state (GamToggle *gam_toggle, gboolean state)
{
    GamTogglePrivate *priv;
    const gboolean internal_state = gam_toggle_get_state (gam_toggle);
    int err;

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    if (state == internal_state) return;

    if (snd_mixer_selem_has_playback_switch (priv->elem)) {
        err = snd_mixer_selem_set_playback_switch_all (priv->elem, state);
    } else if (snd_mixer_selem_has_capture_switch (priv->elem)) {
        err = snd_mixer_selem_set_capture_switch_all (priv->elem, state);
    } else {
        g_warning ("%s (). No idea what to do for mixer element \"%s\"!",
                   __FUNCTION__, snd_mixer_selem_get_name (priv->elem));
        err = 0;
    }

    if (err)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gam_toggle),
                                      internal_state);
}

const gchar *
gam_toggle_get_name (GamToggle *gam_toggle)
{
    GamTogglePrivate *priv;

    g_return_val_if_fail (GAM_IS_TOGGLE (gam_toggle), NULL);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    return snd_mixer_selem_get_name (priv->elem);
}

const gchar *
gam_toggle_get_config_name (GamToggle *gam_toggle)
{
    GamTogglePrivate *priv;

    g_return_val_if_fail (GAM_IS_TOGGLE (gam_toggle), NULL);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

//    if (priv->name_config == NULL) {
        priv->name_config = g_strdup (gam_toggle_get_name (gam_toggle));
        priv->name_config = g_strdelimit (priv->name_config, GAM_CONFIG_DELIMITERS, '_');
//    }

    return priv->name_config;
}

gchar *
gam_toggle_get_display_name (GamToggle *gam_toggle)
{
    GamTogglePrivate *priv;
    gchar *key, *name;

    g_return_val_if_fail (GAM_IS_TOGGLE (gam_toggle), NULL);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

//    return name == NULL ? g_strdup (gam_toggle_get_name (gam_toggle)) : name;
	return g_strdup (gam_toggle_get_name (gam_toggle));
}

void
gam_toggle_set_display_name (GamToggle *gam_toggle, const gchar *name)
{
    GamTogglePrivate *priv;
    gchar *key;

    g_return_if_fail (GAM_IS_TOGGLE (gam_toggle));

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);

    gtk_button_set_label (GTK_BUTTON (gam_toggle), name);
}

gboolean
gam_toggle_get_visible (GamToggle *gam_toggle)
{
    GamTogglePrivate *priv;
    gchar *key;
    gboolean visible = TRUE;

    g_return_val_if_fail (GAM_IS_TOGGLE (gam_toggle), TRUE);

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);


    return visible;
}

void
gam_toggle_set_visible (GamToggle *gam_toggle, gboolean visible)
{
    GamTogglePrivate *priv;
    gchar *key;

    g_return_if_fail (GAM_IS_TOGGLE (gam_toggle));

    priv = GAM_TOGGLE_GET_PRIVATE (gam_toggle);


    if (visible)
        gtk_widget_show (GTK_WIDGET (gam_toggle));
    else
        gtk_widget_hide (GTK_WIDGET (gam_toggle));
}
