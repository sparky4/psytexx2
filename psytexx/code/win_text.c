/*
    PsyTexx: win_text.cpp. Text window handler
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
#include "../win_text.h"

long NEW_TEXT_SIZE = MAX_TEXT_LEN;

void text_set_text( long win_num, char *text, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;
    long a;

    data->text[ 0 ] = 0;
    data->cursor_position = 0;
    //Get text length:
    for( a = 0; a < MAX_TEXT_LEN ; a++ ) if( text[ a ] == 0 ) break;
    data->text[ MAX_TEXT_LEN - 1 ] = 0;
    data->text_len = a;
    //Save text:
    mem_strcat( (char*)data->text, text );
    send_event( win_num, EVT_REDRAW, 0, 0, 0, 0, MODE_WINDOW, wm ); //Redraw text window
}

char* text_get_text( long win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    return (char*)data->text;
}

void text_set_readonly( long win_num, long readonly, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    data->readonly = readonly;
}

void text_set_numerical( long win_num, long numerical, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    data->numerical_flag = numerical;
    if( numerical )
    {
	//Create right/left buttons for numerical text field:
	data->button_left = create_window(
	    "textleft",
	    win->x_size - 4, 0,
	    2, win->y_size,
	    wm->colors[9], 1,
	    win_num,
	    &button_handler,
	    wm );
	button_set_name( "-", data->button_left, wm );
	button_set_autorepeat( 1, data->button_left, wm );
	button_set_handler( &button_left_handler, (void*)data, data->button_left, wm );
        data->button_right = create_window(
	    "textright",
    	    win->x_size - 2, 0,
	    2, win->y_size,
	    wm->colors[9], 1,
	    win_num,
	    &button_handler,
	    wm );
	button_set_name( "+", data->button_right, wm );
	button_set_autorepeat( 1, data->button_right, wm );
	button_set_handler( &button_right_handler, (void*)data, data->button_right, wm );
	//Show buttons:
	send_event( data->button_left, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( data->button_right, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
}

void text_set_caption( long win_num, char *caption, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    data->caption = (uchar*)caption;
}

void text_set_bounds( long win_num, long min, long max, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    data->min_value = min;
    data->max_value = max;
}

void text_set_value( long win_num, long value, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    int_to_string( value, (char*)data->text );

    long a;
    for( a = 0; a < MAX_TEXT_LEN ; a++ ) if( data->text[ a ] == 0 ) break;
    data->text_len = a;
    data->cursor_position = 0;
}

long text_get_value( long win_num, window_manager *wm )
{
	long a;
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    long value = 0;
    long sign = 1;
    for( a = 0; ; a++ )
    {
	if( data->text[ a ] == 0 ) break;
	value *= 10;
	if( data->text[ a ] == '-' ) sign = -sign;
	if( data->text[ a ] >= 48 && data->text[ a ] <= 57 )
	{ //Numbers:
	    value += data->text[ a ] - 48;
	}
    }
    return value * sign;
}

long button_left_handler( void* user_data, long button_win, window_manager* wm )
{
    text_data *data = (text_data*)user_data;

    long value = text_get_value( data->this_window, wm );
    value--;
    if( value < data->min_value ) value = data->min_value;
    text_set_value( data->this_window, value, wm );

    //Redraw window:
    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_WINDOW, wm );

    //User defined handler:
    if( data->handler ) data->handler( data->user_data, data->this_window, (void*)wm );

    return 0;
}

long button_right_handler( void* user_data, long button_win, window_manager* wm )
{
    text_data *data = (text_data*)user_data;

    long value = text_get_value( data->this_window, wm );
    value++;
    if( value > data->max_value ) value = data->max_value;
    text_set_value( data->this_window, value, wm );

    //Redraw window:
    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_WINDOW, wm );

    //User defined handler:
    if( data->handler ) data->handler( data->user_data, data->this_window, (void*)wm );

    return 0;
}

void text_set_handler( long (*handler)(void*,long,window_manager*), void *user_data, long win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    text_data *data = (text_data*)win->data;

    data->handler = (long (*)(void*,long,void*))handler;
    data->user_data = user_data;
}

void text_add_char( text_data *data, uchar s )
{
    long a;
    for( a = data->text_len + 1; a > data->cursor_position; a-- )
    {
        data->text[ a ] = data->text[ a - 1 ];
    }
    data->text[ data->cursor_position ] = s;
    data->cursor_position++;
    data->text_len++;
}

long text_handler(event *evt, window_manager *wm)
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    text_data *data = (text_data*)win->data;
    long a;
    long button;
    long value;
    long retval = 0;
	ulong resulted_char;

    switch( evt->event_type )
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(text_data), "menu data", evt->event_win );

	    //Init data:
	    data = (text_data*)win->data;
	    data->this_window = evt->event_win;
	    data->text = (uchar*)mem_new( HEAP_DYNAMIC, NEW_TEXT_SIZE, "text data", evt->event_win );
	    NEW_TEXT_SIZE = MAX_TEXT_LEN;
	    data->readonly = 0;
	    data->text[ 0 ] = 0;
	    data->text_len = 0;
	    data->cursor_position = 0;
	    data->shift_status = 0;
	    data->numerical_flag = 0;
	    data->caption = 0;
	    data->min_value = 0;
	    data->max_value = 0xFFFFFF;
	    data->handler = 0;
	    data->active = 1;
	    break;

	case EVT_BEFORECLOSE:
	    if( win->data )
	    {
		if( data->text ) mem_free( data->text );
	    }
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
		//draw_window_touch_area( evt->event_win, wm );
		draw_touch_box( win->parent_win, win->x, win->y, win->x_size, win->y_size, evt->event_win, wm );

		draw_window_box(evt->event_win,wm); //draw window box
		//Draw cursor:
		if( !data->readonly && data->active )
		if( wm->mouse_win == evt->event_win )
		pdraw_box( evt->event_win,
		           data->cursor_position * wm->char_x,
			   (win->y_size - 1) * wm->char_y,
			   wm->char_x, wm->char_y,
			   wm->colors[3],
			   wm );
		if( data->caption )
		{
		    pdraw_box( evt->event_win,
			0, 0,
			win->x_size * wm->char_x, wm->char_y,
			wm->colors[9],
			wm );
		    draw_string( evt->event_win, 0, 0, 0, (char*)data->caption, wm ); //Draw caption
		}
		draw_string(
		    evt->event_win,
		    0, (win->y_size - 1) * wm->char_y,
		    0, (char*)data->text,
		    wm ); //draw text
	    }

	    break;

	case EVT_REDRAW:
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box( evt->event_win, wm ); //draw window box
		//Draw cursor:
		if( !data->readonly && data->active )
		if( wm->mouse_win == evt->event_win )
		pdraw_box( evt->event_win,
		           data->cursor_position * wm->char_x,
			   (win->y_size - 1) * wm->char_y,
			   wm->char_x, wm->char_y,
			   wm->colors[3],
			   wm );
		if( data->caption )
		{
		    pdraw_box( evt->event_win,
			0, 0,
			win->x_size * wm->char_x, wm->char_y,
			wm->colors[9],
			wm );
		    draw_string( evt->event_win, 0, 0, 0, (char*)data->caption, wm ); //Draw caption
		}
		draw_string(
		    evt->event_win,
		    0, (win->y_size - 1) * wm->char_y,
		    0, (char*)data->text,
		    wm ); //draw text
	    }
	    break;

	case EVT_BUTTONDOWN:
	    if( data->readonly ) break;
	    if( evt->button & BUTTON_LEFT )
	    {
		data->active = 1;
		data->cursor_position = evt->x / wm->char_x;
		if( data->cursor_position >= data->text_len )
		    data->cursor_position = data->text_len;
		send_event( evt->event_win, EVT_REDRAW, 0, 0, 0, 0, MODE_WINDOW, wm ); //Redraw text
	    }
	    button = evt->button >> 3;
	    data->shift_status = button & KEY_SHIFT;
	    button = button & 511; //Key code without SHIFT/ALT/CTRL flags
	    if( button && data->active )
	    { //Keyboard press:
		switch( button )
		{
		    case KEY_ENTER:
			data->active = 0;
			if( data->numerical_flag )
			{
		    	    value = text_get_value( data->this_window, wm );
			    if( value > data->max_value ) value = data->max_value;
			    if( value < data->min_value ) value = data->min_value;
			    text_set_value( data->this_window, value, wm );
		    	    //Redraw window:
		    	    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_WINDOW, wm );
			}
		        //User defined handler:
			if( data->handler ) data->handler( data->user_data, data->this_window, (void*)wm );
			break;
		    case KEY_LEFT:
			data->cursor_position--;
			if( data->cursor_position < 0 ) data->cursor_position = 0;
			break;
		    case KEY_RIGHT:
			data->cursor_position++;
			if( data->cursor_position >= data->text_len )
			    data->cursor_position = data->text_len;
			break;
		    case KEY_BACKSPACE:
			a = data->cursor_position - 1;
			if( a >= 0 )
			{
			    data->cursor_position = a;
			    for( ; a < data->text_len ; a++ )
			    {
				data->text[ a ] = data->text[ a + 1 ];
				if( data->text[ a ] == 0 ) data->text_len = a;
			    }
			}
			break;
		    case KEY_DELETE:
			a = data->cursor_position;
			for( ; a < data->text_len; a++ )
			{
			    data->text[ a ] = data->text[ a + 1 ];
			    if( data->text[ a ] == 0 ) data->text_len = a;
			}
			break;
		    case KEY_HOME:
			data->cursor_position = 0;
			break;
		    case KEY_END:
			data->cursor_position = data->text_len;
			if( data->cursor_position < 0 ) data->cursor_position = 0;
			break;
		    case KEY_SPACE:
			text_add_char( data, ' ' );
			break;
		    default:
			resulted_char = button & 0xFF; //Default: ASCII code
			if( button >= 0x30 && button <= 0x39 )
			{ //Digits:
			    if( data->shift_status )
			    {
				switch( button )
				{
				    case 0x30: resulted_char = ')'; break;
				    case 0x31: resulted_char = '!'; break;
				    case 0x32: resulted_char = '@'; break;
				    case 0x33: resulted_char = '#'; break;
				    case 0x34: resulted_char = '$'; break;
				    case 0x35: resulted_char = '%'; break;
				    case 0x36: resulted_char = '^'; break;
				    case 0x37: resulted_char = '&'; break;
				    case 0x38: resulted_char = '*'; break;
				    case 0x39: resulted_char = '('; break;
				}
			    }
			}
			if( button >= 0x61 && button <= 0x7A )
			{ //small letters:
			    if( data->shift_status )
				resulted_char = (uchar)(button - 0x20);
			}
			//Other symbols:
			if( data->shift_status )
			{ //With pressed shift:
			    switch( button )
			    {
			        case '-': resulted_char = '_'; break;
			        case '=': resulted_char = '+'; break;
			        case '[': resulted_char = '{'; break;
				case ']': resulted_char = '}'; break;
				case ';': resulted_char = ':'; break;
				case 0x27: resulted_char = 0x22; break;
				case ',': resulted_char = '<'; break;
				case '.': resulted_char = '>'; break;
				case '/': resulted_char = '?'; break;
				case 0x5C: resulted_char = '|'; break;
				case '`': resulted_char = '~'; break;
			    }
			}
			//Add resulted char:
			text_add_char( data, (uchar)resulted_char );
			break;
		} //End of switch( button )
		retval = 1;
	    } //End of if( button & 511 )
	    //Redraw text
	    send_event( evt->event_win, EVT_REDRAW, 0, 0, 0, 0, MODE_WINDOW, wm );
	    break;

	case EVT_UNFOCUS:
	    send_event( evt->event_win, EVT_REDRAW, 0, 0, 0, 0, MODE_WINDOW, wm );
	    break;

	case EVT_BUTTONUP:
	    if( data->readonly ) break;
	    button = evt->button >> 3;
	    if( button )
	    { //Keyboard:
	    }
	    break;
    }
    return retval;
}
