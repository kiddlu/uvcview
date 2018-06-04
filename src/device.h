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

#ifndef	__INC_DEVICE_H_
#define	__INC_DEVICE_H_
#include	"types.h"
#include	<linux/videodev2.h>

struct	_V4L_BufferEnt {
	void	*start;
	size_t	length;
};

#ifndef	TimeFiller
typedef	struct {
	int64_t	next;
}	_TimeFiller;
#define	TimeFiller	_TimeFiller
extern	TimeFiller	*InitTimeFiller(void);
extern	void		SleepTimeFiller(TimeFiller *micro, int ms);
extern	void		FreeTimeFiller(TimeFiller *micro);
#endif

typedef	struct	{
	int		n
	,		width
	,		height;
	struct	_V4L_BufferEnt	buffer[0];
}	V4L_Buffer;

#define V4L2_CID_BACKLIGHT_COMPENSATION		(V4L2_CID_PRIVATE_BASE+0)
#define V4L2_CID_POWER_LINE_FREQUENCY		(V4L2_CID_PRIVATE_BASE+1)
#define V4L2_CID_SHARPNESS			(V4L2_CID_PRIVATE_BASE+2)
#define V4L2_CID_HUE_AUTO			(V4L2_CID_PRIVATE_BASE+3)

#define V4L2_CID_FOCUS_AUTO			(V4L2_CID_PRIVATE_BASE+4)
#define V4L2_CID_FOCUS_ABSOLUTE			(V4L2_CID_PRIVATE_BASE+5)
#define V4L2_CID_FOCUS_RELATIVE			(V4L2_CID_PRIVATE_BASE+6)

#define V4L2_CID_PANTILT_RELATIVE		(V4L2_CID_PRIVATE_BASE+7)
#define V4L2_CID_PANTILT_RESET			(V4L2_CID_PRIVATE_BASE+8)

#define V4L2_CID_EXPOSURE_AUTO			(V4L2_CID_PRIVATE_BASE+9)
#define V4L2_CID_EXPOSURE_ABSOLUTE		(V4L2_CID_PRIVATE_BASE+10)

#define V4L2_CID_WHITE_BALANCE_TEMPERATURE_AUTO	(V4L2_CID_PRIVATE_BASE+11)
#define V4L2_CID_WHITE_BALANCE_TEMPERATURE	(V4L2_CID_PRIVATE_BASE+12)

#define V4L2_CID_PRIVATE_LAST			V4L2_CID_WHITE_BALANCE_TEMPERATURE

extern	int		xioctl(int fd, int request, void *arg);
extern	int		OpenV4L(char *dev);
extern	void	CloseV4L(int fd);
extern	Bool	GetV4L_Control(int fd, char *name, int id, int *value);
extern	Bool	SetV4L_Control(int fd, char *name, int id, int value);
extern	int		GetV4L_Format(int fd, struct v4l2_format *fmt);
extern	int		SetV4L_Format(int fd, int format, int w, int h, struct v4l2_format *fmt);
extern	Bool	TryV4L_Format(int fd, int format, int w, int h, struct v4l2_format *fmt);
extern	V4L_Buffer	*InitV4L_mmap(int fd);
extern	void	FinalizeV4L_mmap(V4L_Buffer *buffers);
extern	void	StopV4L_Caputuring(int fd);
extern	void	StartV4L_Caputuring(int fd, V4L_Buffer *buffers);

#endif

