
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
        const char *default_config = "lyrics=Music/Lyrics/\nsorting=number\nsorting=number # number or title\nenable_color=true  # true (1) or false (0), if terminal does not support color output will always be monochrome\ncolor=7\n#settings for color, only useful if enable_color=true\n# 0 = BLACK\n# 1 = RED\n# 2 = GREEN\n# 3 = YELLOW\n# 4 = BLUE\n# 5 = MAGENTA\n# 6 = CYAN\n# 7 = WHITE\n";
        FILE *config = fopen(".config/clapl/config", "w");
        //fprintf(config, "lyrics=Music/Lyrics/\n");
        //fprintf(config, "sorting=number");
        fprintf(config, default_config);
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

