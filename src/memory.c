/*
 * Copyright (C) 1989-2007 Ogochan.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"memory.h"
#include	"debug.h"


extern	void	*
_xmalloc(
	size_t	size,
	char	*fn,
	int		line)
{
	void	*ret;

	if		(  ( ret = malloc(size) )  ==  NULL  )	{
		printf("no memory space!! %s(%d)\n",fn,line);
		exit(1);
	}
	return	(ret);
}

extern	void
_xfree(
	void	*p,
	char	*fn,
	int		line)
{
	if		(  p  ==  NULL  )	return;
	free(p);
}

