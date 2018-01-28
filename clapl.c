
#include "clapl.h"


void load_config (configparser *cp)
{
    LOGGER_FILE = stdout;
    chdir(getenv("HOME"));
    int ret = mkdir(".config/clapl", 0700);
    if (ret != 0 && ret != -1)
        logcmd(LOG_MSG, "could not create local config directory");

    struct stat sb;
    ret = stat(".config/clapl/config", &sb);
    if (ret != 0)
    {
        FILE *config = fopen(".config/clapl/config", "w");
        fprintf(config, "lyrics=Music/Lyrics/\n");
        fprintf(config, "sorting=number");
        fclose(config);
    }
    
    configparser_init(cp, ".config/clapl/config", 2, '=');
    configparser_load(cp);


}

int main (int argc, char **argv)
{
    configparser cp;
    load_config(&cp);

    LOGGER_PATH = ".config/clapl/log.txt";
    LOGGER_FILE = fopen(LOGGER_PATH, "w");

    playback_init();
    sqlite3 *db = db_init();
    ui_init();
    userinterface *ui = ui_create(db, &cp);

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

