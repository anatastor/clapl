
#include "ui.h"


void ui_init (void)
{
    initscr();
    setlocale(LC_ALL, "de_DE"); // set local language to german -> Umlaute
    noecho();
    cbreak();
    nodelay(NULL, TRUE);
    timeout(10);
    curs_set(0);
    refresh();
}


void ui_redraw (userinterface *ui)
{   
    clear();
    ui_clear_window(ui, ui->artistWin, "Artist");
    ui_clear_window(ui, ui->albumWin, "Album");
    ui_clear_window(ui, ui->trackWin, "Track");
    ui_clear_window(ui, ui->lyricWin, "Lyrics");
    ui_clear_window(ui, ui->infoWin, NULL);
    
    ui_print_artist(ui);
}


WINDOW *win_create (const int x, const int y, const int width, const int height)
{
    WINDOW *win = NULL;
    win = newwin(height, width, y, x);
    if (! win)
        logcmd(LOG_ERROR, "ui: win_create: could not create WINDOW");

    nodelay(win, TRUE);

    wrefresh(win);
    return win;
}


void ui_clear_window (userinterface *ui, WINDOW *win, const char *string)
{
    werase(win);
    refresh();
    if (ui->selectedWin == win)
        wattron(win, ui->color ? COLOR_PAIR(1) : A_STANDOUT);
    box(win, 0, 0);
    mvwprintw(win, 0, 0, "%s", string ? string : "");
    wattroff(win, ui->color ? COLOR_PAIR(1) : A_STANDOUT);

    wrefresh(win);
    refresh();
}


void ui_createWindows (userinterface *ui)
{   
    ui->artistWin = win_create(ARTISTWIN_POS, ARTISTWIN_WIDTH, ARTISTWIN_HEIGHT);
    ui->albumWin = win_create(ALBUMWIN_POS, ALBUMWIN_WIDTH, ALBUMWIN_HEIGHT);
    ui->trackWin = win_create(TRACKWIN_POS, TRACKWIN_WIDTH, TRACKWIN_HEIGHT);
    ui->lyricWin = win_create(LYRICWIN_POS, LYRICWIN_WIDTH, LYRICWIN_HEIGHT);
    ui->infoWin = win_create(INFOWIN_POS, INFOWIN_WIDTH, INFOWIN_HEIGHT);

    ui->selectedWin = ui->artistWin;

    refresh();
}


userinterface *ui_create (sqlite3 *db, configparser *cp)
{
    userinterface *ui = malloc(sizeof(userinterface));
    if (! ui)
        logcmd(LOG_ERROR_MALLOC, "ui: ui_create: unable to allocate memory for ui");

    if (has_colors() == TRUE && configparser_get_bool(cp, "enable_color"))
    {
        ui->color = 1;
        start_color();
        use_default_colors();
        int color = configparser_get_int(cp, "color");
        if (color < COLOR_BLACK || color > COLOR_WHITE)
            color = COLOR_RED;
        init_pair(1, color, COLOR_BLACK);
    }
    else
        ui->color = 0;

    ui_createWindows(ui);

    ui->c = cache_load(db);
    ui->c->lyrics_path = configparser_get_string(cp, "lyrics");
    ui->c->sorting = configparser_get_string(cp, "sorting");

    ui_redraw(ui);
    ui_refresh(ui);

    logcmd(LOG_DMSG, "ui created");
    return ui;
}


void ui_refresh (userinterface *ui)
{   
    wrefresh(ui->artistWin);
    wrefresh(ui->albumWin);
    wrefresh(ui->trackWin);
    wrefresh(ui->lyricWin);
    wrefresh(ui->infoWin);
}


void ui_print_to_window (WINDOW *win, const int width, const int line, const char *string)
{
    for (int i = 1; i < width && string; i++)
    {
        if (*string == '\0')
            return;
        mvwprintw(win, line, i, "%c", *string);
        string++;
    }
}


void ui_print_artist (userinterface *ui)
{
    ui_clear_window(ui, ui->artistWin, "Artist");
    if (ui->c->selectedArtist == -1)
        return;

    int offset =  0;
    if (ui->c->selectedArtist > (ARTISTWIN_HEIGHT - 4))
        offset = ui->c->selectedArtist - (ARTISTWIN_HEIGHT - 4);

    for (int i = 1; (i < (ARTISTWIN_HEIGHT - 2)) && (i <= ui->c->nartists); i++)
    {   
        if ((i + offset - 1) == ui->c->selectedArtist)
        {
            wattron(ui->artistWin, ui->color ? COLOR_PAIR(1) : A_STANDOUT);
            ui_print_to_window(ui->artistWin, (ARTISTWIN_WIDTH - 1), i, ui->c->artists[i + offset - 1].name);
            wattroff(ui->artistWin, ui->color ? COLOR_PAIR(1) : A_STANDOUT);
            continue;
        }
        else
            ui_print_to_window(ui->artistWin, (ARTISTWIN_WIDTH - 2), i, ui->c->artists[i + offset - 1].name);
    }
    
    cache_entry_load_album(ui->c);
    
    ui_print_album(ui);
}


void ui_print_album (userinterface *ui)
{   
    ui_clear_window(ui, ui->albumWin, "Album");

    int offset = 0;
    if (ui->c->selectedAlbum > (ALBUMWIN_HEIGHT - 4))
        offset = ui->c->selectedAlbum - (ALBUMWIN_HEIGHT - 4);

    for (int i = 1; (i < (ALBUMWIN_HEIGHT -2)) && (i <= ui->c->nalbum); i++)
    {   
        if ((i + offset - 1) == ui->c->selectedAlbum)
        {
            wattron(ui->albumWin, ui->color ? COLOR_PAIR(1) : A_STANDOUT);
            ui_print_to_window(ui->albumWin, (ALBUMWIN_WIDTH - 2), i, ui->c->album[i + offset - 1].name);
            wattroff(ui->albumWin, ui->color ? COLOR_PAIR(1) : A_STANDOUT);
        }
        else
            ui_print_to_window(ui->albumWin, (ALBUMWIN_WIDTH - 2), i, ui->c->album[i + offset -1].name);
    }

    cache_entry_load_tracks(ui->c);
    
    ui_print_track(ui);
}


void ui_print_track (userinterface *ui)
{   
    ui_clear_window(ui, ui->trackWin, "Track");

    int offset = 0;
    if (ui->c->selectedTrack > (TRACKWIN_HEIGHT - 4))
        offset = ui->c->selectedTrack - (TRACKWIN_HEIGHT - 4);

    for (int i = 1; (i < (TRACKWIN_HEIGHT - 2)) && (i <= ui->c->ntracks); i++)
    {   
        if ((i + offset - 1) == ui->c->selectedTrack)
        {
            wattron(ui->trackWin, ui->color ? COLOR_PAIR(1) : A_STANDOUT);
            ui_print_to_window(ui->trackWin, (TRACKWIN_WIDTH - 1), i, ui->c->tracks[ui->c->selectedTrack].name);
            wattroff(ui->trackWin, ui->color ? COLOR_PAIR(1) : A_STANDOUT);
        }
        else
            ui_print_to_window(ui->trackWin, (TRACKWIN_WIDTH - 2), i, ui->c->tracks[i + offset - 1].name);
    }
}


char *ui_read_line (FILE *file, const int buffer_size)
{   
    char c;
    char *line = malloc(sizeof(char) * buffer_size);
    
    for (int i = 0; i < buffer_size; i++)
        line[i] = 0;

    int pos = 0;

    c = getc(file);
    while (c != '\n' && c != EOF)
    {
        line[pos] = c;
        if (++pos >= buffer_size)
            break;
        c = getc(file);
    }

    if (c == EOF && pos == 0)
        return NULL;

    return line;
}


void ui_print_lyrics (userinterface *ui)
{
    ui_clear_window(ui, ui->lyricWin, "Lyrics");

    int size = snprintf(NULL, 0, "%s%s", ui->c->lyrics_path, ui->c->currentTrack.name) + 1;
    char *path = malloc(sizeof(char) * size);
    snprintf(path, size, "%s%s", ui->c->lyrics_path, ui->c->currentTrack.name);

    FILE *file = NULL;
    char *buffer = NULL;
    int pos = 2;
    if ((file = fopen(path, "r")))
    {
        buffer = ui_read_line(file, LYRICWIN_WIDTH - 2);
        while (buffer && pos < LYRICWIN_HEIGHT - 1)
        {
            mvwprintw(ui->lyricWin, pos, 1, "%s", buffer);
            free(buffer);
            buffer = ui_read_line(file, LYRICWIN_WIDTH - 2);
            pos++;
        }
        fclose(file);
    }
    else
        mvwprintw(ui->lyricWin, 2, 1, "no lyrics found: %s", path);

}


void ui_print_info (userinterface *ui, audio *a)
{
    logcmd(LOG_DMSG, "ui_print_info: executing");
    ui_clear_window(ui, ui->infoWin, NULL);
    switch (a->playstate)
    {
        case PLAYSTATE_PLAY:
            mvwprintw(ui->infoWin, 1, 1, "playing \"%s\"", ui->c->currentTrack.name);
            break;

        case PLAYSTATE_PAUSE:
            mvwprintw(ui->infoWin, 1, 1, "pause \"%s\"", ui->c->currentTrack.name);
            break;

        default:
            mvwprintw(ui->infoWin, 1, 1, "not playing");
            break;
    }

    switch (a->cycle)
    {
        case CYCLE_ALL_ARTIST:
            mvwprintw(ui->infoWin, 2, 1, "repeat artist");
            break;

        case CYCLE_ALL_ALBUM:
            mvwprintw(ui->infoWin, 2, 1, "repeat album");
            break;
        
        case CYCLE_TRACK:
            mvwprintw(ui->infoWin, 2, 1, "repeat track");
            break;

        case CYCLE_RANDOM:
            mvwprintw(ui->infoWin, 2, 1, "random");
            break;
    }
    wrefresh(ui->infoWin);
}
