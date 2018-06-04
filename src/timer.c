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
#include	<unistd.h>
#include	<sys/time.h>
#include	<time.h>

#include	"types.h"
#include	"timer.h"
#include	"memory.h"
#include	"debug.h"

extern	TimeFiller	*
InitTimeFiller(void)
{
	struct	timeval	tv;
	TimeFiller	*ret;

	ret = New(TimeFiller);
	gettimeofday(&tv,NULL);
	ret->next = tv.tv_sec * (int64_t)1000000 + tv.tv_usec;
	return	(ret);
}

extern	void
SleepTimeFiller(
	TimeFiller	*micro,
	int			ms)
{
	struct	timeval	tv;
	int64_t	now;

	gettimeofday(&tv,NULL);
	now = tv.tv_sec * (int64_t)1000000 + tv.tv_usec;
	if		(  micro->next  >  now  ) {
		usleep((int)(micro->next - now));
	}
	micro->next += ms * 1000;
}

extern	void
FreeTimeFiller(
	TimeFiller	*micro)
{
	xfree(micro);
}

