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

#define	MAIN
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/mman.h>
#include	<sys/ioctl.h>
#include	<asm/types.h>
#include	<linux/videodev2.h>

#include	"memory.h"
#include	"device.h"
#include	"debug.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

extern	int
xioctl(
	int		fd,
	int		request,
	void	*arg)
{
	int r;

	do	{
		r = ioctl (fd, request, arg);
	}	while (-1 == r && EINTR == errno);

	return r;
}

extern	int
OpenV4L(
	char	*dev)
{
	int		fd;
	struct stat st; 

	if (-1 == stat (dev, &st)) {
		fprintf (stderr, "Cannot identify '%s': %d, %s\n",
				 dev, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}

	if (!S_ISCHR (st.st_mode)) {
		fprintf (stderr, "%s is no device\n", dev);
		exit (EXIT_FAILURE);
	}

	fd = open (dev, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == fd) {
		fprintf (stderr, "Cannot open '%s': %d, %s\n",
				 dev, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}
	return	(fd);
}


extern	void
CloseV4L(
	int		fd)
{
	if (-1 == close (fd))
		errno_exit ("close");
}

extern	Bool
GetV4L_Control(
	int		fd,
	char	*name,
	int		id,
	int		*value)
{
	struct	v4l2_control	ctrl;
	Bool	ret;

	ctrl.id = id;
	if (xioctl(fd,  VIDIOC_G_CTRL, &ctrl) < 0) {
		ret = FALSE;
	} else {
		dbgprintf("now %s = %d", name, ctrl.value);
		*value = ctrl.value;
		ret = TRUE;
	}

	return	(ret);
}

extern	Bool
SetV4L_Control(
	int		fd,
	char	*name,
	int		id,
	int		value)
{
	int now;
	Bool	ret;
	struct	v4l2_control	ctrl;

	if		(  !GetV4L_Control(fd,name, id, &now)  ) {
		ret = FALSE;
	} else {
		ctrl.id = id;
		ctrl.value = value;
		if (ioctl(fd,  VIDIOC_S_CTRL, &ctrl) < 0) {
			ret = FALSE;
		} else {
			ret = TRUE;
			dbgprintf("change %s = %d", name, ctrl.value);
		}
	}

	return	(ret);
}

extern	int
GetV4L_Format(
	int		fd,
	struct v4l2_format	*fmt)
{
	int ret;

ENTER_FUNC;
	fmt->type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_G_FMT, fmt);
LEAVE_FUNC;
	return ret;
}	

extern	int
SetV4L_Format(
	int		fd,
	int		format,
	int		w,
	int		h,
	struct v4l2_format	*fmt)
{
	int ret;
	unsigned int	min;

ENTER_FUNC;
	memset(fmt, 0, sizeof(struct v4l2_format));
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->fmt.pix.sizeimage =   0;
	fmt->fmt.pix.pixelformat = format;
	fmt->fmt.pix.width = w;
	fmt->fmt.pix.height = h;

	ret = ioctl(fd, VIDIOC_S_FMT, fmt);
	/* Buggy driver paranoia. */
	min = fmt->fmt.pix.width * 2;
	if (fmt->fmt.pix.bytesperline < min)
		fmt->fmt.pix.bytesperline = min;
	min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
	if (fmt->fmt.pix.sizeimage < min)
		fmt->fmt.pix.sizeimage = min;
LEAVE_FUNC;
	return ret;
}

extern	Bool
TryV4L_Format(
	int		fd,
	int		format,
	int		w,
	int		h,
	struct v4l2_format	*fmt)
{
	Bool	ret;
	unsigned int	min;

ENTER_FUNC;
	memset(fmt, 0, sizeof(struct v4l2_format));
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->fmt.pix.sizeimage =   0;
	fmt->fmt.pix.pixelformat = format;
	fmt->fmt.pix.width = w;
	fmt->fmt.pix.height = h;
	ret = ioctl(fd, VIDIOC_TRY_FMT, fmt) < 0 ? FALSE : TRUE;
	dbgprintf("%dx%d,ret=%s, capability %dx%d bytesperline=%d sizeimage=%d",
			  w, h, (ret?"TRUE":"FALSE"),
			  fmt->fmt.pix.width, fmt->fmt.pix.height,
			  fmt->fmt.pix.bytesperline, fmt->fmt.pix.sizeimage);
LEAVE_FUNC;
	return ret;
}

extern	V4L_Buffer	*
InitV4L_mmap(
	int		fd)
{
	struct	v4l2_requestbuffers req;
	V4L_Buffer	*buffers;
	int		n;

	CLEAR (req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if		(  xioctl (fd, VIDIOC_REQBUFS, &req)  <  0  )	{
		if (EINVAL == errno) {
			fprintf (stderr, "device does not support "
					 "memory mapping\n");
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf (stderr, "Insufficient buffer memory");
		exit (EXIT_FAILURE);
	}

	if		(  ( buffers = (V4L_Buffer *)xmalloc(sizeof(V4L_Buffer)
												 + req.count * sizeof(struct _V4L_BufferEnt)) )
			   ==  NULL  ) {
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	for	( n = 0; n < req.count; n ++ )	{
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n;

		if		(  xioctl (fd, VIDIOC_QUERYBUF, &buf)  <  0  )
			errno_exit ("VIDIOC_QUERYBUF");

		dbgprintf("buf.length = %d",buf.length);
		buffers->buffer[n].length = buf.length;
		buffers->buffer[n].start =
			mmap (NULL /* start anywhere */,
				  buf.length,
				  PROT_READ | PROT_WRITE /* required */,
				  MAP_SHARED /* recommended */,
				  fd, buf.m.offset);

		dbgprintf("used = %d",buf.bytesused);
		if (MAP_FAILED == buffers->buffer[n].start)
			errno_exit ("mmap");
	}
	buffers->n = n;
	return	(buffers);
}

extern	void
FinalizeV4L_mmap(
	V4L_Buffer	*buffers)
{
	unsigned int i;

	for (i = 0; i < buffers->n; ++i)
		if (-1 == munmap (buffers->buffer[i].start, buffers->buffer[i].length))
			errno_exit ("munmap");

	free (buffers);
}

extern	void
StopV4L_Caputuring(
	int		fd)
{
    enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
		errno_exit ("VIDIOC_STREAMOFF");
}

extern	void
StartV4L_Caputuring(
	int			fd,
	V4L_Buffer	*buffers)
{
	unsigned int i;
	enum v4l2_buf_type type;
	struct	v4l2_streamparm	parm;

ENTER_FUNC;
	for	( i = 0; i < buffers->n; i ++ )	{
		struct v4l2_buffer buf;
		CLEAR (buf);
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;
		if		(  xioctl (fd, VIDIOC_QBUF, &buf)  <  0  )
			errno_exit ("VIDIOC_QBUF");
	}
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if		(  xioctl(fd, VIDIOC_G_PARM , &parm)  <  0  )
		errno_exit ("VIDIOC_G_PARM");
	printf("parm.parm.capture.timeperframe.numerator   = %d\n",parm.parm.capture.timeperframe.numerator);
	printf("parm.parm.capture.timeperframe.denominator = %d\n",parm.parm.capture.timeperframe.denominator);
#if	0
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 5;
	if		(  xioctl(fd, VIDIOC_S_PARM , &parm)  <  0  )
		errno_exit ("VIDIOC_S_PARM");
#endif
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if		(  xioctl(fd, VIDIOC_G_PARM , &parm)  <  0  )
		errno_exit ("VIDIOC_G_PARM");
	printf("parm.parm.capture.timeperframe.numerator   = %d\n",parm.parm.capture.timeperframe.numerator);
	printf("parm.parm.capture.timeperframe.denominator = %d\n",parm.parm.capture.timeperframe.denominator);

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if		(  xioctl (fd, VIDIOC_STREAMON, &type)  <  0  )
		errno_exit ("VIDIOC_STREAMON");
LEAVE_FUNC;
}

