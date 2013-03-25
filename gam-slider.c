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

#include <math.h>
#include <glib/gi18n.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkvseparator.h>
#include <gtk/gtktogglebutton.h>

#include "gam-slider.h"

enum {
    PROP_0,
    PROP_ELEM,
    PROP_MIXER,
    PROP_APP
};

enum {
    REFRESH,
    LAST_SIGNAL
};

#define GAM_SLIDER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GAM_TYPE_SLIDER, GamSliderPrivate))

typedef struct _GamSliderPrivate GamSliderPrivate;

struct _GamSliderPrivate
{
    gpointer          app;
    gpointer          mixer;
    snd_mixer_elem_t *elem;
    gchar            *name;
    gchar            *name_config;
    GtkWidget        *vbox;
    GtkWidget        *label;
    GtkWidget        *mute_button;
    GtkWidget        *capture_button;
};

static void     gam_slider_class_init                (GamSliderClass        *klass);
static void     gam_slider_init                      (GamSlider             *gam_slider);
static void     gam_slider_finalize                  (GObject               *object);
static GObject *gam_slider_constructor               (GType                  type,
                                                      guint                  n_construct_properties,
                                                      GObjectConstructParam *construct_params);
static void     gam_slider_set_property              (GObject               *object,
                                                      guint                  prop_id,
                                                      const GValue          *value,
                                                      GParamSpec            *pspec);
static void     gam_slider_get_property              (GObject               *object,
                                                      guint                  prop_id,
                                                      GValue                *value,
                                                      GParamSpec            *pspec);
static void     gam_slider_set_elem                  (GamSlider             *gam_slider,
                                                      snd_mixer_elem_t      *elem);
static gint     gam_slider_mute_button_toggled_cb    (GtkWidget             *widget,
                                                      GamSlider             *gam_slider);
static gint     gam_slider_capture_button_toggled_cb (GtkWidget             *widget,
                                                      GamSlider             *gam_slider);
static gint     gam_slider_refresh                   (snd_mixer_elem_t      *elem,
                                                      guint                  mask);
static gint     gam_slider_get_widget_position       (GamSlider             *gam_slider,
                                                      GtkWidget             *widget);

static gpointer parent_class;
static guint    signals[LAST_SIGNAL] = { 0 };

GType
gam_slider_get_type (void)
{
    static GType gam_slider_type = 0;

    if (!gam_slider_type) {
        static const GTypeInfo gam_slider_info =
        {
            sizeof (GamSliderClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gam_slider_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (GamSlider),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gam_slider_init,
        };

        gam_slider_type = g_type_register_static (GTK_TYPE_HBOX, "GamSlider",
                                                  &gam_slider_info, (GTypeFlags)0);
    }

    return gam_slider_type;
}

static void
gam_slider_class_init (GamSliderClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = gam_slider_finalize;
    gobject_class->constructor = gam_slider_constructor;
    gobject_class->set_property = gam_slider_set_property;
    gobject_class->get_property = gam_slider_get_property;

    signals[REFRESH] =
        g_signal_new ("refresh",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GamSliderClass, refresh),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

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

    g_type_class_add_private (gobject_class, sizeof (GamSliderPrivate));
}

static void
gam_slider_init (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_if_fail (GAM_IS_SLIDER (gam_slider));

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    priv->elem = NULL;
    priv->app = NULL;
    priv->mixer = NULL;
    priv->vbox = NULL;
    priv->name = NULL;
    priv->name_config = NULL;
    priv->mute_button = NULL;
    priv->capture_button = NULL;
}

static void
gam_slider_finalize (GObject *object)
{
    GamSlider *gam_slider;
    GamSliderPrivate *priv;
    
    g_return_if_fail (GAM_IS_SLIDER (object));

    gam_slider = GAM_SLIDER (object);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    snd_mixer_elem_set_callback (priv->elem, NULL);

    g_free (priv->name);
    g_free (priv->name_config);

    priv->name = NULL;
    priv->name_config = NULL;
    priv->label = NULL;
    priv->mute_button = NULL;
    priv->capture_button = NULL;
    priv->elem = NULL;
    priv->app = NULL;
    priv->mixer = NULL;
    priv->vbox = NULL;

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GObject *
gam_slider_constructor (GType                  type,
                        guint                  n_construct_properties,
                        GObjectConstructParam *construct_params)
{
    GObject *object;
    GamSlider *gam_slider;
    GamSliderPrivate *priv;
    GtkWidget *separator;
    gint value;

    object = (* G_OBJECT_CLASS (parent_class)->constructor) (type,
                                                             n_construct_properties,
                                                             construct_params);

    gam_slider = GAM_SLIDER (object);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    priv->vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (priv->vbox);

    gtk_box_pack_start (GTK_BOX (gam_slider),
                        priv->vbox, TRUE, TRUE, 0);

    separator = gtk_vseparator_new ();
    gtk_widget_show (separator);

    gtk_box_pack_start (GTK_BOX (gam_slider),
                        separator, FALSE, TRUE, 0);

    priv->label = gtk_label_new_with_mnemonic (gam_slider_get_display_name (gam_slider));
    gtk_widget_show (priv->label);

    gtk_box_pack_start (GTK_BOX (priv->vbox),
                        priv->label, FALSE, TRUE, 0);

    if (snd_mixer_selem_has_playback_switch (priv->elem)) {
        if (gam_app_get_slider_toggle_style (GAM_APP (priv->app)) == 0)
            priv->mute_button = gtk_toggle_button_new_with_label (_("Mute"));
        else
            priv->mute_button = gtk_check_button_new_with_label (_("Mute"));

        snd_mixer_selem_get_playback_switch (priv->elem, SND_MIXER_SCHN_FRONT_LEFT, &value);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->mute_button), !value);

        g_signal_connect (G_OBJECT (priv->mute_button), "toggled",
                          G_CALLBACK (gam_slider_mute_button_toggled_cb), gam_slider);
    } else
        priv->mute_button = gtk_label_new (NULL);

    gtk_widget_show (priv->mute_button);
    gtk_box_pack_start (GTK_BOX (priv->vbox),
                        priv->mute_button, FALSE, FALSE, 0);

    if (snd_mixer_selem_has_capture_switch (priv->elem)) {
        if (gam_app_get_slider_toggle_style (GAM_APP (priv->app)) == 0)
            priv->capture_button = gtk_toggle_button_new_with_label (_("Rec."));
        else
            priv->capture_button = gtk_check_button_new_with_label (_("Rec."));

        snd_mixer_selem_get_capture_switch (priv->elem, SND_MIXER_SCHN_FRONT_LEFT, &value);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->capture_button), value);

        g_signal_connect (G_OBJECT (priv->capture_button), "toggled",
                          G_CALLBACK (gam_slider_capture_button_toggled_cb), gam_slider);
    } else
        priv->capture_button = gtk_label_new (NULL);

    gtk_widget_show (priv->capture_button);
    gtk_box_pack_start (GTK_BOX (priv->vbox),
                        priv->capture_button, FALSE, FALSE, 0);

    return object;
}

static void
gam_slider_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    GamSlider *gam_slider;
    GamSliderPrivate *priv;

    gam_slider = GAM_SLIDER (object);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    switch (prop_id) {
        case PROP_ELEM:
            gam_slider_set_elem (gam_slider, g_value_get_pointer (value));
            break;
        case PROP_MIXER:
            priv->mixer = g_value_get_pointer (value);
            g_object_notify (G_OBJECT (gam_slider), "mixer");
            break;
        case PROP_APP:
            priv->app = g_value_get_pointer (value);
            g_object_notify (G_OBJECT (gam_slider), "app");
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gam_slider_get_property (GObject     *object,
                         guint        prop_id,
                         GValue      *value,
                         GParamSpec  *pspec)
{
    GamSlider *gam_slider;
    GamSliderPrivate *priv;

    gam_slider = GAM_SLIDER (object);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

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

static void
gam_slider_set_elem (GamSlider *gam_slider, snd_mixer_elem_t *elem)
{
    GamSliderPrivate *priv;

    g_return_if_fail (GAM_IS_SLIDER (gam_slider));

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    if (priv->elem)
        snd_mixer_elem_set_callback (priv->elem, NULL);

    if (elem) {
        snd_mixer_elem_set_callback_private (elem, gam_slider);
        snd_mixer_elem_set_callback (elem, gam_slider_refresh);
    }

    priv->elem = elem;

    g_object_notify (G_OBJECT (gam_slider), "elem");
}

static gint
gam_slider_mute_button_toggled_cb (GtkWidget *widget, GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    snd_mixer_selem_set_playback_switch_all (priv->elem,
                !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));

    return TRUE;
}

static gint
gam_slider_capture_button_toggled_cb (GtkWidget *widget, GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    snd_mixer_selem_set_capture_switch_all (priv->elem,
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));

    return TRUE;
}

static gint
gam_slider_get_widget_position (GamSlider *gam_slider, GtkWidget *widget)
{
    GamSliderPrivate *priv;
    GValue value = { 0, };
    gint position = -1;

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), -1);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), -1);

    g_value_init (&value, G_TYPE_INT);

    gtk_container_child_get_property (GTK_CONTAINER (priv->vbox),
                                      widget, "position", &value);

    position = g_value_get_int (&value);

    g_value_unset (&value);

    return position;
}


static gint
gam_slider_refresh (snd_mixer_elem_t *elem, guint mask)
{
    GamSlider * const gam_slider = GAM_SLIDER (snd_mixer_elem_get_callback_private (elem));
    GamSliderPrivate *priv;
    gint value;

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    if (snd_mixer_selem_has_playback_switch (priv->elem)) {
        snd_mixer_selem_get_playback_switch (priv->elem, SND_MIXER_SCHN_FRONT_LEFT, &value);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->mute_button), !value);
    }

    if (snd_mixer_selem_has_capture_switch (priv->elem)) {
        snd_mixer_selem_get_capture_switch (priv->elem, SND_MIXER_SCHN_FRONT_LEFT, &value);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->capture_button), value);
    }

    g_signal_emit (gam_slider, signals[REFRESH], 0);
    return 0;
}

const gchar *
gam_slider_get_name (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

//g_warning("lolcat");g_warning(snd_mixer_selem_get_name (priv->elem));
    return snd_mixer_selem_get_name (priv->elem);
    
}

const gchar *
gam_slider_get_config_name (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

//    if (priv->name_config == NULL) {
        priv->name_config = g_strdup (gam_slider_get_name (gam_slider));
        priv->name_config = g_strdelimit (priv->name_config, GAM_CONFIG_DELIMITERS, '_');
//    }

    return priv->name_config;
}

gchar *
gam_slider_get_display_name (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;
    gchar *key, *name, *disp_name;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    disp_name = g_strndup (gam_slider_get_name (gam_slider), 8);

//    if (name == NULL)
        name = g_strdup (disp_name);

    g_free (disp_name);

    return name;
}

void
gam_slider_set_display_name (GamSlider *gam_slider, const gchar *name)
{
    GamSliderPrivate *priv;
    gchar *key;

    g_return_if_fail (GAM_IS_SLIDER (gam_slider));

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    gtk_label_set_text_with_mnemonic (GTK_LABEL (priv->label), name);
}

gboolean
gam_slider_get_visible (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;
    gchar *key;
    gboolean visible = TRUE;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), TRUE);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    return visible;
}

void
gam_slider_set_visible (GamSlider *gam_slider, gboolean visible)
{
    GamSliderPrivate *priv;
    gchar *key;

    g_return_if_fail (GAM_IS_SLIDER (gam_slider));

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    if (visible)
        gtk_widget_show (GTK_WIDGET (gam_slider));
    else
        gtk_widget_hide (GTK_WIDGET (gam_slider));

}

snd_mixer_elem_t *
gam_slider_get_elem (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    g_return_val_if_fail (priv->elem != NULL, NULL);

    return priv->elem;
}

GtkLabel *
gam_slider_get_label_widget (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    return GTK_LABEL (priv->label);
}

GtkWidget *
gam_slider_get_mute_widget (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    return priv->mute_button;
}

GtkWidget *
gam_slider_get_capture_widget (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    return priv->capture_button;
}

GamMixer *
gam_slider_get_mixer (GamSlider *gam_slider)
{
    GamSliderPrivate *priv;

    g_return_val_if_fail (GAM_IS_SLIDER (gam_slider), NULL);

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    g_return_val_if_fail (GAM_IS_MIXER (priv->mixer), NULL);

    return priv->mixer;
}

void
gam_slider_add_pan_widget (GamSlider *gam_slider, GtkWidget *widget)
{
    GamSliderPrivate *priv;

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    gtk_box_pack_start (GTK_BOX (priv->vbox),
                        widget, FALSE, FALSE, 0);

    gtk_box_reorder_child (GTK_BOX (priv->vbox), widget,
                           gam_slider_get_widget_position (gam_slider, priv->mute_button));
}

void
gam_slider_add_volume_widget (GamSlider *gam_slider, GtkWidget *widget)
{
    GamSliderPrivate *priv;

    priv = GAM_SLIDER_GET_PRIVATE (gam_slider);

    gtk_box_pack_start (GTK_BOX (priv->vbox),
                        widget, TRUE, TRUE, 0);

    gtk_box_reorder_child (GTK_BOX (priv->vbox), widget, 1);
}
