#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h> 
#include <string.h>
#include <dirent.h>

int num_of_directories = 0;
int num_of_files = 0;

void print_info(char *path);

char* concat(char *s1, char *s2) {

    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);                      

    char *result = malloc(len1 + len2 + 1);

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);    

    return result;
}

void find_in_directory(char *directory, char *search_name)
{
    DIR *dir;
    struct dirent *ent;
    if (directory[strlen(directory) - 1] != '/')
        directory = concat(directory, "/");
    if ((dir = opendir(directory)) != NULL) 
        while ((ent = readdir(dir)) != NULL) 
        {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, ".."))
                if (opendir(concat(directory, ent->d_name)) != NULL)
                {
                    num_of_directories++;
                    find_in_directory(concat(directory, ent->d_name), search_name);
                }
                else
                    num_of_files++;
                    
            if (!strcmp(ent->d_name, search_name))
            {
                printf("Path: %s\n", directory);
                print_info(concat(directory, ent->d_name));
            }
        }   
    closedir(dir);

}

void print_info(char *path)
{
    struct stat buf = { 0 };
    char acesss_rights[9];
    int i;
    stat(path, &buf);
    struct tm * timeinfo;

    printf("Size: %d bytes\n", buf.st_size);

    timeinfo = localtime (&buf.st_mtime);
    printf("Modificated time: %s", asctime(timeinfo));

    for (i = 0; i < 9; i += 3)
    {
        acesss_rights[8 - i] = buf.st_mode & (0x1 << i) ? 'x' : '-';
        acesss_rights[8 - (i + 1)] = buf.st_mode & (0x1 << (i + 1)) ?  'w' : '-';
        acesss_rights[8 - (i + 2)] = buf.st_mode & (0x1 << (i + 2)) ?  'r' : '-';
    }
    printf("Acess rights: %s\n", acesss_rights);

    printf("Inode: %d\n\n", buf.st_ino);

}

int main(int argc, char *argv[])
{   
    if (argc == 3)
    {
        find_in_directory(argv[1], argv[2]);  
        //find_in_directory("/home/alex/", "file.txt");   
        printf("Viewed directories: %d\nViewed files: %d\n", num_of_directories, num_of_files);   
    }
    else
    {
        printf("Incorrect format\n");
        return 1;
    }
    return 0;
}