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

#ifndef __GAM_SLIDER_H__
#define __GAM_SLIDER_H__

#include <asoundlib.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>

#undef ABS
#define ABS(a)    (((a) < 0) ? -(a) : (a))
#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

G_BEGIN_DECLS

#define GAM_TYPE_SLIDER            (gam_slider_get_type ())
#define GAM_SLIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAM_TYPE_SLIDER, GamSlider))
#define GAM_SLIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAM_TYPE_SLIDER, GamSliderClass))
#define GAM_IS_SLIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAM_TYPE_SLIDER))
#define GAM_IS_SLIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAM_TYPE_SLIDER))
#define GAM_SLIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAM_TYPE_SLIDER, GamSliderClass))

typedef struct _GamSlider GamSlider;
typedef struct _GamSliderClass GamSliderClass;

struct _GamSlider
{
    GtkHBox parent_instance;
};

struct _GamSliderClass
{
    GtkHBoxClass parent_class;

    void (* refresh) (GamSlider *gam_slider);
};

#include "gam-mixer.h"
    
GType                 gam_slider_get_type           (void) G_GNUC_CONST;
const gchar *gam_slider_get_name           (GamSlider   *gam_slider);
const gchar *gam_slider_get_config_name    (GamSlider   *gam_slider);
gchar                *gam_slider_get_display_name   (GamSlider   *gam_slider);
void                  gam_slider_set_display_name   (GamSlider   *gam_slider,
                                                     const gchar *name);
gboolean              gam_slider_get_visible        (GamSlider   *gam_slider);
void                  gam_slider_set_visible        (GamSlider   *gam_slider,
                                                     gboolean     visible);
gint                  gam_slider_get_toggle_style   (GamSlider   *gam_slider);
void                  gam_slider_set_toggle_style   (GamSlider   *gam_slider,
                                                     gint         style);
snd_mixer_elem_t     *gam_slider_get_elem           (GamSlider   *gam_slider);
GtkLabel             *gam_slider_get_label_widget   (GamSlider   *gam_slider);
GtkWidget            *gam_slider_get_mute_widget    (GamSlider   *gam_slider);
GtkWidget            *gam_slider_get_capture_widget (GamSlider   *gam_slider);
GamMixer             *gam_slider_get_mixer          (GamSlider   *gam_slider);
void                  gam_slider_add_pan_widget     (GamSlider   *gam_slider,
                                                     GtkWidget   *widget);
void                  gam_slider_add_volume_widget  (GamSlider   *gam_slider,
                                                     GtkWidget   *widget);

G_END_DECLS

#endif /* __GAM_SLIDER_H__ */
