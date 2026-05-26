#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

// 날짜 관련 로그
int isDateChanged() {
	static struct tm initial_date = {0};
    time_t now = time(NULL);
    struct tm current_date;
	
	FILE *file1 = openLogFile(FILENAME_DATE, "r");
    
	localtime_r(&now, &current_date); // DateTime read
	
    // 초기 날짜가 설정되지 않은 경우, 초기 날짜를 현재 날짜로 설정
    if (initial_date.tm_year == 0 && initial_date.tm_mon == 0 && initial_date.tm_mday == 0) 
	{
		if (fscanf(file1, "%d %d %d", &initial_date.tm_year, &initial_date.tm_mon, &initial_date.tm_mday) == EOF)
		{
			perror("Error opening file \nTime.txt, 내용에는 0 0 0만 있는 파일을 미리 생성해야 함.\n");
			fclose(file1);
			exit(EXIT_FAILURE);
		}
    }
	else
		fscanf(file1, "%d %d %d", &initial_date.tm_year, &initial_date.tm_mon, &initial_date.tm_mday);
	
	fclose(file1);
	
    if (initial_date.tm_year != current_date.tm_year || initial_date.tm_mon != current_date.tm_mon || initial_date.tm_mday != current_date.tm_mday) 
	{
		file1 = openLogFile(FILENAME_DATE, "w");
		fprintf(file1, "%d %d %d", current_date.tm_year, current_date.tm_mon, current_date.tm_mday);
		fclose(file1);
        return 1;
    }
	
    return 0;
}

FILE *openLogFile(char *fileName, char *mode) {
    // close 안하면 메모리에 남아서 나중에 다운됨. 
	// open 할때마다 fclose()해줘야함.
	FILE *file = fopen(fileName, mode);
	if (file == NULL) 
	{
		perror("Error opening file \n");
		exit(EXIT_FAILURE);
	}
	
	return file;
}
