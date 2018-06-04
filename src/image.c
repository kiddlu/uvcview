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

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	<memory.h>
#include	<unistd.h>
#include	<math.h>
#include	<linux/videodev2.h>

#include	"types.h"
#include	"memory.h"
#include	"image.h"
#include	"debug.h"

extern	IMAGE	*
NewImage(
	int		type)
{
	IMAGE	*img;

ENTER_FUNC;
	img = New(IMAGE);
	img->data = NULL;
	img->info = NULL;
	img->size = 0;
	img->frame = 0;
	img->x = 0;
	img->y = 0;
	img->type = type;
LEAVE_FUNC;
	return	(img);
}

extern	void
AllocateImageMemory(
	IMAGE	*img)
{
	if		(  img->data  !=  NULL  ) {
		xfree(img->data);
	}
	switch	(img->type) {
	  case	V4L2_PIX_FMT_RGB24:
	  case	V4L2_PIX_FMT_BGR24:
		img->size = img->w * img->h * 3;
		break;
	  case	V4L2_PIX_FMT_YUYV:
	  case	V4L2_PIX_FMT_UYVY:
		img->size = img->w * img->h * 2;
		break;
	  case	V4L2_PIX_FMT_GREY:
		img->size = img->w * img->h;
		break;
	  default:
		fprintf(stderr,"not support %d\n",img->type);
		exit(1);
		break;
	}
	img->data = xmalloc(img->size);
	memclear(img->data,img->size);
}

extern	void
FreeImage(
	IMAGE	*img)
{
ENTER_FUNC;
	if		(  img->data  !=  NULL  ) {
		xfree(img->data);
	}
	xfree(img);
LEAVE_FUNC;
}

extern	void
YUV2RGB(
	byte	*pix,
	byte	*p,
	int		width,
	int		height)
{
	size_t	bpl;
	int		i
		,	j;
	byte	*q;
	int		y
		,	u
		,	v
		,	r
		,	g
		,	b
		,	cr
		,	cg
		,	cb;

	bpl = width * 2;
	for	( i = 0 ; i < height ; i ++ ) {
		q = p + bpl * i;
		for	( j = 0 ; j < width ; j += 2 ) {
			u = ( (int)q[1] - 128 );
			v = ( (int)q[3] - 128 );
			cr = 718 * v;
			cg = -176 * u - 366 * v;
			cb = 906 * u;

			y = (int)q[0] * 512;
			r = ( y + cr ) >> 9;
			g = ( y + cg ) >> 9;
			b = ( y + cb ) >> 9;
			pix[0] = ( r < 0 )? 0 : ( r > 255 ) ? 255 : r;
			pix[1] = ( g < 0 )? 0 : ( g > 255 ) ? 255 : g;
			pix[2] = ( b < 0 )? 0 : ( b > 255 ) ? 255 : b;

			y = (int)q[2] * 512;
			r = ( y + cr ) >> 9;
			g = ( y + cg ) >> 9;
			b = ( y + cb ) >> 9;
			pix[3] = ( r < 0 )? 0 : ( r > 255 ) ? 255 : r;
			pix[4] = ( g < 0 )? 0 : ( g > 255 ) ? 255 : g;
			pix[5] = ( b < 0 )? 0 : ( b > 255 ) ? 255 : b;

			q += 4;
			pix += 6;
		}
	}
}

extern	void
RGB2YUV(
	byte	*pix,
	byte	*p,
	int		width,
	int		height)
{
	size_t	bpl;
	int		i
		,	j;
	byte	*q;
	int		y1
		,	y2
		,	u
		,	v
		,	r1
		,	g1
		,	b1
		,	r2
		,	g2
		,	b2;

	bpl = width * 3;
	for	( i = 0 ; i < height ; i ++ ) {
		q = p + bpl * i;
		for	( j = 0 ; j < width ; j += 2 ) {
			r1 = (int)q[0];
			g1 = (int)q[1];
			b1 = (int)q[2];

			r2 = (int)q[3];
			g2 = (int)q[4];
			b2 = (int)q[5];

			y1 = ( 153 * r1 + 301 * g1 + 58 * b1 ) >> 9;
			y2 = ( 153 * r2 + 301 * g2 + 58 * b2 ) >> 9;
			u = ( - 87 * ( r1 + r2 ) - 169 * ( g1 + g2 ) + 256 * ( b1 + b2 ) ) >> 9;
			v = (  256 * ( r1 + r2 ) - 215 * ( g1 + g2 ) +  41 * ( b1 + b2 ) ) >> 9;

			pix[0] = ( y1 < 0 ) ? 0 : ( y1 > 255 ) ? 255 : y1;
			pix[1] = ( u  < 0 ) ? 0 : ( u  > 255 ) ? 255 : u;
			pix[2] = ( y2 < 0 ) ? 0 : ( y2 > 255 ) ? 255 : y2;
			pix[3] = ( v  < 0 ) ? 0 : ( v  > 255 ) ? 255 : v;

			q += 6;
			pix += 4;
		}
	}
}

extern	void
YUV2GREY(
	byte	*pix,
	byte	*p,
	int		width,
	int		height)
{
	size_t	bpl;
	int		i
		,	j;
	byte	*q;

	bpl = width * 2;
	for	( i = 0 ; i < height ; i ++ ) {
		q = p + bpl * i;
		for	( j = 0 ; j < width ; j += 2 ) {
			pix[0] = q[0];
			pix[1] = q[2];
			q += 4;
			pix += 2;
		}
	}
}

extern	void
RGB2GREY(
	byte	*pix,
	byte	*p,
	int		width,
	int		height)
{
	size_t	bpl;
	int		i
		,	j;
	byte	*q;
	int		y
		,	r
		,	g
		,	b;

	bpl = width * 3;
	for	( i = 0 ; i < height ; i ++ ) {
		q = p + bpl * i;
		for	( j = 0 ; j < width ; j ++ ) {
			r = (int)q[0];
			g = (int)q[1];
			b = (int)q[2];
			y = ( 153 * r + 301 * g + 58 * b ) >> 9;
			pix[0] = ( y < 0 ) ? 0 : ( y > 255 ) ? 255 : y;
			q += 3;
			pix ++;
		}
	}
}

extern	void
GREY2RGB(
	byte	*pix,
	byte	*p,
	int		width,
	int		height)
{
	size_t	bpl;
	int		i
		,	j;
	byte	*q;

	bpl = width;
	for	( i = 0 ; i < height ; i ++ ) {
		q = p + bpl * i;
		for	( j = 0 ; j < width ; j ++ ) {
			pix[0] = q[0];
			pix[1] = q[0];
			pix[2] = q[0];
			pix += 3;
			q ++;
		}
	}
}

extern	void
GREY2YUV(
	byte	*pix,
	byte	*p,
	int		width,
	int		height)
{
	size_t	bpl;
	int		i
		,	j;
	byte	*q;

	bpl = width;
	for	( i = 0 ; i < height ; i ++ ) {
		q = p + bpl * i;
		for	( j = 0 ; j < width ; j ++ ) {
			pix[0] = q[0];
			pix[1] = 0;
			pix[2] = q[1];
			pix[3] = 0;
			pix += 4;
			q += 2;
		}
	}
}

extern	Bool
CopyImage(
	IMAGE	*dst,
	IMAGE	*src)
{
	Bool	ret;

	if		(	(  dst->w  ==  src->w  )
			&&	(  dst->h  ==  src->h  ) ) {
		ret = TRUE;
		if		(  dst->type  ==  src->type  ) {
			memcpy(dst->data,src->data,dst->size);
		} else
		switch	(dst->type) {
		  case	V4L2_PIX_FMT_RGB24:
			switch	(src->type) {
			  case	V4L2_PIX_FMT_YUYV:
			  case	V4L2_PIX_FMT_UYVY:
				YUV2RGB(dst->data,src->data,src->h,src->w);
				break;
			  case	V4L2_PIX_FMT_GREY:
				GREY2RGB(dst->data,src->data,src->h,src->w);
				break;
			  default:
				ret = FALSE;
				break;
			}
			break;
		  case	V4L2_PIX_FMT_YUYV:
		  case	V4L2_PIX_FMT_UYVY:
			switch	(src->type) {
			  case	V4L2_PIX_FMT_RGB24:
				RGB2YUV(dst->data,src->data,src->h,src->w);
				break;
			  case	V4L2_PIX_FMT_GREY:
				GREY2YUV(dst->data,src->data,src->h,src->w);
				break;
			  default:
				ret = FALSE;
			}
			break;
		  case	V4L2_PIX_FMT_GREY:
			switch	(src->type) {
			  case	V4L2_PIX_FMT_YUYV:
			  case	V4L2_PIX_FMT_UYVY:
				YUV2GREY(dst->data,src->data,src->h,src->w);
				break;
			  default:
				ret = FALSE;
			}
			break;
		  default:
			ret = FALSE;
			break;
		}
	} else {
		ret = FALSE;
	}
	return	(ret);
}

extern	void
FilterYUV(
	IMAGE	*dst,
	IMAGE	*src,
	FILTER	*kernel,
	double	orig,
	double	fact)
{
	int		x
		,	y
		,	i
		,	j;
	double	lap
		,	v;
	byte	*q
		,	*pix;

#define	INDEX_XY(x,y)		(((x)+(y)*src->w)*2)

	q = src->data;
	pix = dst->data;
	for	( y = kernel->h / 2 ; y < src->h - kernel->h / 2 ; y ++ ) {
		for	( x = kernel->w / 2 ; x < src->w - kernel->w / 2 ; x ++ ) {
			lap = 0;
			for	( i = 0 ; i < kernel->h ; i ++ ) {
				for	( j = 0 ; j < kernel->w ; j ++ ) {
					lap += q[INDEX_XY(x+j-(kernel->w/2),y+i-(kernel->h/2))]
						* kernel->factor[i*kernel->w + j];
				}
			}
			v = (q[INDEX_XY(x,y)] * orig) + (lap * fact);
			pix[INDEX_XY(x,y)] = ( v < 0 ) ? 0 : ( v > 255 ) ? 255 : (int)v;
			pix[INDEX_XY(x,y)+1] = q[INDEX_XY(x,y)+1];
		}
	}
#undef	INDEX_XY
}

extern	void
FilterGREY(
	IMAGE	*dst,
	IMAGE	*src,
	FILTER	*kernel,
	double	orig,
	double	fact)
{
	int		x
		,	y
		,	i
		,	j;
	double	lap
		,	v;
	byte	*q
		,	*pix;

#define	INDEX_XY(x,y)		(((x)+(y)*src->w))

	q = src->data;
	pix = dst->data;
	for	( y = kernel->h / 2 ; y < src->h - kernel->h / 2 ; y ++ ) {
		for	( x = kernel->w / 2 ; x < src->w - kernel->w / 2 ; x ++ ) {
			lap = 0;
			for	( i = 0 ; i < kernel->h ; i ++ ) {
				for	( j = 0 ; j < kernel->w ; j ++ ) {
					lap += q[INDEX_XY(x+j-(kernel->w/2),y+i-(kernel->h/2))]
						* kernel->factor[i*kernel->w + j];
				}
			}
			v = (q[INDEX_XY(x,y)] * orig) + (lap * fact);
			pix[INDEX_XY(x,y)] = ( v < 0 ) ? 0 : ( v > 255 ) ? 255 : (int)v;
		}
	}
#undef	INDEX_XY
}

extern	void
MaskYUV(
	IMAGE	*dst,
	IMAGE	*mask,
	IMAGE	*src)
{
	int		x
		,	y;
	byte	*q
		,	*m
		,	*pix;

	for	( y = 0 ; y < src->h ; y ++ ) {
		q = src->data + src->w * y * 2;
		pix = dst->data + dst->w * y * 2;
		m = mask->data + mask->w * y;
		for	( x = 0 ; x < src->w ; x ++ ) {
			pix[0] = ( *m > 0 ) ? q[0] : 0;
			//pix[1] = ( *m > 0 ) ? q[1] : 0;
			pix[1] = q[1];
			pix += 2;
			q += 2;
			m ++;
		}
	}
}

extern	void
MaskRGB(
	IMAGE	*dst,
	IMAGE	*mask,
	IMAGE	*src)
{
	int		x
		,	y;
	byte	*q
		,	*m
		,	*pix;

	for	( y = 0 ; y < src->h ; y ++ ) {
		q = src->data + src->w * y * 3;
		pix = dst->data + dst->w * y * 3;
		m = dst->data + dst->w * y;
		for	( x = 0 ; x < src->w ; x ++ ) {
			pix[0] = ( *m > 0 ) ? q[0] : 0;
			pix[1] = ( *m > 0 ) ? q[1] : 0;
			pix[2] = ( *m > 0 ) ? q[2] : 0;
			pix += 3;
			q += 3;
			m ++;
		}
	}
}

extern	void
ColorFilterRGB(
	IMAGE	*dst,
	IMAGE	*src,
	int		filR,
	int		filG,
	int		filB)
{
	size_t	bpl;
	int		i
		,	j;
	byte	*q
		,	*pix;
	double	r
		,	g
		,	b
		,	fr
		,	fg
		,	fb
		,	fn
		,	y;

ENTER_FUNC;
	bpl = src->w * 3;
	pix = dst->data;
	fr = (double)filR / 255.0;
	fg = (double)filG / 255.0;
	fb = (double)filB / 255.0;
	fn = sqrt ( fr * fr + fg * fg + fb * fb );
	for	( i = 0 ; i < src->h ; i ++ ) {
		q = src->data + bpl * i;
		for	( j = 0 ; j < src->w ; j ++ ) {
			r = (double)q[0] / 255.0;
			g = (double)q[1] / 255.0;
			b = (double)q[2] / 255.0;
			y = 128.0 * ( r * fr + g * fg + b * fb ) /
				( sqrt ( r * r + g * g + b * b ) * fn );
				
			pix[0] = ( y < 0 ) ? 0 : ( y > 255 ) ? 255 : y;
			q += 3;
			pix ++;
		}
	}
LEAVE_FUNC;
}

extern	void
GREY2BI(
	IMAGE	*dst,
	IMAGE	*src,
	int		threshold)
{
	int		i
		,	j;
	byte	*q
		,	*pix;

ENTER_FUNC;
	pix = dst->data;
	for	( i = 0 ; i < src->h ; i ++ ) {
		q = src->data + src->w * i;
		for	( j = 0 ; j < src->w ; j ++ ) {
			pix[0] = ( (int)q[0] > threshold ) ? 255 : 0;
			pix ++;
			q ++;
		}
	}
LEAVE_FUNC;
}

extern	void
RotateRGB(
	IMAGE	*dst,
	IMAGE	*src,
	int		x,
	int		y,
	double	rad)
{
	double	a11
		,	a12
		,	a13
		,	a21
		,	a22
		,	a23;
	double	x0
		,	y0;
	int		xi
		,	yi
		,	u
		,	v;
	byte	*pix
		,	*q;

#define	INDEX_XY(x,y)		(((x)+(y)*src->w)*3)

	q = src->data;
	pix = dst->data;

	a11 = a22 = cos(rad);
	a12 = sin(rad);
	a13 = x * ( 1.0 - a11 ) - y * a12 + 0.5;
	a21 = - sin(rad);
	a23 = - x * a21 + y * ( 1.0 - a22 ) + 0.5;

	for	( v = 0 ; v < src->h ; v ++ ) {
		x0 = (double)(v+1) * a12 + a13;
		y0 = (double)(v+1) * a22 + a23;
		for	( u = 0 ; u < src->w ; u ++ ) {
			xi = (double)(u+1) * a11 + x0;
			yi = (double)(u+1) * a21 + y0;
			if		(	(  xi  >  0        )
					&&	(  xi  <=  src->w  )
					&&	(  yi  >  0        )
					&&	(  yi  <=  src->h  ) ) {
				pix[INDEX_XY(u,v)+0] = q[INDEX_XY(xi-1,yi-1)+0];
				pix[INDEX_XY(u,v)+1] = q[INDEX_XY(xi-1,yi-1)+1];
				pix[INDEX_XY(u,v)+2] = q[INDEX_XY(xi-1,yi-1)+2];
			} else {
				pix[INDEX_XY(u,v)+0] = 0;
				pix[INDEX_XY(u,v)+1] = 0;
				pix[INDEX_XY(u,v)+2] = 0;
			}
		}
	}
#undef	INDEX_XY
}

extern	void
RotateYUV(
	IMAGE	*dst,
	IMAGE	*src,
	int		x,
	int		y,
	double	rad)
{
	double	a11
		,	a12
		,	a13
		,	a21
		,	a22
		,	a23;
	double	x0
		,	y0;
	int		xi
		,	yi
		,	u
		,	v;
	byte	*pix
		,	*q;

#define	INDEX_XY(x,y)		(((x)+(y)*src->w)*2)

	q = src->data;
	pix = dst->data;

	a11 = a22 = cos(rad);
	a12 = sin(rad);
	a13 = x * ( 1.0 - a11 ) - y * a12 + 0.5;
	a21 = - sin(rad);
	a23 = - x * a21 + y * ( 1.0 - a22 ) + 0.5;

	for	( v = 0 ; v < src->h ; v ++ ) {
		x0 = (double)(v+1) * a12 + a13;
		y0 = (double)(v+1) * a22 + a23;
		for	( u = 0 ; u < src->w ; u += 2 ) {
			xi = (double)(u+1) * a11 + x0;
			yi = (double)(u+1) * a21 + y0;
			if		(	(  xi  >  0        )
					&&	(  xi  <=  src->w  )
					&&	(  yi  >  0        )
					&&	(  yi  <=  src->h  ) ) {
				pix[INDEX_XY(u,v)+0] = q[INDEX_XY(xi-1,yi-1)+0];
				pix[INDEX_XY(u,v)+2] = q[INDEX_XY(xi-1,yi-1)+2];
				if		(  ( xi & 1 )  ==  1  ) {
					pix[INDEX_XY(u,v)+1] = q[INDEX_XY(xi-1,yi-1)+1];
					pix[INDEX_XY(u,v)+3] = q[INDEX_XY(xi-1,yi-1)+3];
				} else {
					pix[INDEX_XY(u,v)+1] = q[INDEX_XY(xi-1,yi-1)+3];
					pix[INDEX_XY(u,v)+3] = q[INDEX_XY(xi-1,yi-1)+1];
				}
			} else {
				pix[INDEX_XY(u,v)+0] = 0;
				pix[INDEX_XY(u,v)+1] = 0;
				pix[INDEX_XY(u,v)+2] = 0;
				pix[INDEX_XY(u,v)+3] = 0;
			}
		}
	}
#undef	INDEX_XY
}

static	int
SizePixel(
	IMAGE	*img)
{
	int		ret;

	switch	(img->type) {
	  case	V4L2_PIX_FMT_RGB24:
		ret = 3;
		break;
	  case	V4L2_PIX_FMT_YUYV:
	  case	V4L2_PIX_FMT_UYVY:
		ret = 2;
		break;
	  case	V4L2_PIX_FMT_GREY:
		ret = 1;
		break;
	  default:
		ret = 0;
		break;
	}
	return	(ret);
}

extern	void
ClipGREY(
	IMAGE	*dst,
	IMAGE	*src,
	int		x,
	int		y)
{
	int		i
		,	j;

#define	SRC(x,y)		*(src->data+((x)+(y)*src->w))
#define	DST(x,y)		*(dst->data+((x)+(y)*dst->w))

	for	( i = 0 ; i < dst->h ; i ++ ) {
		for	( j = 0 ; j < dst->w ; j ++ ) {
			DST(j,i) = SRC(j+x,i+y);
		}
	}
#undef	INDEX_XY
}
