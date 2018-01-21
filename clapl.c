
#include "clapl.h"


int main (int argc, char **argv)
{
    LOGGER_PATH = "log.txt";
    LOGGER_FILE = fopen(LOGGER_PATH, "w");
    logcmd(LOG_MSG, "file opened");

    playback_init();

    sqlite3 *db = db_init();
    cache *c = cache_load(db);
    c->lyrics_path = "/home/mjoelnir/Music/Lyrics/";


    ui_init();
    userinterface *ui = ui_create(c);

    audio *a = malloc(sizeof(audio));
    a->pb = 0;
    a->threadstate = PLAYSTATE_STOP;
    a->cycle = CYCLE_ALL_ARTIST;
    ui_print_info(ui, a);
    ui_refresh(ui);
    pthread_t thread;

    char ch;
    while ((ch = getch()) != 'q')
    {
        input(ui, c, a, &thread, ch);

    }

    endwin();
    free(ui);


    cache_close(c);


    db_close(db);
    if (db)
        free(db);


    fclose(LOGGER_FILE);

    return 0;
}
