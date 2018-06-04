#include <gtk/gtk.h>

#include	"image.h"

#undef	GLOBAL
#ifdef	MAIN
#define	GLOBAL		/*	*/
#else
#define	GLOBAL		extern
#endif

GLOBAL	IMAGE	*ThisImage;

gboolean
on_capture_destroy_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_brightness_value_changed            (GtkRange        *range,
                                        gpointer         user_data);

void
on_contrast_value_changed              (GtkRange        *range,
                                        gpointer         user_data);

void
on_hue_value_changed                   (GtkRange        *range,
                                        gpointer         user_data);

void
on_saturation_value_changed            (GtkRange        *range,
                                        gpointer         user_data);


void
on_gamma_value_changed                 (GtkRange        *range,
                                        gpointer         user_data);

gboolean
on_capture_destroy_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_xoff_value_changed                  (GtkRange        *range,
                                        gpointer         user_data);

void
on_yoff_value_changed                  (GtkRange        *range,
                                        gpointer         user_data);

void
on_zoom_value_changed                  (GtkRange        *range,
                                        gpointer         user_data);
