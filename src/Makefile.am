# a simple UVC viewer
# Copyright (C) 2005  ogochan
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA

## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = uvcview

uvcview_SOURCES = \
	main.c \
	configfile.c	configfile.h	\
	device.c		device.h		\
	image.c			image.h			\
	memory.c		memory.h		\
	timer.c			timer.h			\
	option.c		option.h		\
	monstring.c		monstring.h		\
	input-uvc.c		input-uvc.h		\
	support.c		support.h		\
	interface.c		interface.h		\
	callbacks.c		callbacks.h

uvcview_LDADD	=	\
	@PACKAGE_LIBS@	\
	$(INTLLIBS)		\
	-lpthread -lm
