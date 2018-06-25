
#include "cmd.h"


cmd command_list[] = 
{
    {0, "add", cmd_add, "vs", "help"},
    {0, "seek", cmd_seek, "vi", "help"},
    {0, "rm", cmd_rm, "v", "help"},
    {0, "rmp", cmd_rmp, "v", "help"},
    {0, "load", cmd_load, "vs", "help"},
    {0, "sorting", cmd_sorting, "vs", "help"}
};


cmd_table *
load_commands (void)
{   
    const int command_list_size = 6;

    cmd_table *commands = cmd_table_create(10);
    
    for (int i = 0; i < command_list_size; i++)
        cmd_table_set (commands, &command_list[i]);

    return commands;
}


int
cmd_add (cmd_arg *args)
{   
    userinterface *ui = args[0].v;
    int reload = 0;

    struct entry *e = malloc (sizeof(struct entry));
    e->artist = "Playlists";
    db_add_artist (ui->c->db, e);

    e->album = args[1].s;
    
    db_add_album (ui->c->db, e);

    e->number = 0;
    e->totalnumber = 0;
    e->title = ui->c->tracks[ui->c->selectedTrack].name;
    e->file = cache_load_filepath (ui->c);

    db_add_track (ui->c->db, e);

    if (strcmp(ui->c->artists[ui->c->selectedArtist].name, "Playlists") == 0)
        return 1;
    
    free (e);
    return 0;
}


int
cmd_seek (cmd_arg *args)
{
    audio *a = args[0].v;
    
    if (a && a->threadstate == THREADSTATE_RUNNING || a->threadstate == THREADSTATE_PAUSE)
    {
        int timestamp = args[1].i;
        int duration = a->pb->ctx->duration / AV_TIME_BASE;
        if (timestamp > 0 && timestamp < duration)
        {
            a->playstate = PLAYSTATE_PAUSE;
            playback_seek_timestamp (a->pb, timestamp);
            a->playstate = PLAYSTATE_PLAY;
        }
    }
    return 0;
}


int
cmd_rm (cmd_arg *args)
{   
    userinterface *ui = args[0].v;
    int id = ui->c->tracks[ui->c->selectedTrack].id;
    cache_remove_track (ui->c, id);
    ui_print_track (ui);
    ui_refresh (ui);
    return 0;
}


int
cmd_rmp (cmd_arg *args)
{
    userinterface *ui = args[0].v;
    int id = ui->c->album[ui->c->selectedAlbum].id;
    cache_remove_album (ui->c, id);
    ui_print_album (ui);
    ui_refresh (ui);
    return 0;
}


int
cmd_load (cmd_arg *args)
{
    userinterface *ui = args[0].v;
    db_add_dir (ui->c->db, args[1].s);
    cache_reload (&ui->c);
    ui_redraw (ui);
    ui_refresh (ui);
    return 0;
}


int
cmd_sorting (cmd_arg *args)
{
    userinterface *ui = args[0].v;
    if (strcmp(args[1].s, "number") == 0 || strcmp(args[1].s, "title") == 0)
    {
        free (ui->c->sorting);
        ui->c->sorting = copy_string (args[1].s);
        ui->c->selectedTrack = 0;
        cache_entry_load_tracks (ui->c);
        ui_print_track (ui);
        ui_refresh (ui);
    }

    return 0;
}
