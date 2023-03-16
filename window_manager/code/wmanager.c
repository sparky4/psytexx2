/*
    PsyTexx: wmanager.cpp. Multiplatform window-manager
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

#include "font.h"

#include "core/debug.h"
#include "time/timemanager.h"
#include "../wmanager.h"
#include "filesystem/v3nus_fs.h"

#ifndef NONPALM
    #include <PalmOS.h>
    #include "PceNativeCall.h"
    BitmapType    *active_part_low;  //Low density bitmap (active screen part)
    BitmapTypeV3  *active_part;      //High density bitmap (active screen part)
#ifdef NATIVEARM
    #include "palm_functions.h"
#endif
#endif

#ifdef LINUX
#ifdef TEXTMODE
#include <gpm.h>    //Mouse library for console mode
    char *gray_scale = " .-+%$#@";
#endif
#endif

#ifdef OPENGL
    float screen_transparency = 1.0F;
#endif

//#define DRAW_BACKGROUND

long pscreen_x_size = 320;
long pscreen_y_size = 320;
long char_x_size = 8;
long char_y_size = 8;


void win_init( window_manager *wm )
{
    long a;
    wm->wm_initialized = 0;

    prints( "MAIN: trying to load font" );
    load_font( wm ); //FIRST STEP: FONT LOADING and NEW CHAR SIZE SETTING

    //SECOND STEP: SCREEN SIZE SETTING and some device init
    prints( "MAIN: device start" );
    device_start( wm ); //DEVICE DEPENDENT PART (defined in eventloop.h)

    wm->eventloop_handler = 0;
    wm->constant_window = 0;
    wm->mouse_button_pressed = 0;
    wm->ghost_win = 0;

    wm->timer_handler = 0;

    wm->char_x = char_x_size;
    wm->char_y = char_y_size;
    wm->screen_x = pscreen_x_size / char_x_size;
    wm->screen_y = pscreen_y_size / char_y_size;
    wm->pscreen_x = wm->screen_x * wm->char_x;
    wm->pscreen_y = wm->screen_y * wm->char_y;
    wm->part_size = wm->pscreen_x * wm->char_y;
    wm->long_to_string_mode = 0;
    wm->mouse_win = 0;

#ifdef TEXTMODE
    wm->text_screen = (long*)mem_new( 0, wm->screen_x * wm->screen_y * 4, "text screen", 0 );
    mem_set( wm->text_screen, wm->screen_x * wm->screen_y * 4, 0 );
#endif
    prints2( "MAIN: screenX = ", pscreen_x_size );
    prints2( "MAIN: screenY = ", pscreen_y_size );
    prints( "MAIN: creating g_screen" );
    wm->g_screen = (COLORPTR)mem_new( 0, wm->pscreen_x * wm->pscreen_y * COLORLEN, "g_screen", 0 );
    prints( "MAIN: creating t_screen" );
    wm->t_screen = (unsigned short*)mem_new( 0, wm->screen_x * wm->screen_y * 2, "t_screen", 0 );
    wm->b_screen = 0; //no background
    wm->screen_changes = (uchar*)mem_new( 0, wm->screen_y, "screen changes", 0 );
    wm->g_matrix = (COLORPTR)mem_new( 0, wm->screen_x * wm->screen_y * COLORLEN, "g_matrix", 0 );
    wm->t_matrix = (uchar*)mem_new( 0, wm->screen_x * wm->screen_y, "t_matrix", 0 );
    wm->a_matrix = (uchar*)mem_new( 0, wm->screen_x * wm->screen_y, "a_matrix", 0 );
    wm->w_matrix = (uint16*)mem_new( 0, wm->screen_x * wm->screen_y * 2, "w_matrix", 0 );
    wm->matrix_is_empty = 1;
    prints( "MAIN: creating screen part" );
    create_active_screen_part( wm );

    prints( "MAIN: setting memory" );
    mem_set(wm->g_screen, wm->pscreen_x * wm->pscreen_y * COLORLEN, 0);
    mem_set(wm->t_screen, wm->screen_x * wm->screen_y * 2, 0);
    mem_set(wm->screen_changes, wm->screen_y, 0);
    mem_set(wm->g_matrix, wm->screen_x * wm->screen_y * COLORLEN, 0);
    mem_set(wm->t_matrix, wm->screen_x * wm->screen_y, 0);
    mem_set(wm->a_matrix, wm->screen_x * wm->screen_y, 0);
    mem_set(wm->w_matrix, wm->screen_x * wm->screen_y * 2, 0);

    prints( "MAIN: set palette" );
    set_palette( wm );

    for(a=0;a<MAX_WINDOWS;a++) wm->windows[a] = 0;

    for(a=0;a<MAX_EVENTS;a++) wm->events[a] = (event*)mem_new( 0, sizeof(event), "event", 0 );
    wm->event_pnt = 0;
    wm->events_count = 0;
    wm->current_event = (event*)mem_new( 0, sizeof(event), "current event", 0 );

//0000
	wm->font_shadow_color = get_color( 80, 80, 80 );

    wm->wm_initialized = 1;
}

void win_close(window_manager *wm)
{
    long a;
    //close all windows first:
    prints( "close windows" );
    for( a = 0; a < MAX_WINDOWS; a++ ) close_window( a, wm );

    //then close all events:
    prints( "close events" );
    for( a = 0; a < MAX_EVENTS; a++ ) mem_free( wm->events[ a ] );
    mem_free( wm->current_event );

    //and then...  check for the user's credit-card parameters and send it to observer_page@mail.ru
    prints( "close active screen part" );
    close_active_screen_part( wm );
    mem_free( wm->g_screen );
    mem_free( wm->t_screen );
    if( wm->b_screen ) mem_free( wm->b_screen );
    mem_free( wm->screen_changes );
    mem_free( wm->g_matrix );
    mem_free( wm->t_matrix );
    mem_free( wm->a_matrix );
    mem_free( wm->w_matrix );
    if( wm->user_font )
	mem_free( wm->user_font );
#ifdef TEXTMODE
    mem_free( wm->text_screen );
#endif

    prints( "device end" );
    device_end( wm ); //DEVICE DEPENDENT PART (defined in eventloop.h)
}

void wm_timer_set( ulong t, void (*handler)( void*, window_manager* ), void* data, window_manager *wm )
{
    wm->timer_handler = handler;
    wm->timer = ( ( time_ticks_per_second() * t ) >> 10 ) + time_ticks();
    wm->timer_handler_data = data;
}

void wm_timer_close( window_manager *wm )
{
    wm->timer_handler = 0;
}

long run_string( char *str, long default_value, long max_value, long w, window_manager *wm )
{
    long retval = default_value;
    long sizelimit = 0; // 1 - retval >= x; 2 - retval <= x;
    long limit = 0;
    long *val = &retval;
    long par[ 32 ];  //parameter of an interpreted command
    char type[ 32 ]; //type of an interpreted command:
                     // 0 - number;     example: str = "128"
		     // 1 - percent;    example: str = "128%"
		     // 2 - multiply;   example: str = "2*4"
		     // 3 - divide;     example: str = "4/2"
		     // 4 - add;        example: str = "4+2"
		     // 5 - sub;        example: str = "6-2"
		     // 6 - window;     example: str = "desktop"
		     // 7 - x;          example: str = "desktop.x"
		     // 8 - y;          example: str = "desktop.y"
		     // 9 - x2;         example: str = "desktop.x2"
		     //10 - y2;         example: str = "desktop.y2"
		     //11 - retval >=;  example: str = "1 >14"
		     //12 - retval <=;  example: str = "2 <14"
    long comm = 0;   //number of interpreted commands
    long num = 0;    //current number;
    long current_win = w;
    int a, a2;
    window *sw;
    int sp;
    int dif = 0;
    for( a = 0; ; a++ )
    {
	char c = str[ a ];
	if( c == 0 ) break;
	switch( c )
	{
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	    //number:
	    num *= 10; num += c-'0';
	    if( str[a+1] < '0' || str[a+1] > '9' )
	    {
		//Next symbol is not a digit
		par[ comm ] = num;
		if( str[a+1] == '%' ) type[ comm ] = 1; else type[ comm ] = 0;
		num = 0;
		comm++;
	    }
	    break;

	    case '*': type[ comm++ ] = 2; break;

	    case '/': type[ comm++ ] = 3; break;

	    case '+': type[ comm++ ] = 4; break;

	    case '-': type[ comm++ ] = 5; break;

	    case '%': break;

	    case '>': type[ comm++ ] = 11; break;
	    case '<': type[ comm++ ] = 12; break;

	    case ' ': break;

	    case '.':
	    //window property:
	    if( str[a+1] == 'x' && str[a+2] == '2' )
	    { //x2
		a += 2;
		type[ comm++ ] = 9;
	    }
	    else if( str[a+1] == 'y' && str[a+2] == '2' )
	    { //y2
		a += 2;
		type[ comm++ ] = 10;
	    }
	    else if( str[a+1] == 'x' )
	    { //x
		a++;
		type[ comm++ ] = 7;
	    }
	    else if( str[a+1] == 'y' )
	    { //y
		a++;
		type[ comm++ ] = 8;
	    }
	    break;

	    default:
	    //window name:
	    for( a2 = 0; a2 < MAX_WINDOWS; a2++ )
	    { //Search for an window:
		sw = wm->windows[ a2 ];
		if( sw )
		{ //if window is exist:
		    if( sw->name )
		    { //if window has name:
			dif = 0; //no differences for default
			for( sp = 0; ; sp++ )
			{ //searching name differences:
			    if( sw->name[sp] == 0 )
			    {
				if( str[a+sp] >= 'a' && str[a+sp] <= 'z' ) dif = 1;
				if( str[a+sp] >= 'A' && str[a+sp] <= 'Z' ) dif = 1;
				break;
			    }
			    if( sw->name[sp] != str[a+sp] ) { dif = 1; break; }
			}
			if( dif == 0 ) { current_win = a2; break; }
		    }
		}
	    }
	    par[ comm ] = current_win;
	    type[ comm ] = 6;
	    comm++;
	    //go to the end of current string (window name):
	    while( (str[a]>='a' && str[a]<='z') || (str[a]>='A' && str[a]<='Z') ) a++;
	    a--;
	    break;
    	}
    }

    //Interpret commands:
    for( a = 0; a < comm; a++ )
    {
	switch( type[ a ] )
	{
	    case 0: *val = par[a]; break;
	    case 1: *val = ( par[a] * max_value ) / 100; break;
	    case 2: //Multiply:
		if( type[a+1] == 0 ) { *val *= par[a+1]; a++; }
		else if( type[a+1] == 1 ) { *val *= (( par[a] * max_value ) / 100); a++; }
	    break;
	    case 3: //Divide:
		if( type[a+1] == 0 ) { *val /= par[a+1]; a++; }
		else if( type[a+1] == 1 ) { *val /= (( par[a] * max_value ) / 100); a++; }
	    break;
	    case 4: //Add:
		if( type[a+1] == 0 ) { *val += par[a+1]; a++; }
		else if( type[a+1] == 1 ) { *val += (( par[a] * max_value ) / 100); a++; }
	    break;
	    case 5: //Sub:
		if( type[a+1] == 0 ) { *val -= par[a+1]; a++; }
		else if( type[a+1] == 1 ) { *val -= (( par[a] * max_value ) / 100); a++; }
	    break;
	    case 6: //Window property:
	    current_win = par[a];
	    sw = wm->windows[ current_win ];
	    if( type[a+1] == 7 ) *val = sw->x;
	    else if( type[a+1] == 8 ) *val = sw->y;
	    else if( type[a+1] == 9 ) *val = sw->x_size + sw->x;
	    else if( type[a+1] == 10 ) *val = sw->y_size + sw->y;
	    a++;
	    break;
	    case 7: sw = wm->windows[w]; *val = sw->x; break;
	    case 8: sw = wm->windows[w]; *val = sw->y; break;
	    case 9: sw = wm->windows[w]; *val = sw->x + sw->x_size; break;
	    case 10: sw = wm->windows[w]; *val = sw->y + sw->y_size; break;
	    case 11: //Retval must be >= x:
	    sizelimit = 1;
	    val = &limit;
	    break;
	    case 12: //Retval must be <= x:
	    sizelimit = 2;
	    val = &limit;
	    break;
	}
    }

    if( sizelimit == 1 )
    {
	if( retval < limit ) retval = limit;
    }
    if( sizelimit == 2 )
    {
	if( retval > limit ) retval = limit;
    }
    return retval;
}

void resize_all_windows( window_manager *wm )
{
    int a;
    long max_x, max_y;
    window *w;
    event evt;
    for( a = 0; a < MAX_WINDOWS; a++ )
    {
	if( wm->windows[ a ] )
	{
	    w = wm->windows[ a ];
	    w->scroll_flags &= 0xF0; //Clear all flags except of SF_STATICWIN
	    if( w->parent_win != a )
	    {
		window *parent = wm->windows[ w->parent_win ];
		max_x = parent->x_size;
		max_y = parent->y_size;
	    }
	    else
	    {
		max_x = wm->screen_x;
		max_y = wm->screen_y;
	    }
	    long x = w->x;
	    long y = w->y;
	    long x2 = w->x + w->x_size;
	    long y2 = w->y + w->y_size;
	    long temp;
	    if( w->xs && w->xs[0]!=0 ) x = run_string( w->xs, w->x, max_x, a, wm );
	    if( w->ys && w->ys[0]!=0 ) y = run_string( w->ys, w->y, max_y, a, wm );
	    if( w->xs2 && w->xs2[0]!=0 )
	    {
		x2 = run_string( w->xs2, w->x + w->x_size, max_x, a, wm );
		if( x2 < x ) { temp = x; x = x2; x2 = temp; }
	    }
	    if( w->ys2 && w->ys2[0]!=0 )
	    {
		y2 = run_string( w->ys2, w->y + w->y_size, max_y, a, wm );
		if( y2 < y ) { temp = y; y = y2; y2 = temp; }
	    }
	    w->x = x;
	    w->y = y;
	    w->x_size = x2 - x;
	    w->y_size = y2 - y;

	    //Set real values:
	    calculate_real_window_position( a, wm );

	    evt.mode = MODE_WINDOW;
    	    evt.x = 0;
    	    evt.y = 0;
    	    evt.button = 0;
    	    evt.event_type = EVT_RESIZE;
    	    evt.event_win = a;

    	    //Process EVT_RESIZE:
    	    w->win_handler( &evt, wm );
	}
    }
}

void resize_all( long reload_background, window_manager *wm )
{
    close_active_screen_part( wm );
    create_active_screen_part( wm );
    wm->part_size = wm->pscreen_x * wm->char_y;

    long new_size;
#ifdef TEXTMODE
    //Text screen:
    new_size = wm->screen_x * wm->screen_y * 4;
    if( mem_get_size( wm->text_screen ) < new_size )
    {
	mem_free( wm->text_screen );
	wm->text_screen = (long*)mem_new( 0, new_size, "text screen", 0 );
	mem_set( wm->text_screen, new_size, 0 );
    }
#endif
    //Graphics screen:
    new_size = wm->pscreen_x * wm->pscreen_y * COLORLEN;
    if( mem_get_size( wm->g_screen ) < new_size )
    {
	mem_free( wm->g_screen );
	wm->g_screen = (COLORPTR)mem_new( 0, new_size+(new_size/2), "g_screen", 0 );
	if( wm->b_screen ) mem_free( wm->b_screen );
	wm->b_screen = 0;
	mem_set( wm->g_screen, new_size+(new_size/2), 0 );
    }
    //Background:
    if( reload_background )
    {
	if( wm->b_screen ) mem_free( wm->b_screen );
	wm->b_screen = 0;
	load_background_bmp( wm );
    }
    //Other screens:
    new_size = wm->screen_x * wm->screen_y * 2;
    if( mem_get_size( wm->t_screen ) < new_size )
    {
	mem_free( wm->t_screen );
	mem_free( wm->g_matrix );
	mem_free( wm->t_matrix );
	mem_free( wm->a_matrix );
	mem_free( wm->w_matrix );
	wm->t_screen = (unsigned short*)mem_new( 0, wm->screen_x * wm->screen_y * 2*2, "t_screen", 0 );
	wm->g_matrix = (COLORPTR)mem_new( 0, wm->screen_x * wm->screen_y * COLORLEN * 2, "g_matrix", 0 );
	wm->t_matrix = (uchar*)mem_new( 0, wm->screen_x * wm->screen_y*2, "t_matrix", 0 );
	wm->a_matrix = (uchar*)mem_new( 0, wm->screen_x * wm->screen_y*2, "a_matrix", 0 );
	wm->w_matrix = (uint16*)mem_new( 0, wm->screen_x * wm->screen_y * 2*2, "w_matrix", 0 );
	mem_set( wm->t_screen, wm->screen_x * wm->screen_y * 2*2, 0 );
	mem_set( wm->g_matrix, wm->screen_x * wm->screen_y * COLORLEN*2, 0 );
	mem_set( wm->t_matrix, wm->screen_x * wm->screen_y*2, 0 );
	mem_set( wm->a_matrix, wm->screen_x * wm->screen_y*2, 0 );
	mem_set( wm->w_matrix, wm->screen_x * wm->screen_y * 2*2, 0 );
    }
    //Screen changes:
    if( mem_get_size( wm->screen_changes ) < wm->screen_y )
    {
	mem_free( wm->screen_changes );
	wm->screen_changes = (uchar*)mem_new( 0, wm->screen_y*2, "screen changes", 0 );
	mem_set( wm->screen_changes, wm->screen_y*2, 0 );
    }

    resize_all_windows( wm );
}

void int_to_string( int value, char *str )
{
    long n, p = 0, f = 0;

    if( value < 0 )
    {
    	str[ p++ ] = '-';
	value = -value;
    }

    n = value / 10000000; if( n || f ) { str[p++] = (char)n+48; value -= n * 10000000; f = 1; }
    n = value / 1000000; if( n || f ) { str[p++] = (char)n+48; value -= n * 1000000; f = 1; }
    n = value / 100000; if( n || f ) { str[p++] = (char)n+48; value -= n * 100000; f = 1; }
    n = value / 10000; if( n || f ) { str[p++] = (char)n+48; value -= n * 10000; f = 1; }
    n = value / 1000; if( n || f ) { str[p++] = (char)n+48; value -= n * 1000; f = 1; }
    n = value / 100; if( n || f ) { str[p++] = (char)n+48; value -= n * 100; f = 1; }
    n = value / 10; if( n || f ) { str[p++] = (char)n+48; value -= n * 10; f = 1; }
    str[p++] = (char)value+48;
    str[p] = 0;
}

void int_to_string_h( int value, char *str )
{
    long n, p = 0, f = 0;

    if( value < 0 )
    {
    	str[ p++ ] = '-';
	value = -value;
    }

    n = value >> 20; if( n || f ) { str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 20; f = 1; }
    n = value >> 16; if( n || f ) { str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 16; f = 1; }
    n = value >> 12; if( n || f ) { str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 12; f = 1; }
    n = value >> 8; if( n || f ) { str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 8; f = 1; }
    n = value >> 4; if( n || f ) { str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 4; f = 1; }
    str[p] = (char)value+48; if(value>9) str[p]+=7; p++;
    str[p] = 0;
}

void ext_long_to_string( long value, char *str, long chars, window_manager *wm )
{
    long n, p = 0, f = 0;

    switch( chars )
    {
	case 1:
	break;

	case 2:
	n = value >> 4; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 4;
	break;

	case 3:
	n = value >> 8; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 8;
	n = value >> 4; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 4;
	break;

	case 4:
        n = value >> 12; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 12;
	n = value >> 8; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 8;
	n = value >> 4; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 4;
	break;

	default:
	n = value >> 20; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 20;
        n = value >> 16; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 16;
        n = value >> 12; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 12;
        n = value >> 8; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 8;
        n = value >> 4; str[p] = (char)n+48; if(n>9) str[p]+=7; p++; value -= n << 4;
	break;
    }

    str[p] = (char)value+48; if(value>9) str[p]+=7; p++;
    str[p] = 0;
}

long get_color( uchar r, uchar g, uchar b )
{
    long res = 0; //resulted color
#ifdef COLOR8BITS
    res += ( b & 192 );
    res += ( g & 224 ) >> 2;
    res += ( r >> 5 );
#endif
#ifdef COLOR16BITS
    res += ( r >> 3 ) << 11;
    res += ( g >> 2 ) << 5;
    res += ( b >> 3 );
#endif
#ifdef COLOR32BITS
    res += 255 << 24;
    res += r << 16;
    res += g << 8;
    res += b;
#endif
    return res;
}

long red( COLOR c )
{
    long res = 0;
#ifdef COLOR8BITS
    res = ( c << 5 ) & 255;
#endif
#ifdef COLOR16BITS
    res = ( ( c >> 11 ) << 3 ) & 0xF8;
#endif
#ifdef COLOR32BITS
    res = ( c >> 16 ) & 255;
#endif
    return res;
}

long green( COLOR c )
{
    long res = 0;
#ifdef COLOR8BITS
    res = ( c << 2 ) & 0xE0;
#endif
#ifdef COLOR16BITS
    res = ( ( c >> 5 ) << 2 ) & 0xFC;
#endif
#ifdef COLOR32BITS
    res = ( c >> 8 ) & 255;
#endif
    return res;
}

long blue( COLOR c )
{
    long res = 0;
#ifdef COLOR8BITS
    res = c & 192;
#endif
#ifdef COLOR16BITS
    res = ( c << 3 ) & 0xF8;
#endif
#ifdef COLOR32BITS
    res = c & 255;
#endif
    return res;
}

COLOR blend( COLOR c1, COLOR c2, int value )
{
    int r1 = red( c1 );
    int g1 = green( c1 );
    int b1 = blue( c1 );
    int r2 = red( c2 );
    int g2 = green( c2 );
    int b2 = blue( c2 );
    int r3 = ( r1 * ( 255 - value ) ) + ( r2 * value );
    int g3 = ( g1 * ( 255 - value ) ) + ( g2 * value );
    int b3 = ( b1 * ( 255 - value ) ) + ( b2 * value );
    return get_color( r3 >> 8, g3 >> 8, b3 >> 8 );
}

void load_font( window_manager *wm )
{
    FILE *f;
    uchar *bmp_data;
    uchar temp[32];
    long file_size;    //BMP file size
    long xsize, ysize; //BMP size
    long x,y,r,g,b,res;
    long pnt;  //BMP pointer
    long pnt2; //converted image pointer
    long linesize, char_xsize;
    long screen_x, screen_y;

    long fontx, fonty;
    long font_ptr;
    long bmp_ptr;

    wm->user_font = 0;

    //Get physical screen size:
    screen_x = pscreen_x_size;
    screen_y = pscreen_y_size;

    mem_off(); //Becouse bmp_data will be in storage heap (PalmOS specific)

    f = fopen( "font.bmp", "rb" );
    if( f != 0 )
    { //File exist:
	fread( temp, 2, 1, f );      //First two bytes
	fread( &file_size, 4, 1, f ); //BMP file size

	bmp_data = (uchar*)mem_new( HEAP_STORAGE, file_size, "BMP data", 0 );

	fread( temp, 4, 1, f );
	fread( temp, 4, 1, f );
	fread( temp, 4, 1, f );
	fread( &xsize, 4, 1, f );
	fread( &ysize, 4, 1, f );
	fread( temp, 28, 1, f );

	fread( bmp_data, xsize * ysize * 3, 1, f ); //Load image data

	fclose( f ); //close file

	//Set font parameters:
	wm->user_font_x = xsize / 16;
	wm->user_font_y = ysize / 16;

	//Convert image data to 8bit or 16bit:
	wm->user_font = (COLORPTR)mem_new( HEAP_STORAGE, xsize * ysize * COLORLEN, "user font", 0 );
	linesize = xsize * 3;
	char_xsize = wm->user_font_x * 3;
	pnt2 = 0;
	for( y = 0; y < ysize; y += wm->user_font_y )
	{
	    pnt = linesize * (ysize - y - 1);
	    for( x = 0; x < xsize; x += wm->user_font_x )
	    {
		//Get one char:
		font_ptr = pnt2;
		bmp_ptr = pnt;
		for( fonty = 0; fonty < wm->user_font_y; fonty++ )
		{
		    for( fontx = 0; fontx < wm->user_font_x; fontx++ )
		    {
			b = bmp_data[ bmp_ptr++ ];
			g = bmp_data[ bmp_ptr++ ];
			r = bmp_data[ bmp_ptr++ ];
#ifdef COLOR8BITS
			r >>= 5;
			g >>= 5;
			b >>= 6;
			res = r + (g<<3) + (b<<6);
#endif
#ifdef COLOR16BITS
			r >>= 3;
			g >>= 2;
			b >>= 3;
			res = b + (g<<5) + (r<<11);
#ifdef NATIVEARM
			res = BSwap16( res );
#endif
#endif
#ifdef COLOR32BITS
			res = b + (g<<8) + (r<<16);
#ifdef NATIVEARM
			res = BSwap32( res );
#endif
#endif
			wm->user_font[ font_ptr ] = (COLOR)res;
			font_ptr++;
		    }
		    bmp_ptr -= linesize + char_xsize;
		}

		pnt += char_xsize;
		pnt2 += wm->user_font_x * wm->user_font_y;
	    }
	}
	mem_free( bmp_data );
    }

#ifdef PALMLOWRES
    if( 1 )
#else
    if( load_long( PROP_KBD_USERFONT ) == 1 )
#endif
    {
	char_x_size = wm->user_font_x;
	char_y_size = wm->user_font_y;
	wm->user_font_mode = 1;
    } else wm->user_font_mode = 0;

    mem_on();
}

void load_background_bmp( window_manager *wm )
{
#ifdef DRAW_BACKGROUND
    FILE *f;
    uchar *bmp_data;
    uchar temp[32];
    long file_size;    //BMP file size
    long xsize, ysize; //BMP size
    long x,y,r,g,b,res;
    long pnt;  //BMP pointer
    long pnt2; //converted image pointer
    long linesize;
    long bmp_delta_x;
    long bmp_delta_y;
    long bmp_xp, bmp_yp; //x/y pointers
    long max_y;

    mem_off(); //Becouse bmp_data will be in the storage heap (PalmOS specific)

    f = fopen( "back.bmp", "rb" );
    if( f != 0 )
    { //File exist:
	fread( temp, 2, 1, f );      //First two bytes
	fread( &file_size, 4, 1, f ); //BMP file size

	bmp_data = (uchar*)mem_new( HEAP_STORAGE, file_size, "BMP data", 0 );

	fread( temp, 4, 1, f );
	fread( temp, 4, 1, f );
	fread( temp, 4, 1, f );
	fread( &xsize, 4, 1, f );
	fread( &ysize, 4, 1, f );
	fread( temp, 28, 1, f );

	fread( bmp_data, xsize * ysize * 3, 1, f ); //Load image data

	fclose( f ); //close file

	ysize--; //One line used by PsyTexx as color palette

	//Convert image data to 8bit:
	wm->b_screen = (COLORPTR)mem_new( HEAP_STORAGE, wm->pscreen_x * wm->pscreen_y * COLORLEN, "b_screen", 0 );
	if( xsize == wm->pscreen_x && ysize == wm->pscreen_y )
	{
	    linesize = xsize * 3;
	    pnt2 = 0;
	    for( y = 0; y < ysize; y++ )
	    {
		pnt = linesize * (ysize - y);// - 1);
		for( x = 0; x < xsize; x++ )
		{
		    b = bmp_data[ pnt++ ];
		    g = bmp_data[ pnt++ ];
		    r = bmp_data[ pnt++ ];
#ifdef COLOR8BITS
		    if( y&1 )
		    {
			if( r&16 && r < 248 ) r += 8;
			if( g&16 && g < 248 ) g += 8;
			if( b&32 && b < 252 ) b += 4;
		    }
		    r >>= 5;
		    g >>= 5;
		    b >>= 6;
		    res = r + (g<<3) + (b<<6);
#endif
#ifdef COLOR16BITS
		    r >>= 3;
		    g >>= 2;
		    b >>= 3;
		    res = b + (g<<5) + (r<<11);
#ifdef NATIVEARM
		    res = BSwap16( res );
#endif
#endif
#ifdef COLOR32BITS
		    res = b + (g<<8) + (r<<16) + (255<<24);
#endif
		    wm->b_screen[ pnt2 ] = (COLOR)res;
		    pnt2++;
		}
	    }
	}
	else
	{ //Need to resize:
	    linesize = xsize * 3;
	    pnt2 = 0;
	    if( wm->pscreen_y ) bmp_delta_y = (ysize << 10) / wm->pscreen_y; else bmp_delta_y = 0;
	    if( wm->pscreen_x ) bmp_delta_x = (xsize << 10) / wm->pscreen_x; else bmp_delta_x = 0;
	    max_y =
	    pnt = 0;
	    for( y = 0; y < wm->pscreen_y; y++ )
	    {
		bmp_yp = ( ( (wm->pscreen_y - y - 1) * bmp_delta_y ) >> 10 ) * xsize;
		bmp_yp += xsize;
		bmp_xp = 0;
		for( x = 0; x < wm->pscreen_x; x++ )
		{
		    pnt = ( bmp_yp + (bmp_xp>>10) ) * 3;
		    b = bmp_data[ pnt++ ];
		    g = bmp_data[ pnt++ ];
		    r = bmp_data[ pnt++ ];
#ifdef COLOR8BITS
		    if( y&1 )
		    {
			if( r&16 && r < 248 ) r += 8;
			if( g&16 && g < 248 ) g += 8;
			if( b&32 && b < 252 ) b += 4;
		    }
		    r >>= 5;
		    g >>= 5;
		    b >>= 6;
		    res = r + (g<<3) + (b<<6);
#endif
#ifdef COLOR16BITS
		    r >>= 3;
		    g >>= 2;
		    b >>= 3;
		    res = b + (g<<5) + (r<<11);
#endif
#ifdef NATIVEARM
		    res = BSwap16( res );
#endif
#ifdef COLOR32BITS
		    res = b + (g<<8) + (r<<16) + (255<<24);
#endif
		    wm->b_screen[ pnt2 ] = (COLOR)res;
		    pnt2++;
		    bmp_xp += bmp_delta_x;
		}
	    }
	}

	pnt2 = 0;
	x = ( xsize / 16 ) * 3;
        for( long c = 0; c < 16; c++ )
	{
	    wm->colors[ c ] = get_color( bmp_data[ pnt2 + 2 ], bmp_data[ pnt2 + 1 ], bmp_data[ pnt2 ] );
	    pnt2 += x;
	}

	mem_free( bmp_data );
    }

    mem_on();
#endif
}

void draw_string( long win_num, long x, long y, long color, char* str, window_manager *wm )
{
    long a = 0;
    long pnt, pnt2, pnt3, pnt4;            //touch screen pointers
    long wx = 0, wy = 0;

    window *win = wm->windows[ win_num ];
    wx = win->real_x;
    wy = win->real_y;
    x += ( wx * wm->char_x ); y += ( wy * wm->char_y );
    pnt = ( ( y / wm->char_y ) * wm->screen_x ) + ( x / wm->char_x );
    pnt2 = ( ( y / wm->char_y ) * wm->screen_x ) + ( ( x + wm->char_x - 1 ) / wm->char_x );
    pnt3 = ( ( ( y + wm->char_y - 1 ) / wm->char_y ) * wm->screen_x ) + ( ( x + wm->char_x - 1 ) / wm->char_x );
    pnt4 = ( ( ( y + wm->char_y - 1 ) / wm->char_y ) * wm->screen_x ) + ( x / wm->char_x );
    int max_pnt = wm->screen_x * wm->screen_y;

    //Screen bounds control:
    if( y < 0 || y > wm->pscreen_y - wm->char_y ) return;

    long changed = 0;
    int start_x = x;
    int line_size = 0;
    for(;;)
    {
	if( str[a] == 0 ) break;
	if( str[a] == 0xA )
	{
	    if( changed && ( ( y / wm->char_y ) < wm->screen_y ) ) wm->screen_changes[ y / wm->char_y ] = 1;
	    changed = 0;
	    pnt += wm->screen_x - line_size;
	    pnt2 += wm->screen_x - line_size;
	    pnt3 += wm->screen_x - line_size;
	    pnt4 += wm->screen_x - line_size;
	    y += wm->char_y; x = start_x; line_size = 0;
	}
	if( str[a] != 0xD && str[a] != 0xA )
	{
	    if( x >= 0 && x <= wm->pscreen_x - wm->char_x &&
		y >= 0 && y <= wm->pscreen_y - wm->char_y )
		if( pnt >= 0 &&
		    wm->t_screen[pnt] == win_num &&
		    wm->t_screen[pnt2] == win_num &&
		    wm->t_screen[pnt3] == win_num &&
		    wm->t_screen[pnt4] == win_num
		)
		{
		    draw_char( x, y, (uchar)str[a], color, wm );
		    changed = 1;
		}
	    pnt++;
	    pnt2++;
	    pnt3++;
	    pnt4++;
	    line_size++;
	    x += wm->char_x;
	}
	a++;
    }
    if( changed && ( ( y / wm->char_y ) < wm->screen_y ) ) wm->screen_changes[ y / wm->char_y ] = 1;
}

void draw_pixel( long win_num, long x, long y, long color, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    x += win->real_x * wm->char_x; y += win->real_y * wm->char_y;

    if( x >= 0 && y >= 0 && x < wm->pscreen_x && y < wm->pscreen_y )
    if( wm->t_screen[ ( ( y / wm->char_y ) * wm->screen_x ) + ( x / wm->char_x ) ] == win_num )
    wm->g_screen[ y * wm->pscreen_x + x ] = color;
}

void draw_rectangle( long win_num, long x, long y, long x_size, long y_size, long color, window_manager *wm )
{
    long a;
    for( a = 0; a < x_size; a++ )
    {
	draw_pixel( win_num, x + a, y, color, wm );
	draw_pixel( win_num, x + a, y + y_size - 1, color, wm );
    }
    for( a = 0; a < y_size; a++ )
    {
        draw_pixel( win_num, x, y + a, color, wm );
        draw_pixel( win_num, x + x_size - 1, y + a, color, wm );
    }
}

void draw_rectangle2( long win_num, long x, long y, long x_size, long y_size, long color, window_manager *wm )
{
    long a;
    for( a = 0; a < x_size; a += 2 )
    {
	draw_pixel( win_num, x + a, y, color, wm );
	draw_pixel( win_num, x + a, y + y_size - 1, color, wm );
    }
    for( a = 0; a < y_size; a += 2 )
    {
        draw_pixel( win_num, x, y + a, color, wm );
        draw_pixel( win_num, x + x_size - 1, y + a, color, wm );
    }
}

#define bottom 1
#define top 2
#define left 4
#define right 8
int make_code( int x, int y, int clip_x, int clip_y )
{
    int code = 0;
    if( y >= clip_y ) code = bottom;
    else if( y < 0 ) code = top;
    if( x >= clip_x ) code += right;
    else if( x < 0 ) code += left;
    return code;
}

void draw_line( long win_num, int x1, int y1, int x2, int y2, COLOR color, window_manager *wm )
{
	int a;
    window *win = wm->windows[ win_num ];
    x1 += win->real_x * wm->char_x; y1 += win->real_y * wm->char_y;
    x2 += win->real_x * wm->char_x; y2 += win->real_y * wm->char_y;

    //Cohen-Sutherland line clipping algorithm:
    int code0;
    int code1;
    int out_code;
    int x, y;
    code0 = make_code( x1, y1, wm->pscreen_x, wm->pscreen_y );
    code1 = make_code( x2, y2, wm->pscreen_x, wm->pscreen_y );
    while( code0 || code1 )
    {
	if( code0 & code1 ) return; //Trivial reject
	else
	{
	    //Failed both tests, so calculate the line segment to clip
	    if( code0 )
		out_code = code0; //Clip the first point
	    else
		out_code = code1; //Clip the last point

	    if( out_code & bottom )
	    {
		//Clip the line to the bottom of the viewport
		y = wm->pscreen_y - 1;
		x = x1 + ( x2 - x1 ) * ( y - y1 ) / ( y2 - y1 );
	    }
	    else
	    if( out_code & top )
	    {
		y = 0;
		x = x1 + ( x2 - x1 ) * ( y - y1 ) / ( y2 - y1 );
	    }
	    else
	    if( out_code & right )
	    {
		x = wm->pscreen_x - 1;
		y = y1 + ( y2 - y1 ) * ( x - x1 ) / ( x2 - x1 );
	    }
	    else
	    if( out_code & left )
	    {
		x = 0;
		y = y1 + ( y2 - y1 ) * ( x - x1 ) / ( x2 - x1 );
	    }

	    if( out_code == code0 )
	    { //Modify the first coordinate
		x1 = x; y1 = y;
		code0 = make_code( x1, y1, wm->pscreen_x, wm->pscreen_y );
	    }
	    else
	    { //Modify the second coordinate
		x2 = x; y2 = y;
		code1 = make_code( x2, y2, wm->pscreen_x, wm->pscreen_y );
	    }
	}
    }

    if( wm->t_screen[ ( ( y1 / wm->char_y ) * wm->screen_x ) + ( x1 / wm->char_x ) ] != win_num ) return;
    if( wm->t_screen[ ( ( y2 / wm->char_y ) * wm->screen_x ) + ( x2 / wm->char_x ) ] != win_num ) return;

    //Draw line:
    int len_x = x2 - x1; if( len_x < 0 ) len_x = -len_x;
    int len_y = y2 - y1; if( len_y < 0 ) len_y = -len_y;
    int ptr = y1 * wm->pscreen_x + x1;
    int delta;
    int v = 0, old_v = 0;
    if( len_x > len_y )
    {
	//Horisontal:
	if( len_x != 0 )
	    delta = ( len_y << 10 ) / len_x;
	else
	    delta = 0;
	for( a = 0; a <= len_x; a++ )
	{
	    wm->g_screen[ ptr ] = color;
	    old_v = v;
	    v += delta;
	    if( x2 - x1 > 0 ) ptr++; else ptr--;
	    if( ( old_v >> 10 ) != ( v >> 10 ) )
	    {
		if( y2 - y1 > 0 )
		    ptr += wm->pscreen_x;
		else
		    ptr -= wm->pscreen_x;
	    }
	}
    }
    else
    {
	//Vertical:
	if( len_y != 0 )
	    delta = ( len_x << 10 ) / len_y;
	else
	    delta = 0;
	for( a = 0; a <= len_y; a++ )
	{
	    wm->g_screen[ ptr ] = color;
	    old_v = v;
	    v += delta;
	    if( y2 - y1 > 0 )
		ptr += wm->pscreen_x;
	    else
		ptr -= wm->pscreen_x;
	    if( ( old_v >> 10 ) != ( v >> 10 ) )
	    {
		if( x2 - x1 > 0 ) ptr++; else ptr--;
	    }
	}
    }
}

void draw_vert_line( long win_num, long x, long y, long y_size, long color, long add, window_manager *wm )
{
	long cy;
    for( cy = y; cy < y + y_size; cy += add )
    {
	draw_pixel( win_num, x, cy, color, wm );
    }
}

void draw_horis_line( long win_num, long x, long y, long x_size, long color, long add, window_manager *wm )
{
	long cx;
    for( cx = x; cx < x + x_size; cx += add )
    {
	draw_pixel( win_num, cx, y, color, wm );
    }
}

void pdraw_box( long win_num, long x, long y, long x_size, long y_size,
                long color, window_manager *wm )
{
    COLOR box_color = color;
    int r = red( color ) + 32; if( r > 255 ) r = 255;
    int g = green( color ) + 32; if( g > 255 ) g = 255;
    int b = blue( color ) + 32; if( b > 255 ) b = 255;
    COLOR light_color = get_color( r, g, b );
    r = red( color ) - 32; if( r < 0 ) r = 0;
    g = green( color ) - 32; if( g < 0 ) g = 0;
    b = blue( color ) - 32; if( b < 0 ) b = 0;
    COLOR shade_color = get_color( r, g, b );
#ifdef NATIVEARM
#ifdef COLOR16BITS
    box_color = (long)BSwap16( box_color );
    light_color = (long)BSwap16( light_color );
    shade_color = (long)BSwap16( shade_color );
#endif
#ifdef COLOR32BITS
    box_color = (long)BSwap32( box_color );
    light_color = (long)BSwap32( light_color );
    shade_color = (long)BSwap32( shade_color );
#endif
#endif
    long screen_size = wm->screen_x * wm->screen_y;
    long wx = 0, wy = 0;
    long cx, cy;
    long pnt;   //t_screen pointer
    long s_pnt; //g_screen pointer

    long xx, yy, xxsize, xxsize2, yysize;
    long cxx,cyy;

    long sx, sy, sxsize, sysize;
    long sx2, sy2;

    COLOR *background;   //background buffer;
    long c1 = 0, c2 = 0; //horisontal and vertical counters for pseudo-transparent background

    background = wm->b_screen;

    window *win = wm->windows[ win_num ];
    wx = win->real_x;
    wy = win->real_y;
    x += wx * wm->char_x; y += wy * wm->char_y;

    //Screen bounds control:
    if( x >= wm->pscreen_x ) return;
    if( x + x_size > wm->pscreen_x ) x_size -= ( x_size - ( wm->pscreen_x - x ) );
    if( x < 0 )
	if( x + x_size <= 0 ) return;
	    else { x_size += x; x = 0; }
    if( y >= wm->pscreen_y ) return;
    if( y + y_size > wm->pscreen_y ) y_size -= ( y_size - ( wm->pscreen_y - y ) );
    if( y < 0 )
	if( y + y_size <= 0 ) return;
	    else { y_size += y; y = 0; }

    xx = x / wm->char_x;
    yy = y / wm->char_y;
    long ch_y = yy;
    xxsize = ( ( x + x_size ) / wm->char_x ) - xx; xxsize++;
    yysize = ( ( y + y_size ) / wm->char_y ) - yy; yysize++;

    //Get touch pointer:
    pnt = ( yy * wm->screen_x ) + xx;
    long t_size = wm->screen_x * wm->screen_y;

    xx *= wm->char_x;
    yy *= wm->char_y;
    xxsize2 = xxsize;
    xxsize *= wm->char_x;
    yysize *= wm->char_y;

    //Draw box:
    for( cyy = yy; cyy < yy + yysize; cyy += wm->char_y, ch_y++ )
    {
	for( cxx = xx; cxx < xx + xxsize; cxx += wm->char_x )
	{
	    if( pnt < t_size && wm->t_screen[ pnt ] == win_num )
	    {
		//Draw small box:
		sx = x; sy = y;
		sxsize = x_size;
		sysize = y_size;
		if( x < cxx ) { sx = cxx; sxsize -= cxx - x; }
		if( x >= cxx + wm->char_x ) continue;
		if( y < cyy ) { sy = cyy; sysize -= cyy - y; }
		if( y >= cyy + wm->char_y ) continue;
		if( sx + sxsize > cxx + wm->char_x )  sxsize -= sxsize + sx - ( cxx + wm->char_x );
		if( sy + sysize > cyy + wm->char_y )  sysize -= sysize + sy - ( cyy + wm->char_y );

		wm->screen_changes[ ch_y ] = 1;

		s_pnt = ( sy * wm->pscreen_x ) + sx; c1 = sx + sy; c2 = sx + sy;
		sy2 = sy;
		int temp_sx2;
		int temp_s_pnt;
	        for( cy = 0; cy < sysize; cy++ )
	        {
		    sx2 = sx;
		    color = box_color;
		    if( sy2 == y + y_size - 1 ) color = shade_color;
		    else
		    if( sy2 == y ) color = light_color;
		    temp_sx2 = sx2;
		    temp_s_pnt = s_pnt;
		    for( cx = 0; cx < sxsize; cx++ )
		    {
			/*
			if( sx2 == x ) color = light_color;
			if( sy2 == y ) color = light_color;
			if( sx2 == x + x_size - 1 ) color = shade_color;
			if( sy2 == y + y_size - 1 ) color = shade_color;
			*/
#ifdef DRAW_BACKGROUND
			if( (c2&1) && background )
			    wm->g_screen[ s_pnt ] = background[ s_pnt ];
			else
#endif
			    wm->g_screen[ s_pnt ] = (COLOR)color;
			s_pnt++; c2++;
			sx2++;
		    }
		    if( sx2 == x + x_size ) wm->g_screen[ s_pnt - 1 ] = shade_color;
		    else
		    if( temp_sx2 == x ) wm->g_screen[ temp_s_pnt ] = light_color;
		    s_pnt += wm->pscreen_x - sxsize;
		    c1++;
		    c2 = c1 & 1;
		    sy2++;
	        }
	    }
	    pnt++;
	}
	pnt += wm->screen_x - xxsize2;
    }
}

void draw_box( long win_num, long x, long y, long x_size, long y_size,
               long color, long chr, window_manager *wm )
{
    long wx, wy;
    long cx, cy;
    long pnt;

    window *win = wm->windows[ win_num ];
    wx = win->real_x;
    wy = win->real_y;

    x += wx; y += wy;

    //Screen bounds control:
    if( x >= wm->screen_x ) return;
    if( x + x_size > wm->screen_x ) x_size -= ( x_size - ( wm->screen_x - x ) );
    if( x < 0 )
	if( x + x_size <= 0 ) return;
	    else { x_size += x; x = 0; }
    if( y >= wm->screen_y ) return;
    if( y + y_size > wm->screen_y ) y_size -= ( y_size - ( wm->screen_y - y ) );
    if( y < 0 )
	if( y + y_size <= 0 ) return;
	    else { y_size += y; y = 0; }

    pnt = ( y * wm->screen_x ) + x;

    for( cy = y; cy < y + y_size; cy++ )
    {
	for( cx = x; cx < x + x_size; cx++ )
	{
	    if( wm->t_screen[ pnt ] == win_num )
	        draw_char( cx * wm->char_x, cy * wm->char_y, chr, color, wm );
	    pnt++;
	}
	pnt += wm->screen_x - x_size;
    }
}

void draw_touch_box( long back_win, long x, long y, long x_size, long y_size,
                     long fore_win, window_manager *wm )
{
    long wx, wy;
    long cx, cy;
    long pnt;

    window *win = wm->windows[ back_win ];
    wx = win->real_x;
    wy = win->real_y;

    x += wx; y += wy;
    x += win->x_offset;
    y += win->y_offset;

    //Screen bounds control:
    if( x >= wm->screen_x ) return;
    if( x + x_size > wm->screen_x ) x_size -= ( x_size - ( wm->screen_x - x ) );
    if( x < 0 )
	if( x + x_size <= 0 ) return;
	    else { x_size += x; x = 0; }
    if( y >= wm->screen_y ) return;
    if( y + y_size > wm->screen_y ) y_size -= ( y_size - ( wm->screen_y - y ) );
    if( y < 0 )
	if( y + y_size <= 0 ) return;
	    else { y_size += y; y = 0; }

    pnt = ( y * wm->screen_x ) + x;

    for( cy = y; cy < y + y_size; cy++ )
    {
	for( cx = x; cx < x + x_size; cx++ )
	{
	    if( wm->t_screen[ pnt ] == back_win )
	    {
	    	wm->t_screen[ pnt ] = (unsigned short)fore_win;
		wm->a_matrix[ pnt ] = 0;
		wm->g_matrix[ pnt ] = 0;
	    }
	    pnt++;
	}
	pnt += wm->screen_x - x_size;
    }
}

void matrix_draw_string( long win_num, long x, long y, uchar attr, char* str, window_manager *wm )
{
    long a = 0;
    long pnt;    //touch screen pointer

    window *win = wm->windows[ win_num ];
    long wx = win->real_x;
    long wy = win->real_y;

    x += wx; y += wy;
    pnt = ( y * wm->screen_x ) + x;

    //Screen bounds control:
    if( y < 0 || y >= wm->screen_y ) return;

    for(;;)
    {
	if( x >= wm->screen_x ) break; //out of screen
	if( str[a] == 0 ) break;
	if( pnt >= 0 )
	{
	    if( x >= 0 )
		if( wm->t_screen[ pnt ] == win_num )
		{
		    wm->t_matrix[ pnt ] = (uchar)str[ a ];
		    wm->a_matrix[ pnt ] = attr;
		    wm->w_matrix[ pnt ] = (uint16)win_num;
		}
	}
	pnt++;
	x++;
	a++;
    }
    wm->matrix_is_empty = 0;

}

void matrix_draw_box( long win_num, long x, long y, long x_size, long y_size,
                      long color, window_manager *wm )
{
    long cx, cy;
    long pnt;

    window *win = wm->windows[ win_num ];
    long wx = win->real_x;
    long wy = win->real_y;

    x += wx; y += wy;

    //Screen bounds control:
    if( x >= wm->screen_x ) return;
    if( x + x_size > wm->screen_x ) x_size -= ( x_size - ( wm->screen_x - x ) );
    if( x < 0 )
	if( x + x_size <= 0 ) return;
	    else { x_size += x; x = 0; }
    if( y >= wm->screen_y ) return;
    if( y + y_size > wm->screen_y ) y_size -= ( y_size - ( wm->screen_y - y ) );
    if( y < 0 )
	if( y + y_size <= 0 ) return;
	    else { y_size += y; y = 0; }

    pnt = ( y * wm->screen_x ) + x;

    for( cy = 0; cy < y_size; cy++ )
    {
	for( cx = 0; cx < x_size; cx++ )
	{
	    if( wm->t_screen[ pnt ] == win_num )
	    {
	        wm->g_matrix[ pnt ] = (COLOR)color;
	        wm->t_matrix[ pnt ] = 0;
	        wm->a_matrix[ pnt ] = 0;
	        wm->w_matrix[ pnt ] = (uint16)win_num;
	    }
	    pnt++;
	}
	pnt += wm->screen_x - x_size;
    }
    wm->matrix_is_empty = 0;
}

void matrix_clear_box( long win_num, long x, long y, long x_size, long y_size, window_manager *wm )
{
    long cx, cy;
    long pnt;

    window *win = wm->windows[ win_num ];
    long wx = win->real_x;
    long wy = win->real_y;

    x += wx; y += wy;

    //Screen bounds control:
    if( x >= wm->screen_x ) return;
    if( x + x_size > wm->screen_x ) x_size -= ( x_size - ( wm->screen_x - x ) );
    if( x < 0 )
	if( x + x_size <= 0 ) return;
	    else { x_size += x; x = 0; }
    if( y >= wm->screen_y ) return;
    if( y + y_size > wm->screen_y ) y_size -= ( y_size - ( wm->screen_y - y ) );
    if( y < 0 )
	if( y + y_size <= 0 ) return;
	    else { y_size += y; y = 0; }

    pnt = ( y * wm->screen_x ) + x;

    for( cy = 0; cy < y_size; cy++ )
    {
	for( cx = 0; cx < x_size; cx++ )
	{
	    if( wm->t_screen[ pnt ] == win_num )
	    {
	        wm->g_matrix[ pnt ] = 0;
	        wm->t_matrix[ pnt ] = 0;
	        wm->a_matrix[ pnt ] = 0;
	        wm->w_matrix[ pnt ] = 0;
	    }
	    pnt++;
	}
	pnt += wm->screen_x - x_size;
    }
}

void matrix_draw( window_manager *wm )
{
	long x,y;
    if( wm->matrix_is_empty ) return;

    long m_pnt = 0; //matrix pointer
    long s_pnt = 0; //screen pointer
    long add_part = ( wm->pscreen_x * ( wm->char_y - 1 ) ) + ( wm->pscreen_x - (wm->screen_x*wm->char_x) );
    long changed;

    for( y = 0; y < wm->screen_y; y++ )
    {
	changed = 0;
	for( x = 0; x < wm->screen_x; x++ )
	{
	    if( wm->g_matrix[ m_pnt ] && wm->w_matrix[ m_pnt ] == wm->t_screen[ m_pnt ] )
	    {
		changed = 1;
		if( wm->t_matrix[ m_pnt ] )
		{ //Draw char:
		    #ifdef TEXTMODE
			wm->g_screen[ m_pnt ] = wm->g_matrix[ m_pnt ];
			draw_char( x, y, wm->t_matrix[ m_pnt ], 0, wm );
		    #else
			fast_draw_char( s_pnt, wm->t_matrix[ m_pnt ], wm->g_matrix[ m_pnt ], wm->a_matrix[ m_pnt ], wm );
		    #endif
		}
		else
		{ //Draw box:
		    #ifdef TEXTMODE
			wm->g_screen[ m_pnt ] = wm->g_matrix[ m_pnt ];
		    #else
			fast_draw_char( s_pnt, ' ', wm->g_matrix[ m_pnt ], 0, wm );
		    #endif
		}
	    }
	    wm->g_matrix[ m_pnt ] = 0;
	    wm->t_matrix[ m_pnt ] = 0;
	    wm->a_matrix[ m_pnt ] = 0;
	    m_pnt++;
	    s_pnt += wm->char_x;
	}
	if( changed ) wm->screen_changes[ y ] = 1;
	s_pnt += add_part;
    }
    wm->matrix_is_empty = 1;
}

void draw_window_box( long win_num, window_manager *wm )
{
    window *win = wm->windows[win_num];

    if( win->draw_box )
        pdraw_box( win_num, 0, 0,
	           win->x_size * wm->char_x, win->y_size * wm->char_y,
		   win->color, wm );
}

void draw_window_touch_area( long win_num, window_manager *wm )
{
    long cx, cy;
    long x, y;
    long x_size, y_size;
    window *win = wm->windows[ win_num ];
    window *pwin = wm->windows[ win->parent_win ];

    x = win->real_x;
    y = win->real_y;
    x_size = win->x_size;
    y_size = win->y_size;

    //Screen bounds control:
    long xb = 0;
    long yb = 0;
    long xb2 = wm->screen_x;
    long yb2 = wm->screen_y;
    if( x >= xb2 ) return;
    if( x + x_size > xb2 ) x_size -= ( x_size - ( xb2 - x ) );
    if( x < xb )
        if( x + x_size <= xb ) return;
	    else { x_size -= xb - x; x = xb; }
    if( y >= yb2 ) return;
    if( y + y_size > yb2 ) y_size -= ( y_size - ( yb2 - y ) );
    if( y < yb )
        if( y + y_size <= yb ) return;
	    else { y_size += yb - y; y = yb; }

    //Parent bounds control:
    if( win->parent_win != win_num )
    {
next_iter:
	xb = pwin->real_x;
	yb = pwin->real_y;
	xb2 = pwin->real_x + pwin->x_size;
	yb2 = pwin->real_y + pwin->y_size;
	if( x >= xb2 ) return;
	if( x + x_size > xb2 ) x_size -= ( x_size - ( xb2 - x ) );
	if( x < xb )
	    if( x + x_size <= xb ) return;
		else { x_size -= xb - x; x = xb; }
	if( y >= yb2 ) return;
	if( y + y_size > yb2 ) y_size -= ( y_size - ( yb2 - y ) );
	if( y < yb )
	    if( y + y_size <= yb ) return;
		else { y_size += yb - y; y = yb; }
	if( pwin->parent_win != win->parent_win )
	{
	    win = pwin;
	    pwin = wm->windows[ pwin->parent_win ];
	    goto next_iter;
	}
    }

    long pnt;

    for( cy = y; cy < y + y_size; cy++ )
    {
	for( cx = x; cx < x + x_size; cx++ )
	{
	    pnt = ( cy * wm->screen_x ) + cx;
	    wm->t_screen[ pnt ] = (unsigned short)win_num;
	    wm->g_matrix[ pnt ] = 0;
	}
    }
}

void redraw_screen(window_manager *wm)
{
    long y;
    for( y = 0; y < wm->screen_y; y++ ) wm->screen_changes[y] = 1;
    draw_screen(wm);
}

long create_window( char *name,
                    long x, long y, long x_size, long y_size, long color, long draw_box,
                    long parent,
		    long (*win_handler)(event*,window_manager*),
		    window_manager *wm )
{
    long win_num;
    window *win;
    event evt; //temp event

    for(win_num=0;win_num<MAX_WINDOWS;win_num++) {
	if(wm->windows[win_num] == 0) break;
    }
    win = wm->windows[win_num] = (window*)mem_new( 0, sizeof(window), name, win_num );

    win->name = name;
    win->x = x;
    win->y = y;
    win->x_size = x_size;
    win->y_size = y_size;
    win->xs = ""; win->ys = ""; win->xs2 = ""; win->ys2 = "";
    win->x_offset = 0;
    win->y_offset = 0;
    win->color = color;
    win->draw_box = draw_box;
    win->ghost = 0;
    win->parent_win = parent;
    win->win_handler = (long (*)(void*,void*))win_handler;
    win->visible = 0;
    win->childs = 0;
    win->data = 0;

    if( parent != win_num ) create_child( parent, win_num, wm );

    win->scroll_flags = 0;
    calculate_real_window_position( win_num, wm );
    win->scroll_flags = 0;

    evt.mode = MODE_WINDOW;
    evt.x = 0;
    evt.y = 0;
    evt.button = 0;
    evt.event_type = EVT_AFTERCREATE;
    evt.event_win = win_num;

    //Process EVT_AFTERCREATE:
    win->win_handler( &evt, wm );

    return win_num;
}

void close_window( long win_num, window_manager *wm )
{
    window *win = wm->windows[win_num];
    window *parent;
    long a, b, c;
    event evt; //temp event

    evt.mode = MODE_WINDOW;
    evt.x = 0;
    evt.y = 0;
    evt.button = 0;
    evt.event_type = EVT_BEFORECLOSE;
    evt.event_win = win_num;

    if( win )
    {
	//Process EVT_BEFORECLOSE:
	if( win->win_handler ) win->win_handler( &evt, wm );

	//Search for parents:
	for( a = 0; a < MAX_WINDOWS; a++ )
	{
	    parent = wm->windows[ a ];
	    if( parent )
	    {
		for( b = 0; b < parent->childs; b++ )
		{
		    if( parent->child[ b ] == win_num )
		    {
			parent->childs --;
			for( c = b; c < parent->childs; c++ )
			{
			    parent->child[ c ] = parent->child[ c + 1 ];
			}
		    }
		}
	    }
	}
	//Closing:
	if(win->childs)
	{
	    for(a=0;a<win->childs;a++) close_window(win->child[a],wm); //close all child windows
	    mem_free(win->child);
	}
	mem_free(win); //close window
	wm->windows[win_num] = 0;
    }
}

void create_child(long parent, long child_win, window_manager* wm)
{
    window *win = wm->windows[parent];
    if(win->childs == 0) { //create new memory block for childs:
	win->child = (long*)mem_new( 0, 4 * 20, "childs", parent ); //Maximum 20 childs at the moment
	win->childs = 1;
	win->child[0] = child_win;
    } else { //add child to the existent memory block:
	win->childs++;
	if( win->childs >= (long)( mem_get_size( win->child ) / sizeof( long ) ) )
	    win->child = (long*)mem_resize( win->child, mem_get_size( win->child ) + 20 * sizeof( long ) );
	win->child[ win->childs-1 ] = child_win;
    }
}

void move_window( long win_num, long x, long y, window_manager *wm )
{
    window *win = wm->windows[ win_num ];

    win->x = x;
    win->y = y;

    calculate_real_window_position( win_num, wm );
}

void calculate_real_window_position( long win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    window *p = wm->windows[ win->parent_win ];

    long x = win->x;
    long y = win->y;
    if( p && !(win->scroll_flags & SF_STATICWIN) )
    {
	x += p->x_offset;
	y += p->y_offset;
    }
    long x_size = win->x_size;
    long y_size = win->y_size;

    if( x < 0 )
    {
	if( p ) p->scroll_flags |= SF_LEFT;
    }
    if( y < 0 )
    {
	if( p ) p->scroll_flags |= SF_UP;
    }

    if( win->parent_win != win_num )
    {
	long parent_x = p->real_x;
	long parent_y = p->real_y;
	win->real_x = parent_x + x;
	win->real_y = parent_y + y;
	if( x + x_size > p->x_size )
	    p->scroll_flags |= SF_RIGHT;
	if( y + y_size > p->y_size )
	    p->scroll_flags |= SF_DOWN;
    }
    else
    {
	win->real_x = x;
	win->real_y = y;
    }
}

void calculate_real_window_position_with_childs( long win_num, window_manager *wm )
{
	int c;
    window *win = wm->windows[ win_num ];
    calculate_real_window_position( win_num, wm );
    if( win->childs )
    {
	for( c = 0; c < win->childs; c++ )
	{
	    calculate_real_window_position_with_childs( win->child[ c ], wm );
	}
    }
}

void set_window_string_controls( long win_num, char *xs, char *ys, char *xs2, char *ys2, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    win->xs = xs;
    win->ys = ys;
    win->xs2 = xs2;
    win->ys2 = ys2;
}

void push_button(long x, long y, long button, long pressure, long type, window_manager *wm)
{
    if( !wm->wm_initialized ) return;
    long window_num = wm->mouse_win;
    long wx,wy; //window x/y

    if( x < 0 ) x = 0;
    if( y < 0 ) y = 0;

    if( x < wm->pscreen_x && y < wm->pscreen_y ) {

	wm->current_mouse_x = x;
	wm->current_mouse_y = y;

	if( button & 7 && type == 0 )
	{ //If mouse click:
	    //determine window number:
	    window_num = wm->t_screen[ ((y/wm->char_y) * wm->screen_x) + (x/wm->char_x) ];
	}

        window *win = wm->windows[ window_num ];
	window *gwin = wm->windows[ wm->ghost_win ];

	switch(type) {
	    case 0: //BUTTON DOWN:
		    wx = win->real_x;
		    wy = win->real_y;
		    wx *= wm->char_x;
		    wy *= wm->char_y;
            	    send_event( window_num, EVT_BUTTONDOWN, x - wx, y - wy,
			        button, pressure, MODE_WINDOW, wm );
		    if( !win->ghost )
		    {
			wm->mouse_button_pressed = (char)button & 7;
			if( wm->mouse_win != window_num )
			    send_event( wm->mouse_win, EVT_UNFOCUS, x - wx, y - wy,
				button, pressure, MODE_WINDOW, wm );
			wm->mouse_win = window_num;
			if( button & 7 ) wm->ghost_win = 0;
		    }
		    else
		    {
			wm->ghost_win = window_num;
		    }
		    break;
	    case 1: //BUTTON UP:
		    if( !wm->ghost_win )
		    {
			wx = win->real_x;
			wy = win->real_y;
		    }
		    else
		    {
			wx = gwin->real_x;
			wy = gwin->real_y;
		    }
		    wx *= wm->char_x;
		    wy *= wm->char_y;
		    if( !wm->ghost_win )
            		send_event( window_num, EVT_BUTTONUP, x - wx, y - wy,
			            button, pressure, MODE_WINDOW, wm );
		    else
            		send_event( wm->ghost_win, EVT_BUTTONUP, x - wx, y - wy,
			            button, pressure, MODE_WINDOW, wm );
		    break;
	    case 2: //MOUSE MOVE:
		    if( !wm->ghost_win )
		    {
			wx = win->real_x;
			wy = win->real_y;
		    }
		    else
		    {
			wx = gwin->real_x;
			wy = gwin->real_y;
		    }
		    wx *= wm->char_x;
		    wy *= wm->char_y;
		    if( !wm->ghost_win )
            		send_event( window_num, EVT_MOUSEMOVE, x - wx, y - wy,
		    	            button, pressure, MODE_WINDOW, wm);
		    else
            		send_event( wm->ghost_win, EVT_MOUSEMOVE, x - wx, y - wy,
		    	            button, pressure, MODE_WINDOW, wm);
		    break;
	}

    }
}


//###################################
//### DEVICE DEPENDENT FUNCTIONS: ###
//###################################

void create_active_screen_part(window_manager *wm)
{
#ifndef NONPALM
    //PalmOS devices:
    uint16 bmp_err;
    active_part_low = BmpCreate( wm->pscreen_x, wm->char_y, COLORBITS, 0, &bmp_err );
    wm->screen_part = (COLORPTR)BmpGetBits( active_part_low );
    #ifndef PALMLOWRES
	active_part = BmpCreateBitmapV3( active_part_low, kDensityDouble, wm->screen_part, 0 );
    #endif
#else
    //Other devices:
    //wm->screen_part = (COLORPTR)mem_new( 0, wm->screen_x * wm->char_x * wm->char_y * COLORLEN, "one screen part", 0 );
#endif
}

void close_active_screen_part(window_manager *wm)
{
#ifndef NONPALM
    //PalmOS devices:
    BmpDelete( active_part_low );
    #ifndef PALMLOWRES
	BmpDelete( (BitmapType*)active_part );
    #endif
#else
    //Other devices:
    //mem_free( wm->screen_part );
#endif
}

void draw_screen(window_manager *wm)
{
    matrix_draw( wm );

    long y;
    long changed = 0;
    for( y = 0; y < wm->screen_y; y++ )
    {
	if( wm->screen_changes[ y ] ) { draw_screen_part( y, wm ); changed = 1; }
	wm->screen_changes[ y ] = 0;
    }

#ifdef OPENGL
//############################
//############################
#ifndef DEMOENGINE
    /* clear color and depth buffers */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();
    //When demoengine is active, color clears by demoengine.cpp
#endif
//############################
//############################
    /* draw polygon with the screen */
    glBindTexture( GL_TEXTURE_2D, 1 );
    if(changed)
#ifdef COLOR8BITS
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 3,
                 wm->pscreen_x, wm->pscreen_y,
                 0,
                 GL_COLOR_INDEX,
                 GL_UNSIGNED_BYTE,
                 wm->g_screen);
#endif
#ifdef COLOR16BITS
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 3,
                 wm->pscreen_x, wm->pscreen_y,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_SHORT,
                 wm->g_screen);
#endif
#ifdef COLOR32BITS
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 4,
                 wm->pscreen_x, wm->pscreen_y,
                 0,
                 GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE,
                 wm->g_screen);
#endif
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_POLYGON);
//############################
//############################
#ifdef DEMOENGINE
    glColor4f( 1.0F, 1.0F, 1.0F, screen_transparency );
#else
    glColor3f( 1.0F, 1.0F, 1.0F );
#endif
//############################
//############################

    glTexCoord2f(0,0);glVertex3f( -1, 1, 0 );
    glTexCoord2f(1,0);glVertex3f( 1, 1, 0 );
    glTexCoord2f(1,1);glVertex3f( 1, -1, 0 );
    glTexCoord2f(0,1);glVertex3f( -1, -1, 0 );
    glEnd();
    glEnable(GL_DEPTH_TEST);
#ifdef WIN
    SwapBuffers((HDC)wm->hdc);
#endif
#ifdef LINUX
    XSync( wm->dpy, 0 );
    glFlush();
    if( wm->doubleBuffer ) glXSwapBuffers( wm->dpy, wm->win );
#endif //LINUX
#endif //OPENGL

#ifdef GDI
    BITMAPINFO *bi = (BITMAPINFO*)wm->gdi_bitmap_info;
    bi->bmiHeader.biWidth = wm->pscreen_x;
    bi->bmiHeader.biHeight = -wm->pscreen_y;
    if(changed)
    SetDIBitsToDevice( (HDC)wm->hdc,
			0,              // Destination top left hand corner X Position
                        0,              // Destination top left hand corner Y Position
                        wm->pscreen_x,  // Destinations Width
                        wm->pscreen_y,  // Desitnations height
                        0,              // Source top left hand corner's X Position
                        0,              // Source top left hand corner's Y Position
                        0,
                        wm->pscreen_y,
                        wm->g_screen,     // Source's data
                        (BITMAPINFO*)wm->gdi_bitmap_info, // Bitmap Info
                        DIB_RGB_COLORS );
#endif

#ifdef DIRECTX
	if(changed)
	{
		//HRESULT ddrval;
		/*DDSURFACEDESC sd;
		long a;
		memset (&sd, 0, sizeof (DDSURFACEDESC));
		sd.dwSize = sizeof (sd);
		wm->lpDDSBack->Lock( 0, &sd, DDLOCK_SURFACEMEMORYPTR  | DDLOCK_WAIT, 0 );
		long lPitch = sd.lPitch / COLORLEN;
		long src_ptr = 0;
		long dest_ptr = 0;
		long line_size = wm->pscreen_x * COLORLEN;
		for( a = 0; a < wm->pscreen_y; a++ )
		{
			memcpy( (COLORPTR)sd.lpSurface + dest_ptr,
				     wm->g_screen + src_ptr,
					 line_size );
			dest_ptr += lPitch;
			src_ptr += wm->pscreen_x;
		}
		wm->lpDDSBack->Unlock( sd.lpSurface );
		ddrval = wm->lpDDSPrimary->Flip( 0, DDFLIP_WAIT );
		if(ddrval == DDERR_SURFACELOST)
		{
			ddrval = wm->lpDDSPrimary->Restore();
		}*/
	}
#endif

#ifdef X11
    XSync( wm->dpy, 0 );
    //XFlush( wm->dpy );
#endif

#ifdef LINUX
#ifdef TEXTMODE

#endif
#endif
}

void fast_draw_char( long ptr, long c, long back_color, uchar attr, window_manager *wm )
{
#ifndef TEXTMODE
    long p;           //screen pointer
    long cx,cy;       //current x,y
    long cur_line;    //current piece of symbol
    long c1=0,c2=0;   //horisontal and vertical counters for pseudo-transparent background
    COLOR *background;//background buffer;
    COLOR *user_font;
	COLOR color;

#ifdef NATIVEARM
#ifdef COLOR16BITS
    back_color = BSwap16( back_color );
#endif
#ifdef COLOR32BITS
    back_color = BSwap32( back_color );
#endif
#endif
    background = wm->b_screen;

    p = ptr;

	//0000
	color = CLEARCOLOR;
	//dose not look right on the palm. comes out as purple
#ifndef NONPALM
	//do nothing wwww
#else
	if( attr & ATR_SHADOW ) color = wm->font_shadow_color;
#endif

    if( wm->user_font_mode == 0 )
    {
	c <<= 3; //c += 7;
	for( cy = 0; cy < 8; cy ++ )
	{
    	    cur_line = font[c];
	    for( cx = 0; cx < 8; cx ++ )
	    {
#ifdef DRAW_BACKGROUND
		if( ( cur_line & 128 ) || ( attr & ATR_BOLD && ( cur_line & 64 ) ) )
		{ //foreground pixel:
		    wm->g_screen[p] = color;
		}
		else
		{
		    if( c2&1 && background )
			wm->g_screen[p] = background[p];
		    else
		        wm->g_screen[p] = (COLOR)back_color;
		}
#else
		if( ( cur_line & 128 ) || ( attr & ATR_BOLD && ( cur_line & 64 ) ) )
		{ //foreground pixel:
		    wm->g_screen[p] = color;
		}
		else
		{
		    wm->g_screen[p] = (COLOR)back_color;
		}
#endif
		cur_line <<= 1;
		p++; c2++;
	    }
	    c++;
	    c1++;
	    c2 = c1 & 1;
	    p += wm->pscreen_x - wm->char_x;
	}
    }
    else
    {
	user_font = wm->user_font;
	c *= (wm->char_x * wm->char_y);
	for( cy = 0; cy < wm->char_y ; cy++ )
	{
	    for( cx = 0; cx < wm->char_x; cx++ )
	    {
#ifdef DRAW_BACKGROUND
		if( user_font[ c ] || ( attr & ATR_BOLD && cx && user_font[ c - 1 ] ) )
		{ //foreground pixel:
		    wm->g_screen[ p ] = color;//user_font[ c ];
		}
		else
		{ //background pixel:
		    if( c2&1 && background )
			wm->g_screen[ p ] = background[ p ];
		    else
			wm->g_screen[ p ] = (COLOR)back_color;
		}
#else
		if( user_font[ c ] || ( attr & ATR_BOLD && cx && user_font[ c - 1 ] ) )
		{ //foreground pixel:
		    wm->g_screen[ p ] = color;//user_font[ c ];
		}
		else
		{
		    wm->g_screen[ p ] = (COLOR)back_color;
		}
#endif
		p++; c2++;
		c++;
	    }
	    c1++;
	    c2 = c1 & 1;
	    p += wm->pscreen_x - wm->char_x;
	}
    }
#endif
}

void draw_char( long x, long y, long c, long attr, window_manager *wm )
{
#ifndef TEXTMODE
    long p;           //screen pointer
    long cx,cy;       //current x,y
    long cur_line;    //current piece of symbol
    long c1=0,c2=0;   //horisontal and vertical counters for pseudo-transparent background
    COLOR *background;//background buffer;
    COLOR *user_font;

    if( x > wm->pscreen_x - wm->char_x ) return;
    if( y > wm->pscreen_y - wm->char_y ) return;

    background = wm->b_screen;

    p = (y * wm->pscreen_x) + x;
    wm->screen_changes[p / wm->part_size] = 1;

    if( wm->user_font_mode == 0 )
    {
	c <<= 3; //c += 7;
	for( cy = 0; cy < 8; cy ++ )
	{
    	    cur_line = font[c];
	    for( cx = 0; cx < 8; cx++ )
	    {
		if( cur_line & 128 ) { //foreground pixel:
		    wm->g_screen[p] = CLEARCOLOR;
		}
		cur_line <<= 1;
		p++; c2++;
	    }
	    c++;
	    c1++;
	    c2 = c1 & 1;
	    p += wm->pscreen_x - wm->char_x;
	}
    }
    else
    {
	user_font = wm->user_font;
	c *= (wm->char_x * wm->char_y);
	for( cy = 0; cy < wm->char_y ; cy++ )
	{
	    for( cx = 0; cx < wm->char_x; cx++ )
	    {
		if( user_font[ c ] )
		{ //foreground pixel:
		    wm->g_screen[ p ] = user_font[ c ];
		}
		p++; c2++;
		c++;
	    }
	    c1++;
	    c2 = c1 & 1;
	    p += wm->pscreen_x - wm->char_x;
	}
    }
#else
    //For Text Mode devices (Linux console):
    long f,b;
    long f_result = 0, b_result = 0;

    if( x > wm->pscreen_x - wm->char_x ) return;
    if( y > wm->pscreen_y - wm->char_y ) return;

    x /= wm->char_x;
    y /= wm->char_y;
    //get color:
    f = 0xFFFF;
    b = wm->g_screen[ ( y * wm->screen_x ) + x ];
#ifdef COLOR8BITS
    if(f&4) f_result |= 1;
    if(f&32) f_result |= 2;
    if(f&128) f_result |= 4;
    if(b&4) b_result |= 1;
    if(b&32) b_result |= 2;
    if(b&128) b_result |= 4;
#endif
#ifdef COLOR16BITS
    if(f&16) f_result |= 4;
    if(f&1024) f_result |= 2;
    if(f&32768) f_result |= 1;
    if(b&16) b_result |= 4;
    if(b&1024) b_result |= 2;
    if(b&32768) b_result |= 1;
#endif
#ifdef COLOR32BITS
    if(f&0x80) f_result |= 4;
    if(f&0x0080) f_result |= 2;
    if(f&0x000080) f_result |= 1;
    if(b&0x80) b_result |= 4;
    if(b&0x0080) b_result |= 2;
    if(b&0x000080) b_result |= 1;
#endif
    //save char and color:
    wm->text_screen[ ( y * wm->screen_x ) + x ] = c;
    wm->text_screen[ ( y * wm->screen_x ) + x ] |= f_result << 8;
    wm->text_screen[ ( y * wm->screen_x ) + x ] |= b_result << 16;
    //clear graphics:
    wm->g_screen[ ( y * wm->screen_x ) + x ] = 0;
    //set changes:
    wm->screen_changes[ y ] = 1;
#endif
}

void draw_screen_part(long part, window_manager *wm)
{
#ifndef NONPALM
    //PalmOS devices:
#ifdef COLOR8BITS
    MemMove(wm->screen_part, wm->g_screen + (part * wm->part_size), wm->part_size);
#endif
#ifdef COLOR16BITS
    MemMove(wm->screen_part, wm->g_screen + (part * wm->part_size), wm->part_size<<1);
#endif
#ifdef COLOR32BITS
    MemMove(wm->screen_part, wm->g_screen + (part * wm->part_size), wm->part_size<<2);
#endif
    #ifdef PALMLOWRES
	WinDrawBitmap( (BitmapType*)active_part_low, 0, part * wm->char_y );
    #else
	WinDrawBitmap( (BitmapType*)active_part, 0, (part * wm->char_y) >> 1 );
    #endif
#else

    //Other devices:
#ifdef TEXTMODE
    //Linux console:
    long ptr, c, f_color, b_color, a;
    long r,g,b,gray;
    ptr = part * wm->screen_x;
    //Set cursor position:
    printf( "\033[%d;%dH", part + 1, 1 );
    //Draw text line:
    printf( "\033[0m" );
    printf( "\033[1m" );
    for( a = 0; a < wm->screen_x; a++ )
    {
	c = wm->text_screen[ ptr ] & 0x0000FF;
	if( c == 0 ) c = ' ';
	f_color = ( wm->text_screen[ ptr ] & 0x00FF00 ) >> 8;
	b_color = ( wm->text_screen[ ptr ] & 0xFF0000 ) >> 16;
	//set color:
	if( wm->g_screen[ ptr ] )
	{
	    c = wm->g_screen[ ptr ];
	    f_color = 0;
#ifdef COLOR8BITS
	    if(c&4) f_color |= 1;
	    if(c&32) f_color |= 2;
	    if(c&128) f_color |= 4;
#endif
#ifdef COLOR16BITS
	    if(c&16) f_color |= 4;
	    if(c&1024) f_color |= 2;
	    if(c&32768) f_color |= 1;
#endif
#ifdef COLOR32BITS
	    if(c&0x80) f_color |= 4;
	    if(c&0x0080) f_color |= 2;
	    if(c&0x000080) f_color |= 1;
#endif
	    printf( "\033[%dm\033[%dm", 30+f_color, 40+f_color );
	    //Get gray component:
	    r = ( (c<<5)&224 ) | 31;
	    g = ( (c<<2)&224 ) | 31;
	    b = (c&192)|63;
	    gray = ( r + g + b ) / 3;
	    c = gray_scale[ gray>>5 ];
	}
	else
	{
	    printf( "\033[%dm\033[%dm", 30+f_color, 40+b_color );
	}
	//draw symbol:
	printf( "%c", c );
	ptr++;
    }
    printf("\n");
#endif
#endif

#ifdef DIRECTX
    //HRESULT ddrval;
    DDSURFACEDESC sd;
    long a;
    memset (&sd, 0, sizeof (DDSURFACEDESC));
    sd.dwSize = sizeof (sd);
    wm->lpDDSPrimary->Lock( 0, &sd, DDLOCK_SURFACEMEMORYPTR  | DDLOCK_WAIT, 0 );
    long lPitch = (sd.lPitch / COLORLEN);
    long src_ptr = part * wm->char_y * wm->pscreen_x;
    long dest_ptr = part * wm->char_y * (sd.lPitch / COLORLEN);
    long line_x_size = wm->pscreen_x * COLORLEN;
    for( a = 0; a < wm->char_y; a++ )
    {
    	memcpy( (COLORPTR)sd.lpSurface + dest_ptr,
	        wm->g_screen + src_ptr,
		line_x_size );
	dest_ptr += lPitch;
	src_ptr += wm->pscreen_x;
    }
    wm->lpDDSPrimary->Unlock( sd.lpSurface );
#endif

#ifdef X11
    wm->win_img->height = wm->char_y;
    wm->win_img->width = wm->pscreen_x;
    wm->win_img->bytes_per_line = wm->pscreen_x * wm->win_img_depth;
    long bytes_per_pixel = wm->win_img_depth;
    long src_ptr = part * wm->char_y * wm->pscreen_x;
    uchar *dest = (uchar*)wm->win_img->data;
    long a;
    ulong r, g, b, res;
    COLOR p;
    if( wm->win_img->byte_order != MSBFirst && bytes_per_pixel == COLORLEN )
    {
	mem_copy( dest, wm->g_screen + src_ptr, wm->part_size * COLORLEN );
    }
    else
    for( a = 0; a < wm->part_size; a++ )
    {
	p = wm->g_screen[ src_ptr++ ];
	#ifdef COLOR8BITS
	r = p & 7; r <<= 5; r |= 31;
	g = p & 56; g <<= 2; g |= 31;
	b = p & 192; b |= 63;
	#endif
	#ifdef COLOR16BITS
	b = p & 31; b <<= 3;
	g = p & 2016; g >>= 3;
	r = p & 0xF800; r >>= 8;
	#endif
	#ifdef COLOR32BITS
	b = p & 255;
	g = (p>>8) & 255;
	r = (p>>16) & 255;
	#endif
	if( wm->win_img->byte_order == MSBFirst )
	{
	    switch( bytes_per_pixel )
	    {
		case 1:
		*(dest++) = (uchar)( (r>>5) + ((g>>5)<<3) + (b>>6)<<6 ); break;
		case 2:
		res = (b>>3) + ((g>>2)<<5) + ((r>>3)<<11);
		*(dest++) = (uchar)(res & 255); *(dest++) = (uchar)(res >> 8); break;
		case 3:
		*(dest++) = (uchar)r; *(dest++) = (uchar)g; *(dest++) = (uchar)b; break;
		case 4:
		*(dest++) = (uchar)r; *(dest++) = (uchar)g; *(dest++) = (uchar)b; *(dest++) = 255; break;
	    }
	}
	else
	{
	    switch( bytes_per_pixel )
	    {
		case 1:
		*(dest++) = (uchar)( (r>>5) + ((g>>5)<<3) + (b>>6)<<6 ); break;
		case 2:
		res = (b>>3) + ((g>>2)<<5) + ((r>>3)<<11);
		*(dest++) = (uchar)(res & 255); *(dest++) = (uchar)(res >> 8); break;
		case 3:
		*(dest++) = (uchar)b; *(dest++) = (uchar)g; *(dest++) = (uchar)r; break;
		case 4:
		*(dest++) = (uchar)b; *(dest++) = (uchar)g; *(dest++) = (uchar)r; *(dest++) = 255; break;
	    }
	}
    }
    if( XPutImage( wm->dpy, wm->win, wm->win_gc, wm->win_img, 0, 0, 0, part * wm->char_y, wm->pscreen_x, wm->char_y ) == BadMatch ) prints( "Bad match" );
#endif
}

void set_palette( window_manager* wm )
{
	short i;
#ifndef NONPALM
    //PalmOS devices:
    RGBColorType pal[256];
    ulong x,y,depth;
    long a;

    x = 320; y = 320; depth = COLORBITS;
    WinScreenMode(winScreenModeSet, &x, &y, &depth, 0);

#ifdef COLOR8BITS
    for(a=0;a<256;a++){
	pal[a].r = ( (a<<5)&224 ) | 31;
	pal[a].g = ( (a<<2)&224 ) | 31;
	pal[a].b = (a&192)|63;
	pal[a].index = a;
    }
    WinPalette(winPaletteSet, 0, 256, pal);
#endif
#endif

#ifdef OPENGL
    //OpenGL devices:
    unsigned short map[256];
    long a;
    unsigned short const_alpha = 0xFFFF;
    for(a=0; a<256; a++) { map[a] = ( (a<<5)&224 ) << 8; if(map[a]) map[a] |= 0x1FFF; }
    glPixelMapusv(GL_PIXEL_MAP_I_TO_R, 256, map);
    for(a=0; a<256; a++) { map[a] = ( (a<<2)&224 ) << 8; if(map[a]) map[a] |= 0x1FFF; }
    glPixelMapusv(GL_PIXEL_MAP_I_TO_G, 256, map);
    for(a=0; a<256; a++) { map[a] = ( (a&192)|32 ) << 8; if(map[a]) map[a] |= 0x3FFF; }
    glPixelMapusv(GL_PIXEL_MAP_I_TO_B, 256, map);
    glPixelMapusv(GL_PIXEL_MAP_I_TO_A, 1, &const_alpha);
#endif

#ifdef GDI
    int a;
    BITMAPINFO *bi = (BITMAPINFO*)wm->gdi_bitmap_info;
    memset( bi, 0, sizeof( wm->gdi_bitmap_info ) );
    bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi->bmiHeader.biWidth = wm->pscreen_x;
    bi->bmiHeader.biHeight = -wm->pscreen_y;
    bi->bmiHeader.biPlanes = 1;
    bi->bmiHeader.biBitCount = COLORBITS;
    bi->bmiHeader.biCompression = BI_RGB;

    //Set palette:
    for(a=0; a<256; a++)
    {
    	bi->bmiColors[a].rgbRed = (a<<5)&224;
	if( bi->bmiColors[a].rgbRed )
	    bi->bmiColors[a].rgbRed |= 0x1F;
		bi->bmiColors[a].rgbReserved = 0;
    }
    for(a=0; a<256; a++)
    {
	bi->bmiColors[a].rgbGreen = (a<<2)&224;
	if( bi->bmiColors[a].rgbGreen )
	    bi->bmiColors[a].rgbGreen |= 0x1F;
    }
    for(a=0; a<256; a++)
    {
	bi->bmiColors[a].rgbBlue = (a&192)|32;
	if( bi->bmiColors[a].rgbBlue )
	    bi->bmiColors[a].rgbBlue |= 0x3F;
    }
#endif

    //Set palette for all windows:
#ifdef TEXTMODE
    wm->colors[0]  = get_color( 255, 255, 255 );
    wm->colors[1]  = get_color( 255, 255, 255 );
    wm->colors[2]  = get_color( 255, 255, 255 );
    wm->colors[3]  = get_color( 0,   255, 255 );
    wm->colors[4]  = get_color( 0,   255, 255 );
    wm->colors[5]  = get_color( 0,   255, 255 );
    wm->colors[6]  = get_color( 0,   0,   255 );
    wm->colors[7]  = get_color( 0,   0,   255 );
    wm->colors[8]  = get_color( 0,   0,   255 );
    wm->colors[9]  = get_color( 64,  64,  64 );
    wm->colors[10] = get_color( 64,  64,  64 );
    wm->colors[11] = get_color( 64,  64,  64 );
    wm->colors[12] = get_color( 64,  64,  64 );
    wm->colors[13] = get_color( 32,  32,  32 );
    wm->colors[14] = get_color( 32,  32,  32 );
    wm->colors[15] = get_color( 32,  32,  32 );
#else
#if 0
    wm->colors[0]  = get_color( 255, 255, 255 );
    wm->colors[1]  = get_color( 250, 245, 235 );
    wm->colors[2]  = get_color( 245, 235, 215 );
    wm->colors[3]  = get_color( 240, 220, 200 );
    wm->colors[4]  = get_color( 230, 210, 190 );
    wm->colors[5]  = get_color( 215, 200, 170 );
    wm->colors[6]  = get_color( 200, 180, 160 );
    wm->colors[7]  = get_color( 190, 170, 150 );
    wm->colors[8]  = get_color( 180, 160, 140 );
    wm->colors[9]  = get_color( 170, 150, 120 );
    wm->colors[10] = get_color( 165, 145, 115 );
    wm->colors[11] = get_color( 160, 140, 110 );
    wm->colors[12] = get_color( 155, 135, 105 );
    wm->colors[13] = get_color( 150, 130, 100 );
    wm->colors[14] = get_color( 150, 130, 100 );
    wm->colors[15] = get_color( 150, 130, 100 );
#else
	wm->colors[0]  = get_color( 255, 255, 255 );
	wm->colors[1]  = get_color( 245, 250, 235 );
	wm->colors[2]  = get_color( 235, 245, 215 );
	wm->colors[3]  = get_color( 220, 240, 200 );
	wm->colors[4]  = get_color( 210, 230, 190 );
	wm->colors[5]  = get_color( 200, 215, 170 );
	wm->colors[6]  = get_color( 180, 200, 160 );
	wm->colors[7]  = get_color( 170, 190, 150 );
	wm->colors[8]  = get_color( 160, 180, 140 );
	wm->colors[9]  = get_color( 150, 170, 120 );
	wm->colors[10] = get_color( 145, 165, 115 );
	wm->colors[11] = get_color( 140, 160, 110 );
	wm->colors[12] = get_color( 135, 155, 105 );
	wm->colors[13] = get_color( 130, 150, 100 );
	wm->colors[14] = get_color( 130, 150, 100 );
	wm->colors[15] = get_color( 130, 150, 100 );
/*cyan
	wm->colors[0]  = get_color( 255, 255, 255 );
	wm->colors[1]  = get_color( 245, 250, 250 );
	wm->colors[2]  = get_color( 235, 245, 245 );
	wm->colors[3]  = get_color( 220, 240, 240 );
	wm->colors[4]  = get_color( 210, 230, 230 );
	wm->colors[5]  = get_color( 200, 215, 215 );
	wm->colors[6]  = get_color( 180, 200, 200 );
	wm->colors[7]  = get_color( 170, 190, 190 );
	wm->colors[8]  = get_color( 160, 180, 180 );
	wm->colors[9]  = get_color( 150, 170, 170 );
	wm->colors[10] = get_color( 145, 165, 165 );
	wm->colors[11] = get_color( 140, 160, 160 );
	wm->colors[12] = get_color( 135, 155, 155 );
	wm->colors[13] = get_color( 130, 150, 150 );
	wm->colors[14] = get_color( 130, 150, 150 );
	wm->colors[15] = get_color( 130, 150, 150 );
*/
#endif
	//0000
#if 0//def NONPALM
	for(i=0;i<16;i++)//		wm->colors[i] = get_color( time_seconds()%255, time_seconds()%255, time_seconds()%255 );//SysRandom(time_seconds())%255, SysRandom(time_seconds())%255, SysRandom(time_seconds())%255 );
		wm->colors[i] = get_color( rand()%200, rand(), rand()%250 );
#endif
	wm->highlight_color[0] = get_color( 255, 0, 0 );
	wm->highlight_color[1] = get_color( 90, 90, 255 );
	wm->psytable[0] = get_color( 150, 150, 255 );
	wm->font_shadow_color = get_color( 80, 80, 80 );
#endif
}

//###################################
//###################################
//###################################
