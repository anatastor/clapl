
#include "input.h"


void parse_command (char *cmd, userinterface *ui)
{
    char *ptr;
    ptr = strtok(cmd, " ");
    ptr = strtok(NULL, " ");

    logcmd(LOG_MSG, "\n%s\n%s", cmd, ptr ? ptr : "NULL");

    if (strcmp(cmd, "create_playlist") == 0)
    {
        struct entry *e = malloc(sizeof(struct entry));
        e->artist = "Playlists";
        e->album = ptr;

        e->artist_id = db_add_artist(ui->c->db, e);
        db_add_album(ui->c->db, e);
        free(e);
        return;
    }
    
    if (strcmp(cmd, "add_track") == 0)
    {
        struct entry *e = malloc(sizeof(struct entry));
        return;
    }
    
    if (strcmp(cmd, "load") == 0)
    {
        db_add_dir(ui->c->db, ptr);
        sqlite3 *db = ui->c->db;
        cache_close(ui->c);
        ui->c = cache_load(db);
        ui->selectedArtist = 0;
        ui_redraw(ui);
        ui_refresh(ui);
        return;
    }

}


void *playbackThread (void *vargp)
{
    logcmd(LOG_DMSG, "input: playbackThread: playbackThread was started");

    audio *a = (audio*) vargp;
    int ret;
    while (a->threadstate == THREADSTATE_RUNNING)
    {
        if (a->playstate == PLAYSTATE_PAUSE)
            continue;

        ret = playback_playback(a->pb);
        if (ret == 0)
            break;
        else if (ret == -1)
        {
            a->playstate == PLAYSTATE_STOP;
            return NULL;
        }
    }

    if (a->threadstate == THREADSTATE_PAUSE)
        return NULL;

    logcmd(LOG_DMSG, "input: playbackThread: terminating thread");
    playback_free_file(a->pb);
    free(a->pb);
    a->pb = NULL;
    a->threadstate = THREADSTATE_FINISHED;
    a->playstate = PLAYSTATE_NEXT;
    return NULL;
}


void start_playback (audio *a, cache *c, userinterface *ui, pthread_t *thread)
{
    logcmd(LOG_DMSG, "input: start_playback: executing");
    a->pb = playback_open_file(cache_load_filepath(c->db, c->tracks[ui->selectedTrack].id));
    a->playstate = PLAYSTATE_PLAY;
    a->threadstate = THREADSTATE_RUNNING;
    c->currentTrack = c->tracks[ui->selectedTrack];
    ui_print_info(ui, a);
    ui_print_lyrics(ui);
    pthread_create(thread, NULL, playbackThread, a);
    ui_refresh(ui);
}


void input (userinterface *ui, cache *c, audio *a, pthread_t *thread, const char ch)
{
    switch (ch)
    {
        case ':': case 27:
            {
                char msg[255] = "";
                echo();
                timeout(-1);
                refresh();
                mvprintw(LINES - 1, 0, ":");
                getstr(msg);
                logcmd(LOG_MSG, "Entered MSG: %s", msg);
                parse_command(msg, ui);
                cbreak();
                noecho();
                timeout(10);
                move(LINES - 1, 0); // move to the line
                clrtoeol();         // clear the line
                refresh();
                break;
            }
            /*
            char msg[255] = "";
            echo();
            timeout(-1);
            refresh();
            mvprintw(LINES - 1, 0, ":");
            getstr(msg);
            logcmd(LOG_DMSG, "Entered MSG: %s", msg);
            cbreak();
            noecho();
            timeout(10);
            break;
            */

        case 'a': case 'A':
            ui->selectedWin = (ui->selectedWin == ui->artistWin) ? ui->albumWin : ui->artistWin;
            ui_redraw(ui);
            ui_print_info(ui, a);
            if (a->playstate == PLAYSTATE_PLAY)
                ui_print_lyrics(ui);
            ui_refresh(ui);
            break;

        case 'j': case 'J':
            if (ui->selectedWin == ui->artistWin)
            {
                ui->selectedArtist++;
                if (ui->selectedArtist == c->nartists)
                    ui->selectedArtist = 0;
                ui_print_artist(ui);
            }
            else if (ui->selectedWin == ui->albumWin)
            {
                ui->selectedAlbum++;
                if (ui->selectedAlbum == c->nalbum)
                    ui->selectedAlbum = 0;
                ui_print_album(ui);
            }
            ui_refresh(ui);
            break;

        case 'k': case 'K':
            if (ui->selectedWin == ui->artistWin)
            {
                ui->selectedArtist--;
                if (ui->selectedArtist < 0)
                    ui->selectedArtist = c->nartists - 1;
                ui_print_artist(ui);
            }
            else if (ui->selectedWin == ui->albumWin)
            {
                ui->selectedAlbum--;
                if (ui->selectedAlbum < 0)
                    ui->selectedAlbum = c->nalbum - 1;
                ui_print_album(ui);
            }
            ui_refresh(ui);
            break;

        case 'n': case 'N':
            ui->selectedTrack++;
            if (ui->selectedTrack == c->ntracks)
                ui->selectedTrack = 0;
            ui_print_track(ui);
            ui_refresh(ui);
            break;

        case 'b': case 'B':
            ui->selectedTrack--;
            if (ui->selectedTrack < 0)
                ui->selectedTrack = c->ntracks - 1;
            ui_print_track(ui);
            ui_refresh(ui);
            break;

        case 'p': case 'P': case 10: // 10 = ENTER
            if (a->threadstate == THREADSTATE_RUNNING)
            {
                a->threadstate = THREADSTATE_TERMINATE;
                pthread_join(*thread, NULL); // wait until the thread is terminated
            }
            start_playback(a, c, ui, thread);
            ui_refresh(ui);
            break;

        case 32: // 32 = SPACE
            if (a->playstate == PLAYSTATE_PLAY)
            {
                // close thread
                a->playstate = PLAYSTATE_PAUSE;
                a->threadstate = THREADSTATE_PAUSE;
                pthread_join(*thread, NULL);
            }
            else if (a->threadstate == THREADSTATE_PAUSE)
            {
                // recreate thread
                a->playstate = PLAYSTATE_PLAY;
                a->threadstate = THREADSTATE_RUNNING;
                pthread_create(thread, NULL, playbackThread, a);
            }
            ui_print_info(ui, a);
            ui_refresh(ui);
            break;

        case 'c': case 'C':
            a->cycle++;
            if (a->cycle == CYCLE_ENUM_SIZE)
                a->cycle = 0;
            ui_print_info(ui, a);
            ui_refresh(ui);
            break;

        case 'r': case 'R':
            // redraw Winows
            {
                int selectedAlbum = ui->selectedAlbum;
                int selectedTrack = ui->selectedTrack;
                endwin();
                ui_createWindows(ui);
                ui_redraw(ui);
                if (a->threadstate == THREADSTATE_RUNNING)
                    ui_print_lyrics(ui);

                ui->selectedAlbum = selectedAlbum;
                ui_print_album(ui);
                ui->selectedTrack = selectedTrack;
                ui_print_track(ui);
                ui_print_info(ui, a);
                ui_refresh(ui);
                break;
            }
    }

    if (a->playstate == PLAYSTATE_NEXT)
    {
        if (c->tracks[ui->selectedTrack].id == c->currentTrack.id)
        {
            switch (a->cycle)
            {
                case CYCLE_ALL_ARTIST:
                    if (ui->selectedTrack + 1 < c->ntracks)
                        ui->selectedTrack++;
                    else
                    {
                        ui->selectedAlbum++;
                        if (ui->selectedAlbum == c->nalbum)
                            ui->selectedAlbum = 0;
                        ui_print_album(ui);
                    }
                    break;

                case CYCLE_ALL_ALBUM:
                    ui->selectedTrack++;
                    if (ui->selectedTrack == c->ntracks)
                        ui->selectedTrack = 0;
                    break;

                case CYCLE_TRACK:
                    break;
            }
            start_playback(a, c, ui, thread);
            ui_print_track(ui);
            ui_refresh(ui);
        }
        else
            start_playback(a, c, ui, thread);
    }


}
