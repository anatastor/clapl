
#include "logger.h"


const char *LOG_STR[] =
{
    "DEBUG MSG",
    "MSG",
    "WARNING",
    "ERROR",
    "ERROR_MALLOC",
    "ERROR_PARAM",
    "ERROR_IO"
};


void logcmd (const LOG_CODE code, char *fmt, ...)
{
    if (!DEBUG_MODE && code == LOG_DMSG)
        return;
    // do not print debugging messages

    fprintf(LOGGER_FILE, "[%s] ", LOG_STR[code]);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(LOGGER_FILE, fmt, arg);
    va_end(arg);
    fprintf(LOGGER_FILE, "\n");

    if (LOGGER_FILE != stderr || LOGGER_FILE != stdout)
    {
        fclose(LOGGER_FILE);
        LOGGER_FILE = fopen(LOGGER_PATH, "a");
    }

    if (code > 1)
        exit(code);

    return;
}
