
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


void ui_init (void);

void ui_redraw (userinterface *ui);

WINDOW *win_create (const int x, const int y, const int width, const int height);
void ui_clear_window (userinterface *ui, WINDOW *win, const char *string);
void ui_createWindows (userinterface *ui);

userinterface *ui_create (sqlite3 *db, configparser *cp);

void ui_refresh (userinterface *ui);



void ui_print_to_window (WINDOW *win, const int width, const int line, const char *string);
void ui_print_artist (userinterface *ui);
void ui_print_album (userinterface *ui);
void ui_print_track (userinterface *ui);

char *ui_read_line (FILE *file, const int buffer_size);

void ui_print_lyrics (userinterface *ui);

void ui_print_info (userinterface *ui, audio *a);

/*
 * func (const userinterface *const ui)
 * ui = NULL; -> err
 * ui->artistWin = NULL; -> err
 */


#endif
