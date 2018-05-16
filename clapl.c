
#include "clapl.h"


void load_config (configparser *cp)
{
    // loading the configuration file
    LOGGER_FILE = stdout; 
    chdir(getenv("HOME")); // goto the home directory

    int ret = mkdir(".config/clapl", 0700); // create '.config/clapl/ if it does not exits
    if (ret != 0 && ret != -1)
        logcmd(LOG_MSG, "could not create local config directory");

    struct stat sb;
    ret = stat(".config/clapl/config", &sb); // check if config file exists
    if (ret != 0) 
    { 
        fprintf(stderr, "could not load config file\ncheck ~/.config/clapl/config");
        free(cp);
        exit(1);
    }
    
    //initialize and load config file
    configparser_init(cp, ".config/clapl/config", 5, '=');
    configparser_load(cp);
}


int main (int argc, char **argv)
{
    srand(time(NULL));

    configparser *cp = malloc(sizeof(configparser));
    load_config(cp);
    
    LOGGER_PATH = configparser_get_string(cp, "log_file"); // set path for log file
    LOGGER_FILE = fopen(LOGGER_PATH, "w");

    playback_init(); // initialize libav and libao
    sqlite3 *db = db_init();
    ui_init(); // initialize userinterface
    userinterface *ui = ui_create(db, cp);

    ui->c->commands = load_commands();

    audio *a = audio_create();
    ui_print_info(ui, a);
    ui_refresh(ui);
    pthread_t thread;

    char ch;
    while (1)
    {
        ch = getch();
        input(ui, ui->c, a, &thread, ch);
        if (ch == 'q' || ch == 'Q')
            break;
    }

    endwin(); // close ncurses
    audio_free(&a);
    configparser_free(&cp);
    cache_close(ui->c);
    free(ui);
    db_close(db);
    
    fclose(LOGGER_FILE);
    return EXIT_SUCCESS;
}

