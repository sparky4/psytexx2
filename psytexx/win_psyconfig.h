#ifndef __PSYWINCONFIG__
#define __PSYWINCONFIG__

#include "core/core.h"
#include "window_manager/wmanager.h"

//PROPERTIES:
#define PROP_VOLUME    "prop_config_vol"

//INTERNAL STRUCTURES:

typedef struct config_data
{
    long this_window;

    long scroll_volume;
//0000
	COLOR color[20];
    long button_close;
}config_data;

//FUNCTIONS:

//HANDLERS:

long scroll_vol_handler( void *user_data, long win, window_manager* ) sec1;
long button_cclose_handler( void *user_data, long win, window_manager* ) sec1;

//WINDOW HANDLERS:

long config_handler( event*, window_manager* ) sec1;

#endif

