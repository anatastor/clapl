
#include "clapl.h"


int main (int argc, char **argv)
{
    LOGGER_FILE = stdout;
    chdir(getenv("HOME"));
    int ret = mkdir(".config/clapl", 0700);
    if (ret != 0 && ret != -1)
        logcmd(LOG_MSG, "could not create local config directory");

    configparser cp;
    
    struct stat sb;
    ret = stat(".config/clapl/config", &sb);
    if (ret != 0)
    {
        FILE *config = fopen(".config/clapl/config", "w");
        fprintf(config, "lyrics=Music/Lyrics/");
        fclose(config);
    }
    
    configparser_init(&cp, ".config/clapl/config", 1, '=');
    configparser_load(&cp);


    LOGGER_PATH = ".config/clapl/log.txt";
    LOGGER_FILE = fopen(LOGGER_PATH, "w");
    logcmd(LOG_MSG, "file opened");

    playback_init();

    sqlite3 *db = db_init();
    /*
    cache *c = cache_load(db);
    c->lyrics_path = "/home/mjoelnir/Music/Lyrics/";
    */
    
    ui_init();
    userinterface *ui = ui_create(cache_load(db));
    ui->c->lyrics_path = configparser_get_string(&cp, "lyrics");

    audio *a = malloc(sizeof(audio));
    a->pb = 0;
    a->playstate = PLAYSTATE_STOP;
    a->cycle = CYCLE_ALL_ARTIST;
    ui_print_info(ui, a);
    ui_refresh(ui);
    pthread_t thread;

    char ch;
    while ((ch = getch()) != 'q')
        input(ui, ui->c, a, &thread, ch);

    endwin();
    cache_close(ui->c);
    free(ui);
    db_close(db);
    if (db)
        free(db);

    fclose(LOGGER_FILE);
    return 0;
}

