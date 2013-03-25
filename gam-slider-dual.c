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
#include <gtk/gtkvscale.h>

#include "gam-slider-dual.h"

#define GAM_SLIDER_DUAL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GAM_TYPE_SLIDER_DUAL, GamSliderDualPrivate))

typedef struct _GamSliderDualPrivate GamSliderDualPrivate;

struct _GamSliderDualPrivate
{
    gpointer   app;
    GtkWidget *lock_button;
    GtkWidget *vol_slider_left;
    GtkWidget *vol_slider_right;
    GtkObject *vol_adjustment_left;
    GtkObject *vol_adjustment_right;
    gdouble    pan;
    gboolean   refreshing;
};

static void     gam_slider_dual_class_init                    (GamSliderDualClass    *klass);
static void     gam_slider_dual_init                          (GamSliderDual         *gam_slider_dual);
static void     gam_slider_dual_finalize                      (GObject               *object);
static GObject *gam_slider_dual_constructor                   (GType                  type,
                                                               guint                  n_construct_properties,
                                                               GObjectConstructParam *construct_params);
static gint     gam_slider_dual_get_volume_left               (GamSliderDual         *gam_slider_dual);
static gint     gam_slider_dual_get_volume_right              (GamSliderDual         *gam_slider_dual);
static void     gam_slider_dual_update_volume_left            (GamSliderDual         *gam_slider_dual);
static void     gam_slider_dual_update_volume_right           (GamSliderDual         *gam_slider_dual);
static gint     gam_slider_dual_lock_button_toggled_cb        (GtkWidget             *widget,
                                                               GamSliderDual         *gam_slider_dual);
static gint     gam_slider_dual_volume_left_value_changed_cb  (GtkWidget             *widget,
                                                               GamSliderDual         *gam_slider_dual);
static gint     gam_slider_dual_volume_right_value_changed_cb (GtkWidget             *widget,
                                                               GamSliderDual         *gam_slider_dual);
static void     gam_slider_dual_refresh                       (GamSlider             *gam_slider);
static void     gam_slider_dual_set_pan                       (GamSliderDual         *gam_slider_dual);
static gboolean gam_slider_dual_get_locked                    (GamSliderDual         *gam_slider_dual);
static void     gam_slider_dual_set_locked                    (GamSliderDual         *gam_slider_dual,
                                                               gboolean               locked);

static gpointer parent_class;

GType
gam_slider_dual_get_type (void)
{
    static GType gam_slider_dual_type = 0;

    if (!gam_slider_dual_type) {
        static const GTypeInfo gam_slider_dual_info =
        {
            sizeof (GamSliderDualClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gam_slider_dual_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (GamSliderDual),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gam_slider_dual_init,
        };

        gam_slider_dual_type = g_type_register_static (GAM_TYPE_SLIDER, "GamSliderDual",
                                                       &gam_slider_dual_info, (GTypeFlags)0);
    }

    return gam_slider_dual_type;
}

static void
gam_slider_dual_class_init (GamSliderDualClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GamSliderClass *object_class = GAM_SLIDER_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = gam_slider_dual_finalize;
    gobject_class->constructor = gam_slider_dual_constructor;

    object_class->refresh = gam_slider_dual_refresh;

    g_type_class_add_private (gobject_class, sizeof (GamSliderDualPrivate));
}

static void
gam_slider_dual_init (GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;

    g_return_if_fail (GAM_IS_SLIDER_DUAL (gam_slider_dual));

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    g_object_get (G_OBJECT (gam_slider_dual),
                  "app", &priv->app,
                  NULL);

    priv->lock_button = NULL;
    priv->vol_slider_left = NULL;
    priv->vol_slider_right = NULL;
    priv->vol_adjustment_left = NULL;
    priv->vol_adjustment_right = NULL;
    priv->refreshing = FALSE;
}

static void
gam_slider_dual_finalize (GObject *object)
{
    GamSliderDual *gam_slider_dual;
    GamSliderDualPrivate *priv;
    
    g_return_if_fail (GAM_IS_SLIDER_DUAL (object));

    gam_slider_dual = GAM_SLIDER_DUAL (object);

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    g_object_unref (priv->app);

    priv->app = NULL;
    priv->vol_adjustment_left = NULL;
    priv->vol_adjustment_right = NULL;
    priv->vol_slider_left = NULL;
    priv->vol_slider_right = NULL;

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GObject *
gam_slider_dual_constructor (GType                  type,
                            guint                  n_construct_properties,
                            GObjectConstructParam *construct_params)
{
    GObject *object;
    GamSliderDual *gam_slider_dual;
    GamSliderDualPrivate *priv;
    GtkWidget *hbox;

    object = (* G_OBJECT_CLASS (parent_class)->constructor) (type,
                                                             n_construct_properties,
                                                             construct_params);

    gam_slider_dual = GAM_SLIDER_DUAL (object);

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);

    priv->vol_adjustment_left = gtk_adjustment_new (gam_slider_dual_get_volume_left (gam_slider_dual), 0, 101, 1, 5, 1);

    g_signal_connect (G_OBJECT (priv->vol_adjustment_left), "value-changed",
                      G_CALLBACK (gam_slider_dual_volume_left_value_changed_cb), gam_slider_dual);

    priv->vol_slider_left = gtk_vscale_new (GTK_ADJUSTMENT (priv->vol_adjustment_left));
    gtk_widget_show (priv->vol_slider_left);
    gtk_scale_set_draw_value (GTK_SCALE (priv->vol_slider_left), FALSE);

    gtk_box_pack_start (GTK_BOX (hbox), priv->vol_slider_left, TRUE, TRUE, 0);

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)))) {
        priv->vol_adjustment_right = gtk_adjustment_new (gam_slider_dual_get_volume_right (gam_slider_dual), 0, 101, 1, 5, 1);

        g_signal_connect (G_OBJECT (priv->vol_adjustment_right), "value-changed",
                          G_CALLBACK (gam_slider_dual_volume_right_value_changed_cb), gam_slider_dual);

        priv->vol_slider_right = gtk_vscale_new (GTK_ADJUSTMENT (priv->vol_adjustment_right));
        gtk_widget_show (priv->vol_slider_right);
        gtk_scale_set_draw_value (GTK_SCALE (priv->vol_slider_right), FALSE);

        gtk_box_pack_start (GTK_BOX (hbox), priv->vol_slider_right, TRUE, TRUE, 0);
    }

    gam_slider_add_volume_widget (GAM_SLIDER (gam_slider_dual), hbox);

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)))) {
        if (gam_app_get_slider_toggle_style (GAM_APP (priv->app)) == 0)
            priv->lock_button = gtk_toggle_button_new_with_label (_("Lock"));
        else
            priv->lock_button = gtk_check_button_new_with_label (_("Lock"));

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->lock_button),
                                      gam_slider_dual_get_locked (gam_slider_dual));

        g_signal_connect (G_OBJECT (priv->lock_button), "toggled",
                          G_CALLBACK (gam_slider_dual_lock_button_toggled_cb), gam_slider_dual);
    } else
        priv->lock_button = gtk_label_new (NULL);

    gtk_widget_show (priv->lock_button);

    gam_slider_add_pan_widget (GAM_SLIDER (gam_slider_dual), priv->lock_button);

    gtk_label_set_mnemonic_widget (gam_slider_get_label_widget (GAM_SLIDER (gam_slider_dual)),
                                   priv->vol_slider_left);

    return object;
}

static gint
gam_slider_dual_get_volume_left (GamSliderDual *gam_slider_dual)
{
    glong left_chn, pmin, pmax;

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual))))
        snd_mixer_selem_get_playback_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);
    else
        snd_mixer_selem_get_capture_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual))))
        snd_mixer_selem_get_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_LEFT, &left_chn);
    else
        snd_mixer_selem_get_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_LEFT, &left_chn);

    return rint (100 - (left_chn * (100 / (gfloat)pmax)));
}

static gint
gam_slider_dual_get_volume_right (GamSliderDual *gam_slider_dual)
{
    glong right_chn, pmin, pmax;

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual))))
        snd_mixer_selem_get_playback_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);
    else
        snd_mixer_selem_get_capture_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual))))
        snd_mixer_selem_get_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_RIGHT, &right_chn);
    else
        snd_mixer_selem_get_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_RIGHT, &right_chn);

    return rint (100 - (right_chn * (100 / (gfloat)pmax)));
}

static void
gam_slider_dual_update_volume_left (GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;
    gint left_chn = 0;
    glong pmin, pmax;

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual))))
        snd_mixer_selem_get_playback_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);
    else
        snd_mixer_selem_get_capture_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);

    left_chn = rint ((100 - GTK_ADJUSTMENT (priv->vol_adjustment_left)->value) / (100 / (gfloat)pmax));

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)))) {
        snd_mixer_selem_set_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_LEFT, left_chn);
    } else {
        snd_mixer_selem_set_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_LEFT, left_chn);
    }
}

static void
gam_slider_dual_update_volume_right (GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;
    gint right_chn = 0;
    glong pmin, pmax;

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual))))
        snd_mixer_selem_get_playback_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);
    else
        snd_mixer_selem_get_capture_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), &pmin, &pmax);

    right_chn = rint ((100 - GTK_ADJUSTMENT (priv->vol_adjustment_right)->value) / (100 / (gfloat)pmax));

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)))) {
        snd_mixer_selem_set_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_RIGHT, right_chn);
    } else {
        snd_mixer_selem_set_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)), SND_MIXER_SCHN_FRONT_RIGHT, right_chn);
    }
}

static gint
gam_slider_dual_lock_button_toggled_cb (GtkWidget *widget, GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    gam_slider_dual_set_locked (gam_slider_dual,
                                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->lock_button)));

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->lock_button)))
        gam_slider_dual_set_pan (gam_slider_dual);

    return TRUE;
}

static gint
gam_slider_dual_volume_left_value_changed_cb (GtkWidget *widget, GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    if (priv->refreshing) return TRUE;

    gam_slider_dual_update_volume_left (gam_slider_dual);

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)))) {
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->lock_button))) {
            gtk_adjustment_set_value (GTK_ADJUSTMENT (priv->vol_adjustment_right),
                                      gtk_adjustment_get_value (GTK_ADJUSTMENT (priv->vol_adjustment_left)) -
                                      priv->pan);

            gam_slider_dual_update_volume_right (gam_slider_dual);
            gam_slider_dual_set_pan (gam_slider_dual);
        }
    }

    return TRUE;
}

static gint
gam_slider_dual_volume_right_value_changed_cb (GtkWidget *widget, GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    if (priv->refreshing) return TRUE;

    gam_slider_dual_update_volume_right (gam_slider_dual);

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_dual)))) {
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->lock_button))) {
            gtk_adjustment_set_value (GTK_ADJUSTMENT (priv->vol_adjustment_left),
                                      gtk_adjustment_get_value (GTK_ADJUSTMENT (priv->vol_adjustment_right)) +
                                      priv->pan);

            gam_slider_dual_update_volume_left (gam_slider_dual);
            gam_slider_dual_set_pan (gam_slider_dual);
        }
    }

    return TRUE;
}

static void
gam_slider_dual_refresh (GamSlider *gam_slider)
{
    GamSliderDual * const gam_slider_dual = GAM_SLIDER_DUAL (snd_mixer_elem_get_callback_private (gam_slider_get_elem (gam_slider)));
    GamSliderDualPrivate *priv;

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    priv->refreshing = TRUE;

    gtk_adjustment_set_value (GTK_ADJUSTMENT (priv->vol_adjustment_left),
                              (gdouble) gam_slider_dual_get_volume_left (gam_slider_dual));

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (gam_slider))) {
        gtk_adjustment_set_value (GTK_ADJUSTMENT (priv->vol_adjustment_right),
                                  (gdouble) gam_slider_dual_get_volume_right (gam_slider_dual));
    }

    gam_slider_dual_set_pan (gam_slider_dual);

    priv->refreshing = FALSE;
}

static void
gam_slider_dual_set_pan (GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;

    g_return_if_fail (GAM_IS_SLIDER_DUAL (gam_slider_dual));

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    priv->pan = gtk_adjustment_get_value (GTK_ADJUSTMENT (priv->vol_adjustment_left)) - 
                                 gtk_adjustment_get_value (GTK_ADJUSTMENT (priv->vol_adjustment_right));
}

static gboolean
gam_slider_dual_get_locked (GamSliderDual *gam_slider_dual)
{
    GamSliderDualPrivate *priv;
    gchar *key;
    gboolean locked = TRUE;

    g_return_val_if_fail (GAM_IS_SLIDER_DUAL (gam_slider_dual), TRUE);

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);


    return locked;
}

static void
gam_slider_dual_set_locked (GamSliderDual *gam_slider_dual, gboolean locked)
{
    GamSliderDualPrivate *priv;
    gchar *key;

    g_return_if_fail (GAM_IS_SLIDER_DUAL (gam_slider_dual));

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

}

GtkWidget *
gam_slider_dual_new (gpointer elem, GamMixer *gam_mixer, GamApp *gam_app)
{
    g_return_val_if_fail (GAM_IS_MIXER (gam_mixer), NULL);

    return g_object_new (GAM_TYPE_SLIDER_DUAL,
                         "elem", elem,
                         "mixer", gam_mixer,
                         "app", gam_app,
                         NULL);
}

void
gam_slider_dual_set_size_groups (GamSliderDual *gam_slider_dual,
                                 GtkSizeGroup *pan_size_group,
                                 GtkSizeGroup *mute_size_group,
                                 GtkSizeGroup *capture_size_group)
{
    GamSliderDualPrivate *priv;

    g_return_if_fail (GAM_IS_SLIDER_DUAL (gam_slider_dual));

    priv = GAM_SLIDER_DUAL_GET_PRIVATE (gam_slider_dual);

    gtk_size_group_add_widget (pan_size_group, priv->lock_button);
    gtk_size_group_add_widget (mute_size_group, gam_slider_get_mute_widget (GAM_SLIDER (gam_slider_dual)));
    gtk_size_group_add_widget (capture_size_group, gam_slider_get_capture_widget (GAM_SLIDER (gam_slider_dual)));
}
