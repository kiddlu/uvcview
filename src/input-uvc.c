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
#include	<stdio.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	<memory.h>
#include	<unistd.h>
#include	<setjmp.h>
#include	<pthread.h>

#include	<errno.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/mman.h>
#include	<sys/ioctl.h>
#include	<asm/types.h>
#include	<linux/videodev2.h>

#include	"types.h"
#include	"memory.h"
#include	"image.h"
#include	"device.h"
#include	"input-uvc.h"
#include	"debug.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

static struct {
	char * colourFormat;
	__u32 code;
}	colourFormatTab[] = {
    { "Grey", V4L2_PIX_FMT_GREY },   //Entries in this table correspond
    { "RGB32", V4L2_PIX_FMT_RGB32 }, //(line by line) to those in the 
    { "BGR32", V4L2_PIX_FMT_BGR32 }, //PVideoDevice ColourFormat table.
    { "RGB24", V4L2_PIX_FMT_RGB24 }, 
    { "BGR24", V4L2_PIX_FMT_BGR24 },
    { "RGB565", V4L2_PIX_FMT_RGB565 },
    { "RGB555", V4L2_PIX_FMT_RGB555 },
    { "YUV411", V4L2_PIX_FMT_Y41P },
    { "YUV411P", V4L2_PIX_FMT_YUV411P },
    { "YUV420", V4L2_PIX_FMT_NV21 },
    { "YUV420P", V4L2_PIX_FMT_YUV420 },
    { "YUV422", V4L2_PIX_FMT_YUYV },   /* Note: YUV422 is for compatibility */
    { "YUV422P", V4L2_PIX_FMT_YUV422P },
    { "YUY2", V4L2_PIX_FMT_YUYV },
    { "JPEG", V4L2_PIX_FMT_JPEG },
	//    { "H263", V4L2_PIX_FMT_H263 },
    { "SBGGR8", V4L2_PIX_FMT_SBGGR8 },
    { "MJPEG", V4L2_PIX_FMT_MJPEG},
    { "UYVY422", V4L2_PIX_FMT_UYVY},
    { NULL, 0}
};

static	V4L_Buffer	*
InitializeCamera(
	int		fd,
	int		width,
	int		height)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	V4L_Buffer	*buffers;
	int		i;

ENTER_FUNC;
	if		(  xioctl (fd, VIDIOC_QUERYCAP, &cap)  <  0  ) {
		if (EINVAL == errno) {
			fprintf (stderr, "device is no V4L2 device\n");
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf (stderr, "device is no video capture device\n");
		exit (EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf (stderr, "device does not support streaming i/o\n");
		exit (EXIT_FAILURE);
	}
	for	( i = 0 ; colourFormatTab[i].colourFormat != NULL ; i ++ ) {
		CLEAR(fmt);
		if		(  TryV4L_Format(fd,colourFormatTab[i].code, width, height, &fmt)  ) {
			printf("%s\n",colourFormatTab[i].colourFormat);
		}
	}

	/* Select video input, video standard and tune here. */

#if	1
	CLEAR (cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if		(  xioctl(fd, VIDIOC_CROPCAP, &cropcap)  ==  0  )	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */
		if		(  xioctl(fd, VIDIOC_S_CROP, &crop)  <  0  )	{
			dbgmsg("error set crop");
			switch (errno) {
			  case EINVAL:
				/* Cropping not supported. */
				break;
			  default:
				/* Errors ignored. */
				break;
			}
		}
	} else {
		dbgmsg("error set format");
	}
#endif
	SetV4L_Format(fd,V4L2_PIX_FMT_YUYV, width, height,&fmt);

	GetV4L_Format(fd,&fmt);
	dbgprintf("%d x %d",fmt.fmt.pix.width, fmt.fmt.pix.height);

	buffers = InitV4L_mmap(fd);
	buffers->width = fmt.fmt.pix.width;
	buffers->height = fmt.fmt.pix.height;

LEAVE_FUNC;
	return	(buffers);
}

extern	IMAGE	*
OpenCamera(
	char	*dev,
	int		width,
	int		height,
	int		type)
{
	V4L_Buffer	*buffers;
	V4L_Info	*info;
	IMAGE	*img;
	int		fd;

ENTER_FUNC;
	if		(  ( fd = OpenV4L(dev) )  <  0  ) {
		exit(1);
	}
	if		(  ( buffers = InitializeCamera(fd,width,height) )  ==  NULL  ) {
		exit(1);
	}
	if		(  ( img = NewImage(type) )  !=  NULL  ) {
		img->w = width;
		img->h = height;
		AllocateImageMemory(img);
		info = New(V4L_Info);
		info->fd = fd;
		info->buffers = buffers;
		info->config = NULL;
		img->info = info;
		StartV4L_Caputuring(fd,buffers);
	}
LEAVE_FUNC;
	return	(img);
}

static	void
ClipImageData(
	IMAGE	*img,
	byte	*data,
	int		width,
	int		height,
	int		bpp)
{
	int		i;

	if		(	(  img->x  ==  0  )
			&&	(  img->y  ==  0  )
			&&	(  img->w  ==  width   )
			&&	(  img->h  ==  height  ) ) {
		memcpy(img->data,data,img->size);
	} else {
		for	( i = 0 ; i < img->h ; i ++ ) {
			memcpy(img->data + img->w * bpp * i,
				   data + ( width * ( i + img->y ) + ( img->x & ~1 ) ) * bpp,
				   img->w * bpp);
		}
	}
}

static void
process_image(
	IMAGE	*img,
    void	*p,
	size_t	size)
{
    static	byte	rgb_buf[SIZE_RGB_BUFF];
	V4L_Info	*info = img->info;
    int		i
		,	j;
	byte	*q
		,	*pix;

ENTER_FUNC;
	switch	(img->type) {
	  case	V4L2_PIX_FMT_RGB24:
	  case	V4L2_PIX_FMT_BGR24:
		YUV2RGB(rgb_buf,p,info->buffers->width,info->buffers->height);
		ClipImageData(img,rgb_buf,info->buffers->width,info->buffers->height,3);
		break;
	  case	V4L2_PIX_FMT_YUYV:
	  case	V4L2_PIX_FMT_UYVY:
		ClipImageData(img,p,info->buffers->width,info->buffers->height,2);
		break;
	}
LEAVE_FUNC;
}

static	void
UpdateMovie(
	IMAGE		*img)
{
	fd_set	fds;
	struct	timeval tv;
	int		r;
    struct	v4l2_buffer buf;
    unsigned int i;
	V4L_Info	*info = img->info;

ENTER_FUNC;
	dbgprintf("fd = %d",info->fd);
	while	(TRUE) {
		FD_ZERO(&fds);
		FD_SET(info->fd, &fds);
		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		if		( select (info->fd + 1, &fds, NULL, NULL, &tv)  <  0  ) {
			errno_exit ("select");
		}
		img->frame ++;

		CLEAR (buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		while	(  xioctl(info->fd, VIDIOC_DQBUF, &buf)  <  0  )	{
			switch (errno) {
			  case EAGAIN:
				dbgmsg("EAGAIN");
				break;
			  case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			  default:
				errno_exit ("VIDIOC_DQBUF");
			}
			usleep(1000);
		}
		process_image(img,info->buffers->buffer[buf.index].start,buf.bytesused);
		if		(  xioctl (info->fd, VIDIOC_QBUF, &buf)  <  0  )
			errno_exit ("VIDIOC_QBUF");
	}
LEAVE_FUNC;    
}

static	pthread_t	thr;

extern	void
StartCamera(
	IMAGE		*img)
{
ENTER_FUNC;
	img->frame = 0;
	pthread_create(&thr,NULL,(void *(*)(void *))UpdateMovie,img);
LEAVE_FUNC;
}

extern	void
StopCamera(
	IMAGE	*img)
{
	V4L_Info	*info = img->info;

	pthread_cancel(thr);
	StopV4L_Caputuring(info->fd);
	FinalizeV4L_mmap(info->buffers);
	CloseV4L(info->fd);
}

extern	void
SetCameraConfig(
	IMAGE		*img,
	CameraConfig	*conf)
{
	SetV4L_Control(IMAGE_FD(img),"BRIGHTNESS", V4L2_CID_BRIGHTNESS, conf->brightness);
	SetV4L_Control(IMAGE_FD(img),"CONTRAST", V4L2_CID_CONTRAST, conf->contrast);
	SetV4L_Control(IMAGE_FD(img),"HUE", V4L2_CID_HUE, conf->hue);
	SetV4L_Control(IMAGE_FD(img),"GAMMA", V4L2_CID_GAMMA, conf->gamma);
	SetV4L_Control(IMAGE_FD(img),"SATURATION", V4L2_CID_SATURATION, conf->saturation);
	SetV4L_Control(IMAGE_FD(img),"POWER LINE FREQUENCY", V4L2_CID_POWER_LINE_FREQUENCY, conf->line_frequency);
	img->x = conf->xoff;
	img->y = conf->yoff;
}

extern	void
GetCameraConfig(
	IMAGE		*img,
	CameraConfig	*conf)
{
	GetV4L_Control(IMAGE_FD(img),"BRIGHTNESS", V4L2_CID_BRIGHTNESS, &conf->brightness);
	GetV4L_Control(IMAGE_FD(img),"CONTRAST", V4L2_CID_CONTRAST, &conf->contrast);
	GetV4L_Control(IMAGE_FD(img),"HUE", V4L2_CID_HUE, &conf->hue);
	GetV4L_Control(IMAGE_FD(img),"GAMMA", V4L2_CID_GAMMA, &conf->gamma);
	GetV4L_Control(IMAGE_FD(img),"SATURATION", V4L2_CID_SATURATION, &conf->saturation);
	GetV4L_Control(IMAGE_FD(img),"POWER LINE FREQUENCY", V4L2_CID_POWER_LINE_FREQUENCY, &conf->line_frequency);
	conf->xoff = img->x;
	conf->yoff = img->y;
}
