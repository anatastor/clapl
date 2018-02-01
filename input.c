
#include "input.h"


void parse_command (char *cmd, userinterface *ui, audio *a)
{
    if (*cmd == '\0')
        return;

    char *ptr = configparser_split_string(cmd, ' ');

    const char *commands[] = 
    {
        "add",
        "seek",
        "rm",
        "rmp",
        "load"
    };

    const char *command_help[] = 
    {
        "add 'name of playlist': adding the selected track into the given playlist, playlist will be created if it does not exist",
        "seek 'int timestamp': seeking forward the track to the given timestamp in seconds (may take a while to complete",
        "rm: removes the selected track from the playlist",
        "rmp: removes the selected playlist",
        "load 'path to dir or file': loads the given directory or file into the database"
    };

    if (strcmp(cmd, "help") == 0)
    {
        if (!ptr)
        {
            mvprintw(LINES - 2, 1, "commands: %s, %s, %s, %s, %s (%s)", commands[0], commands[1], commands[2], commands[3], commands[4], "use help 'command' to get information about each command");
        }
        else
        {
            for (int i = 0; i < 5; i++)
            {
                if (strcmp(ptr, commands[i]) == 0)
                    mvprintw(LINES - 2, 1, command_help[i]);
            }
        }
        return;
    }

    if (strcmp(cmd, "add") == 0) // add the selected track to the given playlist
    {
        int reload = 0;
        struct entry *e = malloc(sizeof(struct entry));
        e->artist = "Playlists";
        if (db_add_artist(ui->c->db, e))
            reload = 1;

        e->album = ptr;
        if(db_add_album(ui->c->db, e))
            reload = 1;
        
        e->number = 0;
        e->totalnumber = 0;
        e->title = ui->c->tracks[ui->c->selectedTrack].name;
        e->file = cache_load_filepath(ui->c);
        
        if (db_add_track(ui->c->db, e))
            reload = 1;

        if (reload && strcmp(ui->c->artists[ui->c->selectedArtist].name, "Playlists") == 0)
        {
            cache_reload(&ui->c);
            ui_redraw(ui);
            ui_refresh(ui);
        }
        return;
    }

    if (strcmp(cmd, "seek") == 0) // seek forward to given position
    {
        if (a && (a->threadstate == THREADSTATE_RUNNING || a->threadstate == THREADSTATE_PAUSE))
        {
            int timestamp = atoi(ptr);
            int duration = a->pb->ctx->duration / AV_TIME_BASE;
            if (timestamp > 0 && timestamp < duration)
            {
                a->playstate = PLAYSTATE_PAUSE;
                playback_seek_timestamp(a->pb, timestamp);
                a->playstate = PLAYSTATE_PLAY;
            }
        }
        return;
    }

    if (strcmp(cmd, "rm") == 0 && strcmp(ui->c->artists[ui->c->selectedArtist].name, "Playlists") == 0) // remove selected track from the playlist
    {
        int id = ui->c->tracks[ui->c->selectedTrack].id;
        cache_remove_track(ui->c, id);
        ui_print_track(ui);
        ui_refresh(ui);
        return;
    }

    if (strcmp(cmd, "rmp") == 0 && strcmp(ui->c->artists[ui->c->selectedArtist].name, "Playlists") == 0) // remove playlist
    {
        int id = ui->c->album[ui->c->selectedAlbum].id;
        cache_remove_album(ui->c, id);
        ui_print_album(ui);
        ui_refresh(ui);
        return;
    }
    
    if (strcmp(cmd, "load") == 0) // load a directory or file into the database
    {
        db_add_dir(ui->c->db, ptr);
        cache_reload(&ui->c);
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
        if (ret == 0) // EOF
            break;
        else if (ret == -1) // ERROR
        {
            a->playstate == PLAYSTATE_STOP;
            return NULL;
        }
    }

    if (a->threadstate == THREADSTATE_PAUSE)
        return NULL;

    logcmd(LOG_DMSG, "input: playbackThread: terminating thread");
    a->threadstate = THREADSTATE_FINISHED;
    playback_free_file(a->pb);
    free(a->pb);
    a->pb = NULL;
    a->playstate = PLAYSTATE_NEXT;
    return NULL;
}


void start_playback (audio *a, userinterface *ui, pthread_t *thread)
{
    logcmd(LOG_DMSG, "input: start_playback: executing");
    a->pb = playback_open_file(cache_load_filepath(ui->c));
    logcmd(LOG_DMSG, "input: start_playback: opened file: %s", a->pb);
    a->playstate = PLAYSTATE_PLAY;
    a->threadstate = THREADSTATE_RUNNING;
    ui->c->currentTrack = ui->c->tracks[ui->c->selectedTrack];
    ui_print_info(ui, a);
    ui_print_lyrics(ui);
    pthread_create(thread, NULL, playbackThread, a);
    ui_refresh(ui);
}


char *input_get_time (int t)
{
    int s = t % 60;
    int m = (t / 60) % 60;
    int h = (t / 60) / 60;
    char *out = malloc(sizeof(char) * 9);
    out[0] = h / 10 + 48;
    out[1] = h % 10 + 48;
    out[2] = ':';
    out[3] = m / 10 + 48;
    out[4] = m % 10 + 48;
    out[5] = ':';
    out[6] = s / 10 + 48;
    out[7] = s % 10 + 48;
    out[8] = '\0';
    return out;
}


void input (userinterface *ui, cache *c, audio *a, pthread_t *thread, const char ch)
{   
    if (a && (a->threadstate == THREADSTATE_RUNNING || a->threadstate == THREADSTATE_PAUSE))
    {
        int duration = a->pb->ctx->duration / AV_TIME_BASE;
        int pos = av_frame_get_best_effort_timestamp(a->pb->frame) / a->pb->ctx->streams[0]->time_base.den;
        char *spos = input_get_time(pos);
        char *sdur = input_get_time(duration);
        mvprintw(3, 2, "%s | %s", spos, sdur);
        free(spos);
        free(sdur);
    }

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
                parse_command(msg, ui, a);
                cbreak();
                noecho();
                timeout(10);
                move(LINES - 1, 0); // move to the line
                clrtoeol();         // clear the line
                ui_print_info(ui, a);
                refresh();
                break;
            }

        case 9: // Tabulator
            ui->selectedWin = (ui->selectedWin == ui->artistWin) ? ui->albumWin : ui->artistWin;
            ui_redraw(ui);
            ui_print_info(ui, a);
            if (a->playstate == PLAYSTATE_PLAY || a->playstate == PLAYSTATE_PAUSE)
                ui_print_lyrics(ui);
            ui_refresh(ui);
            break;

        case 'j': case 'J':
            if (ui->selectedWin == ui->artistWin)
            {
                ui->c->selectedArtist++;
                if (ui->c->selectedArtist == ui->c->nartists)
                    ui->c->selectedArtist = 0;
                ui_print_artist(ui);
            }
            else if (ui->selectedWin == ui->albumWin)
            {
                ui->c->selectedAlbum++;
                if (ui->c->selectedAlbum == ui->c->nalbum)
                    ui->c->selectedAlbum = 0;
                ui_print_album(ui);
            }
            ui_refresh(ui);
            break;

        case 'k': case 'K':
            if (ui->selectedWin == ui->artistWin)
            {
                ui->c->selectedArtist--;
                if (ui->c->selectedArtist < 0)
                    ui->c->selectedArtist = ui->c->nartists - 1;
                ui_print_artist(ui);
            }
            else if (ui->selectedWin == ui->albumWin)
            {
                ui->c->selectedAlbum--;
                if (ui->c->selectedAlbum < 0)
                    ui->c->selectedAlbum = ui->c->nalbum - 1;
                ui_print_album(ui);
            }
            ui_refresh(ui);
            break;

        case 'n': case 'N':
            ui->c->selectedTrack++;
            if (ui->c->selectedTrack == ui->c->ntracks)
                ui->c->selectedTrack = 0;
            ui_print_track(ui);
            ui_refresh(ui);
            break;

        case 'b': case 'B':
            ui->c->selectedTrack--;
            if (ui->c->selectedTrack < 0)
                ui->c->selectedTrack = ui->c->ntracks - 1;
            ui_print_track(ui);
            ui_refresh(ui);
            break;

        case 'p': case 'P': case 10: // 10 = ENTER
            if (a->threadstate == THREADSTATE_RUNNING)
            {
                a->threadstate = THREADSTATE_TERMINATE;
                pthread_join(*thread, NULL); // wait until the thread is terminated
            }
            start_playback(a, ui, thread);
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
                endwin();
                ui_createWindows(ui);
                ui_redraw(ui);
                if (a->threadstate == THREADSTATE_RUNNING)
                    ui_print_lyrics(ui);

                ui_print_album(ui);
                ui_print_track(ui);
                ui_print_info(ui, a);
                ui_refresh(ui);
                break;
            }
    }

    if (a->playstate == PLAYSTATE_NEXT)
    {
        if (c->tracks[ui->c->selectedTrack].id == c->currentTrack.id)
        {
            switch (a->cycle)
            {
                case CYCLE_ALL_ARTIST:
                    if (ui->c->selectedTrack + 1 < c->ntracks)
                        ui->c->selectedTrack++;
                    else
                    {
                        ui->c->selectedAlbum++;
                        if (ui->c->selectedAlbum == c->nalbum)
                            ui->c->selectedAlbum = 0;
                        ui_print_album(ui);
                    }
                    break;

                case CYCLE_ALL_ALBUM:
                    ui->c->selectedTrack++;
                    if (ui->c->selectedTrack == c->ntracks)
                        ui->c->selectedTrack = 0;
                    break;

                case CYCLE_TRACK:
                    break;
            }
            start_playback(a, ui, thread);
            ui_print_track(ui);
            ui_refresh(ui);
        }
        else
            start_playback(a, ui, thread);
    }
}
