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
#include <gtk/gtkhscale.h>
#include <gtk/gtkvscale.h>

#include "gam-slider-pan.h"

#define GAM_SLIDER_PAN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GAM_TYPE_SLIDER_PAN, GamSliderPanPrivate))

typedef struct _GamSliderPanPrivate GamSliderPanPrivate;

struct _GamSliderPanPrivate
{
    GtkWidget *pan_slider;
    GtkWidget *vol_slider;
    GtkObject *pan_adjustment;
    GtkObject *vol_adjustment;
};

static void     gam_slider_pan_class_init              (GamSliderPanClass     *klass);
static void     gam_slider_pan_init                    (GamSliderPan          *gam_slider_pan);
static void     gam_slider_pan_finalize                (GObject               *object);
static GObject *gam_slider_pan_constructor             (GType                  type,
                                                        guint                  n_construct_properties,
                                                        GObjectConstructParam *construct_params);
static gint     gam_slider_pan_get_pan                 (GamSliderPan          *gam_slider_pan);
static gint     gam_slider_pan_get_volume              (GamSliderPan          *gam_slider_pan);
static void     gam_slider_pan_update_volume           (GamSliderPan          *gam_slider_pan);
static gint     gam_slider_pan_pan_event_cb            (GtkWidget             *widget,
                                                        GdkEvent              *event,
                                                        GamSliderPan          *gam_slider_pan);
static gint     gam_slider_pan_pan_value_changed_cb    (GtkWidget             *widget,
                                                        GamSliderPan          *gam_slider_pan);
static gint     gam_slider_pan_volume_value_changed_cb (GtkWidget             *widget,
                                                        GamSliderPan          *gam_slider_pan);
static void     gam_slider_pan_refresh                 (GamSlider             *gam_slider);

static gpointer parent_class;

GType
gam_slider_pan_get_type (void)
{
    static GType gam_slider_pan_type = 0;

    if (!gam_slider_pan_type) {
        static const GTypeInfo gam_slider_pan_info =
        {
            sizeof (GamSliderPanClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gam_slider_pan_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (GamSliderPan),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gam_slider_pan_init,
        };

        gam_slider_pan_type = g_type_register_static (GAM_TYPE_SLIDER, "GamSliderPan",
                                                      &gam_slider_pan_info, (GTypeFlags)0);
    }

    return gam_slider_pan_type;
}

static void
gam_slider_pan_class_init (GamSliderPanClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GamSliderClass *object_class = GAM_SLIDER_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = gam_slider_pan_finalize;
    gobject_class->constructor = gam_slider_pan_constructor;

    object_class->refresh = gam_slider_pan_refresh;

    g_type_class_add_private (gobject_class, sizeof (GamSliderPanPrivate));
}

static void
gam_slider_pan_init (GamSliderPan *gam_slider_pan)
{
    GamSliderPanPrivate *priv;

    g_return_if_fail (GAM_IS_SLIDER_PAN (gam_slider_pan));

    priv = GAM_SLIDER_PAN_GET_PRIVATE (gam_slider_pan);

    priv->pan_slider = NULL;
    priv->vol_slider = NULL;
    priv->pan_adjustment = NULL;
    priv->vol_adjustment = NULL;
}

static void
gam_slider_pan_finalize (GObject *object)
{
    GamSliderPan *gam_slider_pan;
    GamSliderPanPrivate *priv;
    
    g_return_if_fail (GAM_IS_SLIDER_PAN (object));

    gam_slider_pan = GAM_SLIDER_PAN (object);

    priv = GAM_SLIDER_PAN_GET_PRIVATE (gam_slider_pan);

    priv->pan_adjustment = NULL;
    priv->vol_adjustment = NULL;
    priv->pan_slider = NULL;
    priv->vol_slider = NULL;

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GObject *
gam_slider_pan_constructor (GType                  type,
                            guint                  n_construct_properties,
                            GObjectConstructParam *construct_params)
{
    GObject *object;
    GamSliderPan *gam_slider_pan;
    GamSliderPanPrivate *priv;

    object = (* G_OBJECT_CLASS (parent_class)->constructor) (type,
                                                             n_construct_properties,
                                                             construct_params);

    gam_slider_pan = GAM_SLIDER_PAN (object);

    priv = GAM_SLIDER_PAN_GET_PRIVATE (gam_slider_pan);

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)))) {
        priv->pan_adjustment = gtk_adjustment_new (gam_slider_pan_get_pan (gam_slider_pan), -100, 101, 1, 5, 1);

        g_signal_connect (G_OBJECT (priv->pan_adjustment), "value-changed",
                          G_CALLBACK (gam_slider_pan_pan_value_changed_cb), gam_slider_pan);

        priv->pan_slider = gtk_hscale_new (GTK_ADJUSTMENT (priv->pan_adjustment));
        gtk_scale_set_draw_value (GTK_SCALE (priv->pan_slider), FALSE);

        g_signal_connect (G_OBJECT (priv->pan_slider), "event",
                          G_CALLBACK (gam_slider_pan_pan_event_cb), gam_slider_pan);
    } else
        priv->pan_slider = gtk_label_new (NULL);

    gtk_widget_show (priv->pan_slider);

    gam_slider_add_pan_widget (GAM_SLIDER (gam_slider_pan), priv->pan_slider);

    priv->vol_adjustment = gtk_adjustment_new (gam_slider_pan_get_volume (gam_slider_pan), 0, 101, 1, 5, 1);

    g_signal_connect (G_OBJECT (priv->vol_adjustment), "value-changed",
                      G_CALLBACK (gam_slider_pan_volume_value_changed_cb), gam_slider_pan);

    priv->vol_slider = gtk_vscale_new (GTK_ADJUSTMENT (priv->vol_adjustment));
    gtk_widget_show (priv->vol_slider);
    gtk_scale_set_draw_value (GTK_SCALE (priv->vol_slider), FALSE);

    gam_slider_add_volume_widget (GAM_SLIDER (gam_slider_pan), priv->vol_slider);

    gtk_label_set_mnemonic_widget (gam_slider_get_label_widget (GAM_SLIDER (gam_slider_pan)),
                                   priv->vol_slider);

    return object;
}

static gint
gam_slider_pan_get_pan (GamSliderPan *gam_slider_pan)
{
    glong left_chn, right_chn;

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)))) {
        if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
            snd_mixer_selem_get_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_LEFT, &left_chn);
        else
            snd_mixer_selem_get_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_LEFT, &left_chn);

        if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
            snd_mixer_selem_get_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_RIGHT, &right_chn);
        else
            snd_mixer_selem_get_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_RIGHT, &right_chn);

        if ((gam_slider_pan_get_volume (gam_slider_pan) != 0) && (left_chn != right_chn))
            return rint (((gfloat)(right_chn - left_chn) / (gfloat)MAX(left_chn, right_chn)) * 100);
    }

    return 0;
}

static gint
gam_slider_pan_get_volume (GamSliderPan *gam_slider_pan)
{
    glong left_chn, right_chn, pmin, pmax;

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
        snd_mixer_selem_get_playback_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), &pmin, &pmax);
    else
        snd_mixer_selem_get_capture_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), &pmin, &pmax);

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
        snd_mixer_selem_get_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_LEFT, &left_chn);
    else
        snd_mixer_selem_get_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_LEFT, &left_chn);

    if (snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)))) {
        return rint (100 - (left_chn * (100 / (gfloat)pmax)));
    } else {
        if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
            snd_mixer_selem_get_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_RIGHT, &right_chn);
        else
            snd_mixer_selem_get_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_RIGHT, &right_chn);

        return rint (100 - (MAX(left_chn, right_chn) * (100 / (gfloat)pmax)));
    }
}

static void
gam_slider_pan_update_volume (GamSliderPan *gam_slider_pan)
{
    GamSliderPanPrivate *priv;

    gint left_chn = 0, right_chn = 0;
    glong pmin, pmax;

    priv = GAM_SLIDER_PAN_GET_PRIVATE (gam_slider_pan);

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
        snd_mixer_selem_get_playback_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), &pmin, &pmax);
    else
        snd_mixer_selem_get_capture_volume_range (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), &pmin, &pmax);

    left_chn = right_chn = rint ((100 - GTK_ADJUSTMENT (priv->vol_adjustment)->value) / (100 / (gfloat)pmax));

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)))) {
        if (GTK_ADJUSTMENT (priv->pan_adjustment)->value < 0) {
            right_chn = rint (left_chn - ((gfloat)ABS(GTK_ADJUSTMENT (priv->pan_adjustment)->value) / 100) * left_chn);
        } else if (GTK_ADJUSTMENT (priv->pan_adjustment)->value > 0) {
            left_chn = rint (right_chn - ((gfloat)GTK_ADJUSTMENT (priv->pan_adjustment)->value / 100) * right_chn);
        }
    }

    if (snd_mixer_selem_has_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)))) {
        snd_mixer_selem_set_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_LEFT, left_chn);
        if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
            snd_mixer_selem_set_playback_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_RIGHT, right_chn);
    } else {
        snd_mixer_selem_set_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_LEFT, left_chn);
        if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan))))
            snd_mixer_selem_set_capture_volume (gam_slider_get_elem (GAM_SLIDER (gam_slider_pan)), SND_MIXER_SCHN_FRONT_RIGHT, right_chn);
    }
}

static gint
gam_slider_pan_pan_event_cb (GtkWidget *widget, GdkEvent *event, GamSliderPan *gam_slider_pan)
{
    GamSliderPanPrivate *priv;

    priv = GAM_SLIDER_PAN_GET_PRIVATE (gam_slider_pan);

    if (event->type == GDK_2BUTTON_PRESS) {
        gtk_adjustment_set_value (GTK_ADJUSTMENT (priv->pan_adjustment), 0.0);
        gtk_adjustment_value_changed (GTK_ADJUSTMENT (priv->pan_adjustment));

        return TRUE;
    }

    return FALSE;
}

static gint
gam_slider_pan_pan_value_changed_cb (GtkWidget *widget, GamSliderPan *gam_slider_pan)
{
    gam_slider_pan_update_volume (gam_slider_pan);

    return TRUE;
}

static gint
gam_slider_pan_volume_value_changed_cb (GtkWidget *widget, GamSliderPan *gam_slider_pan)
{
    gam_slider_pan_update_volume (gam_slider_pan);

    return TRUE;
}

static void
gam_slider_pan_refresh (GamSlider *gam_slider)
{
    GamSliderPan * const gam_slider_pan = GAM_SLIDER_PAN (snd_mixer_elem_get_callback_private (gam_slider_get_elem (gam_slider)));
    GamSliderPanPrivate *priv;

    priv = GAM_SLIDER_PAN_GET_PRIVATE (gam_slider_pan);

    gtk_adjustment_set_value (GTK_ADJUSTMENT (priv->vol_adjustment),
                              (gdouble) gam_slider_pan_get_volume (gam_slider_pan));

    if (!snd_mixer_selem_is_playback_mono (gam_slider_get_elem (gam_slider))) {
        gtk_adjustment_set_value (GTK_ADJUSTMENT (priv->pan_adjustment),
                                  (gdouble) gam_slider_pan_get_pan (gam_slider_pan));
    }
}

GtkWidget *
gam_slider_pan_new (gpointer elem, GamMixer *gam_mixer, GamApp *gam_app)
{
    g_return_val_if_fail (GAM_IS_MIXER (gam_mixer), NULL);

    return g_object_new (GAM_TYPE_SLIDER_PAN,
                         "elem", elem,
                         "mixer", gam_mixer,
                         "app", gam_app,
                         NULL);
}

void
gam_slider_pan_set_size_groups (GamSliderPan *gam_slider_pan,
                                GtkSizeGroup *pan_size_group,
                                GtkSizeGroup *mute_size_group,
                                GtkSizeGroup *capture_size_group)
{
    GamSliderPanPrivate *priv;

    g_return_if_fail (GAM_IS_SLIDER_PAN (gam_slider_pan));

    priv = GAM_SLIDER_PAN_GET_PRIVATE (gam_slider_pan);

    gtk_size_group_add_widget (pan_size_group, priv->pan_slider);
    gtk_size_group_add_widget (mute_size_group, gam_slider_get_mute_widget (GAM_SLIDER (gam_slider_pan)));
    gtk_size_group_add_widget (capture_size_group, gam_slider_get_capture_widget (GAM_SLIDER (gam_slider_pan)));
}
