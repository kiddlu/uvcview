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

#ifndef	__INPUT_UVC_H__
#define	__INPUT_UVC_H__
#include	"types.h"
#include	"image.h"
#include	"device.h"

#define	SIZE_RGB_BUFF	2048*2048*3

typedef	struct {
	int		brightness;
	int		contrast;
	int		hue;
	int		gamma;
	int		saturation;
	int		line_frequency;
	int		xoff;
	int		yoff;
}	CameraConfig;

typedef	struct {
	int			fd;
	CameraConfig	*config;
	V4L_Buffer	*buffers;
}	V4L_Info;

#define	IMAGE_INFO(img)	((V4L_Info *)(img)->info)
#define	IMAGE_FD(img)	(IMAGE_INFO(img)->fd)

extern	IMAGE	*OpenCamera(char *dev, int width, int height, int type);
extern	void	StartCamera(IMAGE *img);
extern	void	StopCamera(IMAGE *img);
extern	void	SetCameraConfig(IMAGE *img, CameraConfig *conf);
extern	void	GetCameraConfig(IMAGE *img, CameraConfig *conf);

#endif
