/*
#define	DEBUG
#define	TRACE
*/

#define	MAIN

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<gtk/gtk.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<errno.h>
#include	<signal.h>

#include	"interface.h"
#include	"support.h"
#include	"callbacks.h"
#include	"input-uvc.h"
#include	"device.h"
#include	"image.h"
#include	"configfile.h"
#include	"option.h"
#include	"debug.h"

#define	DEVICE	"/dev/video0"
#define	CONFIG	"uvccapture.conf"

#if	1
#define WIDTH	640
#define HEIGHT	480
#else
#define WIDTH	320
#define HEIGHT	240
#define WIDTH	2048
#define HEIGHT	2048
#define WIDTH	1024
#define HEIGHT	576
#endif

static	char	*Config;
static	char	*CamDevice;
static	int		FrameRate;
static	int		Width;
static	int		Height;

static	sigset_t SigMask;

#define FRAME_RATE			24

typedef	struct {
	IMAGE		*img;
    GtkWidget	*area;
	TimeFiller	*filler;
}	FrameArgs;

extern void
errno_exit(
	char	*s)
{
    fprintf (stderr, "%s error %d, %s\n",
	     s, errno, strerror (errno));
    exit (EXIT_FAILURE);
}

static	IMAGE	*
InitSystem(void)
{
	IMAGE	*img;

ENTER_FUNC;
	if		(  ( img = OpenCamera(CamDevice,Width,Height,V4L2_PIX_FMT_RGB24) )  !=  NULL  ) {
		StartCamera(img);
		ThisImage = img;
		dbgprintf("img->h    = %d",img->h);
		dbgprintf("img->w    = %d",img->w);
		dbgprintf("img->size = %d",img->size);
	} else {
		img = NULL;
	}
LEAVE_FUNC;
	return	(img);
}

static	void
StopProcess(
	int		ec)
{
	exit(ec);
}

static int
read_frame(
	FrameArgs	*arg)
{
ENTER_FUNC;
	SleepTimeFiller(arg->filler,1000/FrameRate);
    gdk_draw_rgb_image(arg->area->window,
					   arg->area->style->fg_gc[GTK_STATE_NORMAL], 0, 0,
					   arg->img->w, arg->img->h,
					   GDK_RGB_DITHER_NONE, arg->img->data, arg->img->w * 3);
LEAVE_FUNC;    
	return	(1);
}

static	void
SetUpWidgetsProperty(
	GtkWidget	*capture,
	IMAGE		*img,
	CameraConfig	*config)
{
	GtkWidget	*picture
		,		*range;
	GtkAdjustment	*adj;

	dbgprintf("img->w = %d",img->w);
	dbgprintf("img->h = %d",img->h);

	gtk_widget_set_size_request (capture, img->w+20, img->h+190);

	picture = lookup_widget(capture,"picture");
	gtk_widget_set_size_request (picture, img->w, img->h);

	range = lookup_widget(capture,"xoff");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->upper = ((V4L_Info *)img->info)->buffers->width - img->w;
	gtk_adjustment_changed(adj);

	range = lookup_widget(capture,"yoff");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->upper = ((V4L_Info *)img->info)->buffers->height - img->h;
	gtk_adjustment_changed(adj);

	range = lookup_widget(capture,"brightness");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->value = config->brightness;
	gtk_adjustment_changed(adj);

	range = lookup_widget(capture,"contrast");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->value = config->contrast;
	gtk_adjustment_changed(adj);

	range = lookup_widget(capture,"gamma");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->value = config->gamma;
	gtk_adjustment_changed(adj);

	range = lookup_widget(capture,"saturation");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->value = config->saturation;
	gtk_adjustment_changed(adj);

	range = lookup_widget(capture,"xoff");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->value = img->x;
	gtk_adjustment_changed(adj);

	range = lookup_widget(capture,"yoff");
	adj = gtk_range_get_adjustment(GTK_RANGE(range));
	adj->value = img->y;
	gtk_adjustment_changed(adj);
}

static	ARG_TABLE	option[] = {
	{	"cam",		STRING,		TRUE,	(void*)&CamDevice,
		"camera device"								},
	{	"fps",		INTEGER,	TRUE,	(void*)&FrameRate,
		"frame rate(frame/sec)" 					},
	{	"width",	INTEGER,	TRUE,	(void*)&Width,
		"picture width"			 					},
	{	"height",	INTEGER,	TRUE,	(void*)&Height,
		"picture height"		 					},
	{	"config",	STRING,		TRUE,	(void*)&Config,
		"config file name"							},


	{	NULL,		0,			FALSE,		NULL 	}
};

static	ARG_TABLE	para[] = {
	{	"brightness",		INTEGER,	TRUE,	NULL,	NULL },
	{	"contrast",			INTEGER,	TRUE,	NULL,	NULL },
	{	"hue",				INTEGER,	TRUE,	NULL,	NULL },
	{	"gamma",			INTEGER,	TRUE,	NULL,	NULL },
	{	"saturation",		INTEGER,	TRUE,	NULL,	NULL },
	{	"line_frequency",	INTEGER,	TRUE,	NULL,	NULL },
	{	"yoff",				INTEGER,	TRUE,	NULL,	NULL },
	{	"xoff",				INTEGER,	TRUE,	NULL,	NULL },

	{	NULL,		0,			FALSE,		NULL 	}
};

static	void
SetDefault(void)
{
	CamDevice = DEVICE;
	Config = CONFIG;
	FrameRate = FRAME_RATE;
	Width = WIDTH;
	Height = HEIGHT;
}

extern	int
main(
	int		argc,
	char	*argv[])
{
	GtkWidget	*capture;
	FrameArgs	args;
	IMAGE	*img;
	int			status;
    sigset_t sigmask;
	TimeFiller	*filler;
	Bool	fCont;
	CameraConfig	config;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigmask, &SigMask);
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	(void)signal(SIGPIPE,(void *)StopProcess);
	SetDefault();
	(void)GetOption(option,argc,argv);

	config.brightness = 150;
	config.contrast = 40;
	config.hue = 0;
	config.gamma = 30;
	config.saturation = 20;
	config.line_frequency = 1;
	config.xoff = 0;
	config.yoff = 0;

	para[0].var = &config.brightness;
	para[1].var = &config.contrast;
	para[2].var = &config.hue;
	para[3].var = &config.gamma;
	para[4].var = &config.saturation;
	para[5].var = &config.line_frequency;
	para[6].var = &config.xoff;
	para[7].var = &config.yoff;
	
	GetConfigFile(Config,para);

	gtk_set_locale ();
	gtk_init (&argc, &argv);
	gtk_rc_parse("./gtkrc");
	gdk_rgb_init();
	add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");

	capture = create_capture ();
	gtk_widget_show (capture);
	img = InitSystem();
	SetCameraConfig(img,&config);

	SetUpWidgetsProperty(capture,img,&config);

	args.filler = InitTimeFiller();
	args.area = lookup_widget(capture,"picture");
	args.img = img;

	gtk_idle_add((GtkFunction)read_frame,(gpointer)&args);

	gtk_main ();
	FreeTimeFiller(args.filler);
	GetCameraConfig(img,&config);
	PutConfigFile(Config,para);

	StopCamera(img);

    exit (EXIT_SUCCESS);

	return 0;
}

