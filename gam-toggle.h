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

#ifndef __GAM_TOGGLE_H__
#define __GAM_TOGGLE_H__

#include <asoundlib.h>
#include <gtk/gtkcheckbutton.h>

G_BEGIN_DECLS

#define GAM_TYPE_TOGGLE            (gam_toggle_get_type ())
#define GAM_TOGGLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAM_TYPE_TOGGLE, GamToggle))
#define gam_toggle_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAM_TYPE_TOGGLE, GamToggleClass))
#define GAM_IS_TOGGLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAM_TYPE_TOGGLE))
#define GAM_IS_TOGGLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAM_TYPE_TOGGLE))
#define gam_toggle_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAM_TYPE_TOGGLE, GamToggleClass))

typedef struct _GamToggle GamToggle;
typedef struct _GamToggleClass GamToggleClass;

struct _GamToggle
{
    GtkCheckButton parent_instance;
};

struct _GamToggleClass
{
    GtkCheckButtonClass parent_class;
};

#include "gam-mixer.h"

GType                 gam_toggle_get_type         (void) G_GNUC_CONST;
GtkWidget            *gam_toggle_new              (gpointer          elem,
                                                   GamMixer         *gam_mixer,
                                                   GamApp           *gam_app);
gboolean              gam_toggle_get_state        (GamToggle        *gam_toggle);
void                  gam_toggle_set_state        (GamToggle        *gam_toggle,
                                                   gboolean          state);
const gchar *gam_toggle_get_name         (GamToggle        *gam_toggle);
const gchar *gam_toggle_get_config_name  (GamToggle        *gam_toggle);
gchar                *gam_toggle_get_display_name (GamToggle        *gam_toggle);
void                  gam_toggle_set_display_name (GamToggle        *gam_toggle,
                                                   const gchar      *name);
gboolean              gam_toggle_get_visible      (GamToggle        *gam_toggle);
void                  gam_toggle_set_visible      (GamToggle        *gam_toggle,
                                                   gboolean          visible);

G_END_DECLS

#endif /* __GAM_TOGGLE_H__ */
