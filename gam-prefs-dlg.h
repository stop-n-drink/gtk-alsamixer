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

#ifndef __GAM_PREFS_H__
#define __GAM_PREFS_H__

#include <gtk/gtk.h>

#include "gam-app.h"

G_BEGIN_DECLS

#define GAM_TYPE_PREFS_DLG            (gam_prefs_dlg_get_type ())
#define GAM_PREFS_DLG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAM_TYPE_PREFS_DLG, GamPrefsDlg))
#define GAM_PREFS_DLG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAM_TYPE_PREFS_DLG, GamPrefsDlgClass))
#define GAM_IS_PREFS_DLG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAM_TYPE_PREFS_DLG))
#define GAM_IS_PREFS_DLG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAM_TYPE_PREFS_DLG))
#define GAM_PREFS_DLG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAM_TYPE_PREFS_DLG, GamPrefsDlgClass))

typedef struct _GamPrefsDlg GamPrefsDlg;
typedef struct _GamPrefsDlgClass GamPrefsDlgClass;

struct _GamPrefsDlg
{
    GtkDialog parent_instance;
};

struct _GamPrefsDlgClass
{
    GtkDialogClass parent_class;
};

GType      gam_prefs_dlg_get_type (void) G_GNUC_CONST;
GtkWidget *gam_prefs_dlg_new      (GtkWindow *parent);

G_END_DECLS

#endif /* __GAM_PREFS_H__ */
