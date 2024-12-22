#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define FILENAME "logs/LightPerDay.txt"
#define FILENAME_DATE "logs/Time.txt"

int isDateChanged();
FILE *openLogFile(char *fileName, char *mode);

#endif
