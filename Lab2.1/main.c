#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h> 
#include <string.h>
#include <dirent.h>
#include <limits.h>


FILE *file;


char* concat(char *s1, char *s2) {

    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);                      

    char *result = malloc(len1 + len2 + 1);

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);    

    return result;
}

void find_in_directory(char *directory, char *search_name, int min_size, int max_size, time_t min_time, time_t max_time)
{
    DIR *dir;
    struct dirent *ent;
    struct stat buf = { 0 };
    int i;
    time_t time;
    
    struct tm * timeinfo;
    if (directory[strlen(directory) - 1] != '/')
        directory = concat(directory, "/");
    if ((dir = opendir(directory)) != NULL) 
        while ((ent = readdir(dir)) != NULL) 
        {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, ".."))
                if (opendir(concat(directory, ent->d_name)) != NULL)
                {
                    find_in_directory(concat(directory, ent->d_name), search_name, min_size, max_size, min_time, max_time);
                }
                else
                {
                    stat(concat(directory, ent->d_name), &buf);
                    time = mktime(localtime(&buf.st_mtime));
                    if ((buf.st_size >= min_size) && (buf.st_size <= max_size) && (time >= min_time) && (time <= max_time))
                    {
                        printf("Path: %s%s\n", directory, ent->d_name);
                        fprintf(file, "Path: %s%s\n", directory, ent->d_name);

                        printf("Size: %d bytes\n", buf.st_size);
                        fprintf(file, "Size: %d bytes\n", buf.st_size);

                        timeinfo = localtime (&buf.st_mtime);
                        printf("Modificated time: %s\n", asctime(timeinfo));
                        fprintf(file, "Modificated time: %s\n", asctime(timeinfo));
                    }
                    
                }
        }   
    closedir(dir);

}

#define xDEBUG

int main(int argc, char *argv[])
{   

    struct tm tm;
    time_t min_time;
    time_t max_time;

#ifndef DEBUG
    char *min_timec;
    char *max_timec;
    int min_size, max_size;
#else
    char *min_timec = "23.02.2000 22:00";
    char *max_timec = "10.10.2030 10:00";
    int min_size = atoi("0"), max_size = atoi("2000");
    strptime(min_timec, "%d.%m.%Y %R", &tm);
    min_time = mktime(&tm);
    strptime(max_timec, "%d.%m.%Y %R", &tm);
    max_time = mktime(&tm);
#endif
    

    file = fopen("file.txt", "w");

#ifndef DEBUG
    if (argc == 11)
    {
        min_timec = concat(concat(argv[7], " "), argv[8]);
        max_timec = concat(concat(argv[9], " "), argv[10]);
        strptime(min_timec, "%d.%m.%Y %R", &tm);
        min_time = mktime(&tm);
        strptime(max_timec, "%d.%m.%Y %R", &tm);
        max_time = mktime(&tm);

        min_size = atoi(argv[4]);
        max_size = atoi(argv[5]);

        find_in_directory(argv[1], argv[2], min_size, max_size, min_time, max_time);   
        
    }
    else if (argc == 8 && !strcmp("-t", argv[3]))
    {
        min_timec = concat(concat(argv[4], " "), argv[5]);
        max_timec = concat(concat(argv[6], " "), argv[7]);
        strptime(min_timec, "%d.%m.%Y %R", &tm);
        min_time = mktime(&tm);
        strptime(max_timec, "%d.%m.%Y %R", &tm);
        max_time = mktime(&tm);

        find_in_directory(argv[1], argv[2], 0, INT_MAX, min_time, max_time);  
    }
    else if (argc == 6 && !strcmp("-s", argv[3]))
    {

        min_size = atoi(argv[4]);
        max_size = atoi(argv[5]);

        find_in_directory(argv[1], argv[2], min_size, max_size, 0, INT_MAX);  
    }
    else if (argc == 3)
    {
        find_in_directory(argv[1], argv[2], 0, INT_MAX, 0, INT_MAX);  
    }
    else
    {
        printf("Incorrect format\nEnter Path, Save file, size in bytes and modificated time\nExample: / file.txt -s 2 2000 -t 5.12.2000 00:00 6.9.2001 10:00\n");
        return 1;
    }
#else
    find_in_directory("/home/alex/Desktop/Lab2", "/home/alex/Desktop/Lab2/file.txt", min_size, max_size, min_time, max_time);   
#endif


    fclose(file);
    return 0;
}