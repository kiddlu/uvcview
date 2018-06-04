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
#include	<unistd.h>

#include	"types.h"
#include	"memory.h"
#include	"monstring.h"
#include	"configfile.h"
#include	"debug.h"

extern	Bool
PutConfigFile(
	char	*file,
	ARG_TABLE	*args)
{
	FILE	*fp;
	Bool	ret;

	if		(  ( fp = fopen(file,"w") )  !=  NULL  ) {
		ret = TRUE;
		for	( ; args->option  !=  NULL ; args ++ ) {
			switch	(args->type) {
			  case	BOOLEAN:
				fprintf(fp,"%s: %s\n",args->option, *(Bool *)args->var ? "TRUE" : "FALSE");
				break;
			  case	INTEGER:
				fprintf(fp,"%s: %d\n",args->option, *(int *)args->var);
				break;
			  case	PROCEDURE:
			  case	STRING:
				if		(  (char *)args->var  !=  NULL  ) {
					fprintf(fp,"%s: %s\n",args->option, *(char *)args->var);
				} else {
					fprintf(fp,"%s:\n",args->option);
				}
				break;
			}
		}
		fclose(fp);
	} else {
		ret = FALSE;
	}
	return	(ret);
}


static	void
GetValue(
	FILE		*fp,
	ARG_TABLE	*arg)
{
	char	buff[SIZE_LONGNAME+1];
	char	*p
		,	*q;
#define	SKIP_SPACE		while	((  *p  !=  0    )&&(  isspace(*p)  ) )	p ++

	rewind(fp);
	while	( fgets(buff,SIZE_LONGNAME,fp)  !=  NULL  ) {
		p = buff;
		SKIP_SPACE;
		if		(  *p  ==  '#'  )	continue;
		SKIP_SPACE;
		if		(  strlicmp(p,arg->option)  ==  0  ) {
			p += strlen(arg->option);
			SKIP_SPACE;
			if		(  *p  !=  ':'  )	continue;
			p ++;
			SKIP_SPACE;
			if		(  ( q = strrchr(p,'\r') )  !=  NULL  )	*q = 0;
			if		(  ( q = strrchr(p,'\n') )  !=  NULL  )	*q = 0;
			switch	(arg->type) {
			  case	BOOLEAN:
				if		(  toupper(*p)  ==  'T'  ) {
					*(Bool *)arg->var = TRUE;
				} else {
					*(Bool *)arg->var = FALSE;
				}
				break;
			  case	INTEGER:
				*(int *)arg->var = atoi(p);
				break;
			  case	PROCEDURE :
				(*(void (*)(char *))(arg->var))(p);
				break;
			  case	STRING:
				if		(  strlen(p)  !=  0  )	{
					*(char **)arg->var = StrDup(p);
				} else {
					*(char **)arg->var = NULL;
				}
				break;
			  default:
				break;
			}
			break;
		}
	}
}

extern	Bool
GetConfigFile(
	char	*file,
	ARG_TABLE	*args)
{
	FILE	*fp;
	Bool	ret;

	if		( ( fp = fopen(file,"r") )  !=  NULL  ) {
		for	( ; args->option  !=  NULL ; args ++ ) {
			GetValue(fp,args);
		}
		ret = TRUE;
	} else {
		ret = FALSE;
	}
	return	(ret);
}
