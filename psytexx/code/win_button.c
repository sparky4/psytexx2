/*
    PsyTexx: win_button.cpp. Button handler
    Copyright (C) 2002 - 2007  Zolotov Alexandr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//*** Contact info: Zolotov Alexandr (NightRadio project)
//***               Ekaterinburg. Russia.
//***               Email: nightradio@gmail.com
//***               WWW: warmplace.ru

#include "../win_main.h"
#include "../win_button.h"
#include "time/timemanager.h"

#ifndef TEXTMODE
char txt_up[4] = { 1, 0 };
char txt_down[4] = { 2, 0 };
char txt_left[4] = { 3, 0 };
char txt_right[4] = { 4, 0 };
#else
char txt_up[4] = { '/', 0x5C, 0 };
char txt_down[4] = { 0x5C, '/', 0 };
char txt_left[4] = { '<', 0 };
char txt_right[4] = { '>', 0 };
#endif

void button_set_color( COLOR color, long win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    button_data *data = (button_data*)win->data;
    data->color = color;
}

void button_set_name(char* name, long win_num, window_manager *wm)
{
    window *win = wm->windows[ win_num ]; //Our window
    button_data *data = (button_data*)win->data;
    long len = 0;

    data->name = name;
    for(;;)
    {
	if( name[len] == 0 ) break;
	len++;
    }

    data->len = len;
}

void button_set_handler( long (*handler)(void*,long,window_manager*), void *user_data, long win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    button_data *data = (button_data*)win->data;

    data->handler = (long (*)(void*,long,void*))handler;
    data->user_data = user_data;
}

void button_set_autorepeat( char autorepeat, long win_num, window_manager* wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    button_data *data = (button_data*)win->data;
    data->autorepeat = autorepeat;
}

void button_autorepeat_handler( void *bdata, window_manager *wm )
{
    button_data *data = (button_data*)bdata;

    data->autorepeat_pressed = 1;
    if( data->handler ) data->handler( data->user_data, data->this_window, (void*)wm );
    wm_timer_set( 80, &button_autorepeat_handler, (void*)data, wm ); //Set it again
}

long button_handler(event *evt, window_manager *wm)
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    button_data *data = (button_data*)win->data;
    int offset = 0;

    switch( evt->event_type )
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(button_data), "button data", evt->event_win );
	    //Init data:
	    data = (button_data*)win->data;
	    data->this_window = evt->event_win;
	    data->name = 0;
	    data->color = win->color;
	    data->pressed = 0;
	    data->autorepeat = 0;
	    data->autorepeat_pressed = 0;
	    data->handler = 0;
	    data->user_data = 0;
	    break;
	case EVT_BEFORECLOSE:
	    if( win->data ) mem_free( win->data );
	    break;
	case EVT_SHOW:
	    //Show window:
	    win->visible = 1; //Make it visible
	    send_event(evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Send DRAW event to all childs
	    break;
	case EVT_HIDE:
	    win->visible = 0;
	    break;
	case EVT_DRAW:
	    //Draw window (if it's visible)
	    if( win->visible )
	    {
		data->x = ( (win->x_size - data->len) * wm->char_x ) >> 1;
		data->y = ( (win->y_size - 1) * wm->char_y ) >> 1;

		//draw_window_touch_area( evt->event_win, wm );
		draw_touch_box( win->parent_win, win->x, win->y, win->x_size, win->y_size, evt->event_win, wm );
		if( data->pressed )
		{
		    win->color = blend( data->color, 0, 64 );
		    offset = 1;
		}
		else
		{
		    win->color = data->color;
		    offset = 0;
		}
		draw_window_box( evt->event_win, wm ); //draw window box
		if( data->name )
		    draw_string( evt->event_win, data->x + offset, data->y + offset, win->color, data->name, wm );
	    }
	    break;
	case EVT_REDRAW:
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		data->x = ( (win->x_size - data->len) * wm->char_x ) >> 1;
		data->y = ( (win->y_size - 1) * wm->char_y ) >> 1;

		if( data->pressed )
		{
		    win->color = blend( data->color, 0, 64 );
		    offset = 1;
		}
		else
		{
		    win->color = data->color;
		    offset = 0;
		}
		draw_window_box( evt->event_win, wm ); //draw window box
		if( data->name )
		    draw_string( evt->event_win, data->x + offset, data->y + offset, win->color, data->name, wm );
	    }
	    break;
	case EVT_BUTTONDOWN:
	    if( evt->button & BUTTON_LEFT )
	    {
		data->pressed = 1;
		win->color = blend( data->color, 0, 64 );
		draw_window_box(evt->event_win,wm);
		if( data->name )
		    draw_string( evt->event_win, data->x + 1, data->y + 1, win->color, data->name, wm );
		//Autorepeat control:
		if( data->autorepeat )
		{
		    wm_timer_set( 700, &button_autorepeat_handler, (void*)data, wm );
		}
	    }
	    break;
	case EVT_BUTTONUP:
	    if( evt->button & BUTTON_LEFT && data->pressed )
	    {
		data->pressed = 0;
		win->color = data->color;
		draw_window_box(evt->event_win,wm);
		if( data->name )
	    	    draw_string( evt->event_win, data->x, data->y, win->color, data->name, wm );

		if( evt->x < win->x_size * wm->char_x && evt->y < win->y_size * wm->char_y )
		{
		    if( evt->x >= 0 && evt->y >= 0 && data->autorepeat_pressed == 0 )
		    {
			//User defined handler:
			if( data->handler ) data->handler( data->user_data, evt->event_win, (void*)wm );
		    }
		}

		//Autorepeat control:
		if( data->autorepeat )
		{
		    data->autorepeat_pressed = 0;
		    wm_timer_close( wm );
		}
	    }
	    break;
    }
    return 0;
}
