/*
 * a simple UVC video viewer
 * Copyright (C) 2007 Ogochan.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#ifndef _SYS_TIME_H  
# include <sys/time.h>
#endif

#include <gtk/gtk.h>
#include	<asm/types.h>
#include	<linux/videodev2.h>

#include	"callbacks.h"
#include	"interface.h"
#include	"support.h"
#include	"device.h"
#include	"image.h"
#include	"input-uvc.h"
#include	"debug.h"

#define	ThisFd		IMAGE_FD(ThisImage)

extern	gboolean
on_capture_destroy_event(
	GtkWidget	*widget,
	GdkEvent	*event,
	gpointer	user_data)
{
ENTER_FUNC;
    gtk_main_quit();
LEAVE_FUNC;
	return FALSE;
}

extern	void
on_brightness_value_changed(
	GtkRange	*range,
	gpointer	user_data)
{
	GtkAdjustment	*adj;

ENTER_FUNC;
	adj = gtk_range_get_adjustment(range);
	SetV4L_Control(ThisFd,"BRIGHTNESS", V4L2_CID_BRIGHTNESS, (int)adj->value);
LEAVE_FUNC;
}


extern	void
on_contrast_value_changed(
	GtkRange	*range,
	gpointer	user_data)
{
	GtkAdjustment	*adj;

ENTER_FUNC;
	adj = gtk_range_get_adjustment(range);
	SetV4L_Control(ThisFd,"CONTRAST", V4L2_CID_CONTRAST, (int)adj->value);
LEAVE_FUNC;
}


extern	void
on_hue_value_changed(
	GtkRange	*range,
	gpointer	user_data)
{
	GtkAdjustment	*adj;

ENTER_FUNC;
	adj = gtk_range_get_adjustment(range);
	SetV4L_Control(ThisFd,"HUE", V4L2_CID_HUE, (int)adj->value);
LEAVE_FUNC;
}


extern	void
on_saturation_value_changed(
	GtkRange	*range,
	gpointer	user_data)
{
	GtkAdjustment	*adj;

ENTER_FUNC;
	adj = gtk_range_get_adjustment(range);
	SetV4L_Control(ThisFd,"SATURATION", V4L2_CID_SATURATION, (int)adj->value);
LEAVE_FUNC;
}


extern	void
on_gamma_value_changed(
	GtkRange	*range,
	gpointer	user_data)
{
	GtkAdjustment	*adj;

ENTER_FUNC;
	adj = gtk_range_get_adjustment(range);
	SetV4L_Control(ThisFd,"GAMMA", V4L2_CID_GAMMA, (int)adj->value);
LEAVE_FUNC;
}


extern	void
on_xoff_value_changed(
	GtkRange	*range,
	gpointer	user_data)
{
	GtkAdjustment	*adj;

ENTER_FUNC;
	adj = gtk_range_get_adjustment(range);
	ThisImage->x = (int)adj->value;
LEAVE_FUNC;
}


extern	void
on_yoff_value_changed(
	GtkRange	*range,
	gpointer	user_data)
{
	GtkAdjustment	*adj;

ENTER_FUNC;
	adj = gtk_range_get_adjustment(range);
	ThisImage->y = (int)adj->value;
LEAVE_FUNC;
}
