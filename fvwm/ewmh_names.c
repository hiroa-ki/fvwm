/*
 * Copyright (C) 2001  Olivier Chapuis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 */

#include "config.h"

#ifdef HAVE_EWMH
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>

#include "libs/fvwmlib.h"
#include "libs/Flocale.h"
#include "libs/FlocaleCharset.h"
#include "libs/Ficonv.h"
#include "fvwm.h"
#include "window_flags.h"
#include "cursor.h"
#include "functions.h"
#include "misc.h"
#include "screen.h"
#include "module_interface.h"
#include "borders.h"
#include "add_window.h"
#include "icons.h"
#include "ewmh.h"
#include "ewmh_intern.h"
#include "externs.h"

/***********************************************************************
 * set the visibale window name and icon name
 ***********************************************************************/
void EWMH_SetVisibleName(FvwmWindow *fwin, Bool is_icon_name)
{
	unsigned char *val;
	char *tmp_str;
	FlocaleCharset *fc = NULL;

	if (!FiconvSupport)
		return;

	/* set the ewmh visible name only if it is != wm name */
	if (is_icon_name)
	{
		if ((fwin->icon_name_count == 0 ||
		     !USE_INDEXED_ICON_NAME(fwin)) &&
		    !HAS_EWMH_WM_ICON_NAME(fwin) &&
		    !HAS_EWMH_WM_NAME(fwin))
		{
			ewmh_DeleteProperty(FW_W(fwin),
					    "_NET_WM_ICON_VISIBLE_NAME",
					    EWMH_ATOM_LIST_FVWM_WIN);
			return;
		}
		if (fwin->icon_font != NULL)
		{
			fc = fwin->icon_font->str_fc;
		}
		tmp_str = fwin->visible_icon_name;
	}
	else
	{
		if ((fwin->name_count == 0 || !USE_INDEXED_WINDOW_NAME(fwin)) &&
		    !HAS_EWMH_WM_NAME(fwin) && !HAS_EWMH_WM_ICON_NAME(fwin))
		{
			ewmh_DeleteProperty(FW_W(fwin),
					    "_NET_WM_VISIBLE_NAME",
					    EWMH_ATOM_LIST_FVWM_WIN);
			return;
		}
		if (fwin->title_font != NULL)
		{
			fc = fwin->title_font->str_fc;
		}
		tmp_str = fwin->visible_name;
	}

	if (tmp_str == NULL)
		return; /* should never happen */

	val = (unsigned char *)FiconvCharsetToUtf8(
					dpy, fc, tmp_str, strlen(tmp_str));

	if (val == NULL)
		return;

	if (is_icon_name)
	{
		ewmh_ChangeProperty(FW_W(fwin), "_NET_WM_ICON_VISIBLE_NAME",
				    EWMH_ATOM_LIST_FVWM_WIN,
				    (unsigned char *)val, strlen(val));
	}
	else
	{
		ewmh_ChangeProperty(FW_W(fwin), "_NET_WM_VISIBLE_NAME",
				    EWMH_ATOM_LIST_FVWM_WIN,
				    (unsigned char *)val, strlen(val));
	}
	free(val);
}

/***********************************************************************
 * setup and property notify
 ***********************************************************************/
int EWMH_WMIconName(EWMH_CMD_ARGS)
{
	unsigned int size = 0;
	CARD32 *val;
	char *tmp_str;
	FlocaleCharset *fc = NULL;

	if (!FiconvSupport)
		return 0;

	val = ewmh_AtomGetByName(FW_W(fwin), "_NET_WM_ICON_NAME",
				 EWMH_ATOM_LIST_PROPERTY_NOTIFY, &size);

	if (val == NULL)
	{
		SET_HAS_EWMH_WM_ICON_NAME(fwin,0);
		return 0;
	}
	if (fwin->icon_font != NULL)
	{
		fc = fwin->icon_font->str_fc;
	}

	tmp_str = (char *)FiconvUtf8ToCharset(
					dpy, fc, (const char *) val, size);
	free(val);
	if (tmp_str == NULL)
	{
		SET_HAS_EWMH_WM_ICON_NAME(fwin,0);
		return 0;
	}

	if (ev != NULL)
	{
		/* client message */
		free_window_names(fwin, False, True);
	}

	fwin->icon_name.name = tmp_str;

	SET_HAS_EWMH_WM_ICON_NAME(fwin, 1);
	SET_WAS_ICON_NAME_PROVIDED(fwin, 1);

	if (ev == NULL)
	{
		/* return now for setup */
		return 1;
	}

	if (fwin->icon_name.name &&
	    strlen(fwin->icon_name.name) > MAX_ICON_NAME_LEN)
	{
		(fwin->icon_name.name)[MAX_ICON_NAME_LEN] = 0;
	}

	setup_visible_name(fwin, True);
	EWMH_SetVisibleName(fwin, True);
	BroadcastWindowIconNames(fwin, False, True);
	RedoIconName(fwin);
	return 1;
}

int EWMH_WMName(EWMH_CMD_ARGS)
{
	unsigned int size = 0;
	CARD32 *val;
	char *tmp_str;
	FlocaleCharset *fc = NULL;

	if (!FiconvSupport)
		return 0;

	val = ewmh_AtomGetByName(FW_W(fwin), "_NET_WM_NAME",
				 EWMH_ATOM_LIST_PROPERTY_NOTIFY, &size);

	if (val == NULL)
	{
		SET_HAS_EWMH_WM_NAME(fwin,0);
		return 0;
	}
	if (fwin->title_font != NULL)
	{
		fc = fwin->title_font->str_fc;
	}

	tmp_str = (char *)FiconvUtf8ToCharset(
					dpy, fc, (const char *) val, size);
	free(val);
	if (tmp_str == NULL)
	{
		SET_HAS_EWMH_WM_NAME(fwin,0);
		return 0;
	}

	if (ev != NULL)
		free_window_names(fwin, True, False);

	fwin->name.name = tmp_str;
	SET_HAS_EWMH_WM_NAME(fwin, 1);

	if (ev == NULL)
	{
		return 1;
	}

	if (fwin->name.name && strlen(fwin->name.name) > MAX_WINDOW_NAME_LEN)
	{
		(fwin->name.name[MAX_WINDOW_NAME_LEN]) = 0;
	}

	setup_visible_name(fwin, False);
	SET_NAME_CHANGED(fwin, 1);
	EWMH_SetVisibleName(fwin, False);
	BroadcastWindowIconNames(fwin, True, False);

	/* fix the name in the title bar */
	if(!IS_ICONIFIED(fwin))
	{
		border_draw_decorations(
		  fwin, PART_TITLE, (Scr.Hilite == fwin),
		  True, CLEAR_ALL, NULL, NULL);
	}

	if (!WAS_ICON_NAME_PROVIDED(fwin))
	{
		fwin->icon_name = fwin->name;
		setup_visible_name(fwin, True);
		BroadcastWindowIconNames(fwin, False, True);
		EWMH_SetVisibleName(fwin, True);
		RedoIconName(fwin);
	}
	return 0;
}

#define MAX(A,B) ((A)>(B)? (A):(B))
/***********************************************************************
 * set the desktop name
 ***********************************************************************/
void EWMH_SetDesktopNames(void)
{
	int nbr = 0;
	int len = 0;
	int i;
	int j = 0;
	DesktopsInfo *d,*s;
	unsigned char **names;
	unsigned char *val;

	if (!FiconvSupport)
		return;

	d = Scr.Desktops->next;
	/* skip negative desk */
	while (d != NULL && d->desk < 0)
		d = d->next;
	s = d;
	while (d != NULL && d->name != NULL && d->desk == nbr)
	{
		nbr++;
		len += strlen(d->name) + 1;
		d = d->next;
	}
	if (nbr == 0)
		return;

	val = (unsigned char *)safemalloc(len);
	names = (void *)safemalloc(sizeof(*names)*nbr);
	for (i = 0; i < nbr; i++)
	{
		names[i] =
			(unsigned char *)FiconvCharsetToUtf8(dpy, NULL,
							     s->name,
							     strlen(s->name));
		s = s->next;
	}
	for (i = 0; i < nbr; i++)
	{
		if (names[i] != NULL)
		{
			memcpy(&val[j], names[i], strlen(names[i])+1);
			j += strlen(names[i]) + 1;
			free(names[i]);
		}
	}
	free(names);
	ewmh_ChangeProperty(Scr.Root, "_NET_DESKTOP_NAMES",
			    EWMH_ATOM_LIST_CLIENT_ROOT,
			    (unsigned char *)val, len);
	free(val);
}
#endif /* HAVE_EWMH */