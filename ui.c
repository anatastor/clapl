
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
        wattron(win, A_STANDOUT);
    box(win, 0, 0);
    mvwprintw(win, 0, 0, "%s", string ? string : "");
    wattroff(win, A_STANDOUT);

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


userinterface *ui_create (cache *c)
{
    userinterface *ui = malloc(sizeof(userinterface));
    if (! ui)
        logcmd(LOG_ERROR_MALLOC, "ui: ui_create: unable to allocate memory for ui");

    ui_createWindows(ui);

    ui->c = c;
    ui->selectedArtist = 0;

    ui->inputType = INPUT_DEFAULT;
    
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
            break;
        mvwprintw(win, line, i, "%c", *string);
        string++;
    }
}


void ui_print_artist (userinterface *ui)
{
    ui_clear_window(ui, ui->artistWin, "Artist");

    int offset =  0;
    if (ui->selectedArtist > (ARTISTWIN_HEIGHT - 4))
        offset = ui->selectedArtist - (ARTISTWIN_HEIGHT - 4);
    
    for (int i = 1; (i < (ARTISTWIN_HEIGHT - 2)) && (i <= ui->c->nartists); i++)
    {   
        if ((i + offset - 1) == ui->selectedArtist)
        {
            wattron(ui->artistWin, A_STANDOUT);
            ui_print_to_window(ui->artistWin, (ARTISTWIN_WIDTH - 1), i, ui->c->artists[i + offset - 1].name);
            wattroff(ui->artistWin, A_STANDOUT);
        }
        else
            ui_print_to_window(ui->artistWin, (ARTISTWIN_WIDTH - 2), i, ui->c->artists[i + offset - 1].name);
    }
    
    if (ui->c->album)
        free(ui->c->album);
    ui->c->album = cache_entry_load_album(ui->c->db, &ui->c->nalbum, ui->c->artists[ui->selectedArtist].id);
    
    ui->selectedAlbum = 0;
    ui_print_album(ui);
}


void ui_print_album (userinterface *ui)
{   
    ui_clear_window(ui, ui->albumWin, "Album");

    int offset = 0;
    if (ui->selectedAlbum > (ALBUMWIN_HEIGHT - 4))
        offset = ui->selectedAlbum - (ALBUMWIN_HEIGHT - 4);

    for (int i = 1; (i < (ALBUMWIN_HEIGHT -2)) && (i <= ui->c->nalbum); i++)
    {   
        if ((i + offset - 1) == ui->selectedAlbum)
        {
            wattron(ui->albumWin, A_STANDOUT);
            ui_print_to_window(ui->albumWin, (ALBUMWIN_WIDTH - 2), i, ui->c->album[i + offset - 1].name);
            wattroff(ui->albumWin, A_STANDOUT);
        }
        else
            ui_print_to_window(ui->albumWin, (ALBUMWIN_WIDTH - 2), i, ui->c->album[i + offset -1].name);
    }

    if (ui->c->tracks)
        free(ui->c->tracks);
    ui->c->tracks = cache_entry_load_track(ui->c->db, &ui->c->ntracks, ui->c->album[ui->selectedAlbum].id, ui->c->artists[ui->selectedArtist].id);
    
    ui->selectedTrack = 0;
    ui_print_track(ui);
}


void ui_print_track (userinterface *ui)
{   
    ui_clear_window(ui, ui->trackWin, "Track");

    int offset = 0;
    if (ui->selectedTrack > (TRACKWIN_HEIGHT - 4))
        offset = ui->selectedTrack - (TRACKWIN_HEIGHT - 4);

    for (int i = 1; (i < (TRACKWIN_HEIGHT - 2)) && (i <= ui->c->ntracks); i++)
    {   
        if ((i + offset - 1) == ui->selectedTrack)
        {
            wattron(ui->trackWin, A_STANDOUT);
            ui_print_to_window(ui->trackWin, (TRACKWIN_WIDTH - 1), i, ui->c->tracks[ui->selectedTrack].name);
            wattroff(ui->trackWin, A_STANDOUT);
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


//void ui_print_info (userinterface *ui, const int threadstate, const int playstate, const int cycle)
void ui_print_info (userinterface *ui, audio *a)
{
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
    }
}
