#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>

#define BUF_SIZE 4096

int message_size = 0;

int find(char *src, char *m)
{
    unsigned int ifd, ofd, rcnt;
    char buf[BUF_SIZE];
    struct stat stat_buf, stat_src;
    mode_t mode = 0;
    int rval;
    int find_count = 0;

    rval = access(src, R_OK);
    if (rval != 0)
    {
        fprintf(stderr, "Cannot read file\n");
        exit(0);
    }
    ifd = open(src, O_RDONLY);
    if (ifd == 0)
    {
        fprintf(stderr, "Cannot read file\n");
        exit(0);
    }

    char *buffer = malloc(message_size + 1);
    while (rcnt = read(ifd, buf, BUF_SIZE))
    {
        for (int i = 0; i < rcnt - message_size; i++)
        {
            for (int j = 0; j < message_size; j++)
            {
                buffer[j] = (char)buf[j + i];
            }
            if (strcmp(m, buffer) == 0)
            {
                find_count++;
            }
        }
    }

    free(buffer);
    close(ifd);
    return find_count;
}

char *concat(char *s1, char *s2)
{

    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    char *result = malloc(len1 + len2 + 1);

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);

    return result;
}

#define xDEBUG

int main(int argc, char *argv[])
{

#ifndef DEBUG
    if (argc != 4 && atoi(argv[3]) < 1)
    {
        printf("Incorrect format\n");
        return 1;
    }
#endif

    int pid;
    DIR *dir;
    struct dirent *ent;
    int num_of_proc = 0;

#ifndef DEBUG
    char *src_directory = argv[1];
    char *m = argv[2];
    int max_proc = atoi(argv[3]);
#else
    //char *src_directory = "/etc";
    char *src_directory = "/home/alex/Desktop/lab/4";
    char *m = "ifconfig";
    int max_proc = 15;
#endif

    char *src_file;
    struct stat stat_src;
    int find_count;
    if (src_directory[strlen(src_directory) - 1] != '/')
        src_directory = concat(src_directory, "/");

    message_size = strlen(m);
    if ((dir = opendir(src_directory)) != NULL)
        while ((ent = readdir(dir)) != NULL)
        {
            if (opendir(concat(src_directory, ent->d_name)) == NULL)
            {
                src_file = concat(src_directory, ent->d_name);
                switch (pid = fork())
                {
                case -1:
                    perror("fork");
                    return -1;
                case 0:

                    stat(src_file, &stat_src);
                    find_count = find(src_file, m);
                    printf("\nPid: %i\nPath: %s\nViewed: %ld bytes\nFind: %d\n", getpid(), src_file, stat_src.st_size, find_count);
                    exit(0);
                    break;
                default:
                    num_of_proc++;
                    while (num_of_proc > max_proc)
                    {
                        wait(NULL);
                        num_of_proc--;
                    }
                    break;
                }
            }
        }

    closedir(dir);
    while (num_of_proc > 0)
    {
        wait(NULL);
        num_of_proc--;
    }

    return 0;
}