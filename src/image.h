/*
 * a simple UVC video viewer
 * Copyright (C) 2005-2007 Ogochan.
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

#ifndef	__IMAGE_H__
#define	__IMAGE_H__

#include	<sys/time.h>
#include	<linux/videodev2.h>

#include	"types.h"

typedef struct IMAGE_S {
	int		type;			/*	picture type		*/
	int		w
	,		h
	,		x
	,		y;
	int		frame;			/*	frame count			*/
	size_t	size;			/*	image data size		*/
    void	*info;			/*	format specific data */
    byte	*data;			/*	image data 			*/
}	IMAGE;

typedef	struct {
	int		w
	,		h;
	double	factor[];
}	FILTER;

extern	IMAGE	*NewImage(int type);
extern	void	FreeImage(IMAGE *img);
extern	void	AllocateImageMemory(IMAGE *img);
extern	void	YUV2RGB(byte *pix, byte *p, int width, int height);
extern	void	RGB2YUV(byte *pix, byte *p, int width, int height);
extern	void	FilterYUV(IMAGE *dst, IMAGE *src, FILTER *kernel, double orig, double fact);
extern	void	FilterGREY(IMAGE *dst, IMAGE *src, FILTER *kernel, double orig, double fact);
extern	void	MaskYUV(IMAGE *dst, IMAGE *mask, IMAGE *src);
extern	void	MaskRGB(IMAGE *dst, IMAGE *mask, IMAGE *src);
extern	void	ColorFilterRGB(IMAGE *dst, IMAGE *src, int filR, int filG, int filB);
extern	void	GREY2BI(IMAGE *dst, IMAGE *src, int threshold);
extern	void	RotateRGB(IMAGE *dst, IMAGE *src, int x, int y, double rad);
extern	void	RotateYUV(IMAGE *dst, IMAGE *src, int x, int y, double rad);
extern	void	ClipGREY(IMAGE *dst, IMAGE *src, int x, int y);

#endif
