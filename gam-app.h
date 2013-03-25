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

#ifndef __GAM_APP_H__
#define __GAM_APP_H__

#include <gtk/gtkwindow.h>

G_BEGIN_DECLS

#define GAM_CONFIG_DELIMITERS   " &()+/,"

#define GAM_TYPE_APP            (gam_app_get_type ())
#define GAM_APP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAM_TYPE_APP, GamApp))
#define GAM_APP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAM_TYPE_APP, GamAppClass))
#define GAM_IS_APP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAM_TYPE_APP))
#define GAM_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAM_TYPE_APP))
#define GAM_APP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAM_TYPE_APP, GamAppClass))

typedef struct _GamApp GamApp;
typedef struct _GamAppClass GamAppClass;

struct _GamApp
{
    GtkWindow app;
};

struct _GamAppClass
{
    GtkWindowClass parent_class;
};

#include "gam-mixer.h"

GType        gam_app_get_type                (void) G_GNUC_CONST;
GtkWidget   *gam_app_new                     (void);
void         gam_app_run                     (GamApp *gam_app);
gint         gam_app_get_num_cards           (GamApp *gam_app);
GamMixer    *gam_app_get_mixer               (GamApp *gam_app,
                                              gint    index);
gint         gam_app_get_mixer_slider_style  (GamApp *gam_app);
void         gam_app_set_mixer_slider_style  (GamApp *gam_app,
                                              gint    style);
gint         gam_app_get_slider_toggle_style (GamApp *gam_app);
void         gam_app_set_slider_toggle_style (GamApp *gam_app,
                                              gint    style);

G_END_DECLS

#endif /* __GAM_APP_H__ */
