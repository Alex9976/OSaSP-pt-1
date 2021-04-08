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

#define BUF_SIZE 1024

int copy(char *src, char *dest)
{
    unsigned int ifd, ofd, rcnt;
    char buf[BUF_SIZE];
    struct stat stat_buf, stat_src;
    mode_t mode = 0;
    int rval;

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

    ofd = open(dest, O_WRONLY | O_CREAT);
    if (ofd == NULL)
    {
        fprintf(stderr, "Cannot open output file\n");
        exit(0);
    }

    while (rcnt = read(ifd, buf, BUF_SIZE))
    {
        write(ofd, buf, rcnt);
    }

    stat(src, &stat_src);
    mode = stat_src.st_mode;

    chmod(dest, mode);
    stat(dest, &stat_buf);

    close(ifd);
    close(ofd);
    return stat_buf.st_size;
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
    char *dest_directory = argv[2];
    int max_proc = atoi(argv[3]);
#else
    char *src_directory = "/etc";
    char *dest_directory = "/home/alex/Desktop/Dir2/";
    int max_proc = 5;
#endif

    char *src_file, *dest_file;

    if (src_directory[strlen(src_directory) - 1] != '/')
        src_directory = concat(src_directory, "/");
    if (dest_directory[strlen(dest_directory) - 1] != '/')
        dest_directory = concat(dest_directory, "/");

    if ((dir = opendir(src_directory)) != NULL)
        while ((ent = readdir(dir)) != NULL)
        {
            if (opendir(concat(src_directory, ent->d_name)) == NULL)
            {
                src_file = concat(src_directory, ent->d_name);
                dest_file = concat(dest_directory, ent->d_name);
                if (fopen(dest_file, "r") == NULL)
                {
                    switch (pid = fork())
                    {
                    case -1:
                        perror("fork");
                        return -1;
                    case 0:
                        printf("\nPid: %i\nPath: %s\nCopied: %d bytes\n", getpid(), src_file, copy(src_file, dest_file));
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
        }

    closedir(dir);
    while (num_of_proc > 0)
    {
        wait(NULL);
        num_of_proc--;
    }

    return 0;
}