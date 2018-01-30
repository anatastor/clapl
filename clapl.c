
#include "clapl.h"


void load_config (configparser *cp)
{
    // loading the configuration file
    LOGGER_FILE = stdout; 
    chdir(getenv("HOME")); // goto /home/user/

    int ret = mkdir(".config/clapl", 0700); // create '.config/clapl/ if it does not exits
    if (ret != 0 && ret != -1)
        logcmd(LOG_MSG, "could not create local config directory");

    struct stat sb;
    ret = stat(".config/clapl/config", &sb); // check if config file exists
    if (ret != 0) 
    { 
        // config file does not exitst
        // create defaults
        const char *default_config = "lyrics=Music/Lyrics/\nsorting=number # number or title\nlog_file=.config/clapl/log.log\nenable_color=true  # true (1) or false (0), if terminal does not support color output will always be monochrome\ncolor=7\n#settings for color, only useful if enable_color=true\n# 0 = BLACK\n# 1 = RED\n# 2 = GREEN\n# 3 = YELLOW\n# 4 = BLUE\n# 5 = MAGENTA\n# 6 = CYAN\n# 7 = WHITE\n";
        FILE *config = fopen(".config/clapl/config", "w");
        fprintf(config, default_config);
        fclose(config);
    }
    
    //initialize and load config file
    configparser_init(cp, ".config/clapl/config", 2, '=');
    configparser_load(cp);
}


int main (int argc, char **argv)
{
    configparser cp;
    load_config(&cp); // load config file
    
    //LOGGER_PATH = ".config/clapl/log.txt";
    LOGGER_PATH = configparser_get_string(&cp, "log_file"); // set path for log file
    LOGGER_FILE = fopen(LOGGER_PATH, "w"); // open log file

    playback_init(); // initialize libav and libao
    sqlite3 *db = db_init();
    ui_init(); // initialize userinterface
    userinterface *ui = ui_create(db, &cp); // create userinterface

    audio *a = malloc(sizeof(audio));
    a->pb = 0;
    a->playstate = PLAYSTATE_STOP;
    a->cycle = CYCLE_ALL_ARTIST;
    ui_print_info(ui, a); // prints infos of the currently playing track
    ui_refresh(ui);
    pthread_t thread;

    char ch;
    while ((ch = getch()) != 'q') // main loop
        input(ui, ui->c, a, &thread, ch);

    endwin(); // close ncurses
    cache_close(ui->c);
    free(ui);
    db_close(db);
    if (db)
        free(db);

    fclose(LOGGER_FILE); // close log file
    return 0;
}

