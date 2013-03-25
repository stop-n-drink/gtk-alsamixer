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

#ifndef __GAM_SLIDER_PAN_H__
#define __GAM_SLIDER_PAN_H__

#include "gam-slider.h"

G_BEGIN_DECLS

#define GAM_TYPE_SLIDER_PAN            (gam_slider_pan_get_type ())
#define GAM_SLIDER_PAN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAM_TYPE_SLIDER_PAN, GamSliderPan))
#define GAM_SLIDER_PAN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAM_TYPE_SLIDER_PAN, GamSliderPanClass))
#define GAM_IS_SLIDER_PAN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAM_TYPE_SLIDER_PAN))
#define GAM_IS_SLIDER_PAN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAM_TYPE_SLIDER_PAN))
#define GAM_SLIDER_PAN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAM_TYPE_SLIDER_PAN, GamSliderPanClass))

typedef struct _GamSliderPan GamSliderPan;
typedef struct _GamSliderPanClass GamSliderPanClass;

struct _GamSliderPan
{
    GamSlider parent_instance;
};

struct _GamSliderPanClass
{
    GamSliderClass parent_class;
};

#include "gam-mixer.h"

GType      gam_slider_pan_get_type        (void) G_GNUC_CONST;
GtkWidget *gam_slider_pan_new             (gpointer      elem,
                                           GamMixer     *gam_mixer,
                                           GamApp       *gam_app);
void       gam_slider_pan_set_size_groups (GamSliderPan *gam_slider_pan,
                                           GtkSizeGroup *pan_size_group,
                                           GtkSizeGroup *mute_size_group,
                                           GtkSizeGroup *capture_size_group);

G_END_DECLS

#endif /* __GAM_SLIDER_PAN_H__ */
