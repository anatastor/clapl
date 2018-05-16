/*
 * This file is part of clapl.
 *
 * clapl is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 or later
 * <http://gnu.org/license/gpl.html>
 *
 * clapl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MECHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License of more details.
 */

/**
 * @file ui.h
 * @author Torsten Lehmann
 * @date 2018-05-06
 * @brief file containing definitions of the user interface used by clapl
 */



#ifndef _UI_H_
#define _UI_H_

#include <curses.h>
#include <locale.h>
#include <string.h>
#include <sqlite3.h>

#include "enums.h"
#include "logger.h"
#include "cache.h"
#include "configparser.h"
#include "playback.h"


#define ARTISTWIN_POS       0, LINES / 10
#define ARTISTWIN_WIDTH     COLS / 6
#define ARTISTWIN_HEIGHT    LINES / 2 - 1

#define ALBUMWIN_POS        0, (6 * LINES / 10)
#define ALBUMWIN_WIDTH      COLS / 6
#define ALBUMWIN_HEIGHT     LINES - 6 * LINES / 10 - 1 - 1

#define TRACKWIN_POS        (COLS / 6), LINES / 10
#define TRACKWIN_WIDTH      COLS - 2 * COLS / 3 //COLS / 3
#define TRACKWIN_HEIGHT     LINES - LINES / 10 - 1 - 1

#define LYRICWIN_POS        COLS / 2, 0
#define LYRICWIN_WIDTH      COLS - COLS / 2
#define LYRICWIN_HEIGHT     LINES - 1 - 1

#define INFOWIN_POS         0, 0
#define INFOWIN_WIDTH       COLS / 2
#define INFOWIN_HEIGHT      LINES / 10


#define DEFAULT_OFFSET      2


/*
 * Positions of the Windows
 * ----------------------------------------------
 * |   INFO             |                       |
 * |--------------------|   LYRICS              |
 * |ARTIST   |TITLE     |                       |
 * |         |          |                       |
 * |         |          |                       |
 * |---------|          |                       |
 * |ALBUM    |          |                       |
 * |         |          |                       |
 * ----------------------------------------------
 */


/**
 * @brief definition of the userinterface struct used by clapl
 * This struct sums up some neccessary components as well as some options
 * for the user interface
 */
typedef struct userinterface
{
    WINDOW *artistWin;
    WINDOW *albumWin;
    WINDOW *trackWin;
    WINDOW *lyricWin;
    WINDOW *infoWin;

    WINDOW *selectedWin;

    cache *c;

    char color;
} userinterface;


/**
 * @brief initialize the user interface; is calling initializing functions defined by ncurses
 */
void ui_init (void);

/**
 * @brief redraws the entire user interface (ui have to be created using ui_create)
 * @param ui pointer to the userinterface (have to be created using ui_create)
 */
void ui_redraw (userinterface *ui);

/**
 * @brief creates a single window used in the ui
 * @param x the x coordinate of the window
 * @param y the y coordinate of the window
 * @param width the width of the window
 * @param height the height of the window
 */
WINDOW *win_create (const int x, const int y, const int width, const int height);

/**
 * @brif clears a given window and rewrites the title given by string.
 * @param ui pointer to the user interface
 * @param win pointer to the window to be cleaned
 * @param string title of the window to be displayed in the top left corner of the window
 */
void ui_clear_window (userinterface *ui, WINDOW *win, const char *string);

/**
 * @brief creates the windows of the user interface
 * @param ui pointer to the userinterface
 */
void ui_createWindows (userinterface *ui);

/**
 * @brief creates the user interface
 * @param db pointer to the sqlite3-database (have to be opened before)
 * @param cp pointer to the configparser (have to be loaded before)
 * @return returns the user interface or NULL on failure
 */
userinterface *ui_create (sqlite3 *db, configparser *cp);

/**
 * @brief refreshes the user interface
 * @param ui pointer to the userinterface
 */
void ui_refresh (userinterface *ui);

/**
 * @brief prints a string to a window
 * @param win pointer to the window the string should be written to
 * @param width the maximum width of the window
 * @param line the vertical position of the string
 * @param string pointer to the string
 */
void ui_print_to_window (WINDOW *win, const int width, const int line, const char *string);

/**
 * @brief prints the artists
 * @param ui pointer to the user interface
 */
void ui_print_artist (userinterface *ui);

/**
 * @brief prints the album
 * @param ui pointer to the user interface
 */
void ui_print_album (userinterface *ui);

/**
 * @brief prints the tracks
 * @param ui pointer to the user interface
 */
void ui_print_track (userinterface *ui);

/**
 * @brief reads a single line of text from a file
 * @param file pointer to the file (have to be opened before)
 * @param buffer_size size of the buffer
 * @return pointer to the read line
 */
char *ui_read_line (FILE *file, const int buffer_size);

/**
 * @brief prints the lyrics to the user interface
 * @param ui pointer to the user interface
 */
void ui_print_lyrics (userinterface *ui);

/**
 * @brief prints some info about the current played track
 * @param ui pointer to the user interface
 * @param a pointer to the audio struct of the current played track
 */
void ui_print_info (userinterface *ui, audio *a);


#endif
