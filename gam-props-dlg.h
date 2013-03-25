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

#ifndef __GAM_PROPS_H__
#define __GAM_PROPS_H__

#include <gtk/gtk.h>

#include "gam-mixer.h"

G_BEGIN_DECLS

#define GAM_TYPE_PROPS_DLG            (gam_props_dlg_get_type ())
#define GAM_PROPS_DLG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAM_TYPE_PROPS_DLG, GamPropsDlg))
#define GAM_PROPS_DLG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAM_TYPE_PROPS_DLG, GamPropsDlgClass))
#define GAM_IS_PROPS_DLG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAM_TYPE_PROPS_DLG))
#define GAM_IS_PROPS_DLG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAM_TYPE_PROPS_DLG))
#define GAM_PROPS_DLG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAM_TYPE_PROPS_DLG, GamPropsDlgClass))

typedef struct _GamPropsDlg GamPropsDlg;
typedef struct _GamPropsDlgClass GamPropsDlgClass;

struct _GamPropsDlg
{
    GtkDialog parent_instance;
};

struct _GamPropsDlgClass
{
    GtkDialogClass parent_class;
};

GType      gam_props_dlg_get_type (void) G_GNUC_CONST;
GtkWidget *gam_props_dlg_new      (GtkWindow *parent,
                                   GamMixer  *gam_mixer);

G_END_DECLS

#endif /* __GAM_PROPS_H__ */
