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

#ifndef __GAM_SLIDER_DUAL_H__
#define __GAM_SLIDER_DUAL_H__

#include "gam-slider.h"

G_BEGIN_DECLS

#define GAM_TYPE_SLIDER_DUAL            (gam_slider_dual_get_type ())
#define GAM_SLIDER_DUAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAM_TYPE_SLIDER_DUAL, GamSliderDual))
#define GAM_SLIDER_DUAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAM_TYPE_SLIDER_DUAL, GamSliderDualClass))
#define GAM_IS_SLIDER_DUAL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAM_TYPE_SLIDER_DUAL))
#define GAM_IS_SLIDER_DUAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAM_TYPE_SLIDER_DUAL))
#define GAM_SLIDER_DUAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAM_TYPE_SLIDER_DUAL, GamSliderDualClass))

typedef struct _GamSliderDual GamSliderDual;
typedef struct _GamSliderDualClass GamSliderDualClass;

struct _GamSliderDual
{
    GamSlider parent_instance;
};

struct _GamSliderDualClass
{
    GamSliderClass parent_class;
};

#include "gam-mixer.h"

GType      gam_slider_dual_get_type        (void) G_GNUC_CONST;
GtkWidget *gam_slider_dual_new             (gpointer      elem,
                                            GamMixer     *gam_mixer,
                                            GamApp       *gam_app);
void       gam_slider_dual_set_size_groups (GamSliderDual *gam_slider_dual,
                                            GtkSizeGroup  *pan_size_group,
                                            GtkSizeGroup  *mute_size_group,
                                            GtkSizeGroup  *capture_size_group);

G_END_DECLS

#endif /* __GAM_SLIDER_DUAL_H__ */
