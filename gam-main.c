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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gam-app.h"

int
main (int argc, char *argv[])
{
    GtkWidget *app;
    GError        *error;
//    GnomeProgram *prog;

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif
/*
    prog = gtk_program_init (PACKAGE, VERSION, LIBGTKUI_MODULE,
                               argc, argv,
                               GTK_PARAM_HUMAN_READABLE_NAME, _("GTK ALSA Mixer"),
                               GTK_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                               NULL);

    if (!prog)
        return 1; */

	    if (!gtk_init_with_args (&argc, &argv, 
 		            "alsamixer", 
 		            NULL, 
 		            NULL, 
 		            &error)) 
 		    { 
 		        g_printerr(_("%s\nRun '%s --help' to see a full list of available command line options.\n"), 
 		                   _("Error: could not initialize graphical user interface and option add_price_quotes was not set."), 
 		                   argv[0]); 
 		        return 1; 
 		    } 
 
    app = gam_app_new ();

    if (!app)
        return 1;

    gam_app_run (GAM_APP (app));

    return 0;
}
