#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_PATH_LEN 256
#define MAX_ABS_PATH 257
#define MAX_LAST_CHARS 10

char *permissions(struct stat inode)
{   /// function used for getting the permissions for the user, group and other
    /// I tried making static allocated char variable, but it couldn't compile well, so I chose a dynamic one
    char *perm = (char*) malloc(MAX_LAST_CHARS * sizeof(char));
    int i = 0;
    if (inode.st_mode & S_IRUSR)	 	
        perm[i++] = 'r';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IWUSR)		
        perm[i++] = 'w';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IXUSR)		
        perm[i++] = 'x';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IRGRP)		
        perm[i++] = 'r';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IWGRP)		
        perm[i++] = 'w';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IXGRP)		
        perm[i++] = 'x';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IROTH)		
        perm[i++] = 'r';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IWOTH)		
        perm[i++] = 'w';
    else
        perm[i++] = '-';

    if (inode.st_mode & S_IXOTH)		
        perm[i++] = 'x';
    else
        perm[i++] = '-';

    perm[i] = '\0';
    return perm;
}

void parse(char* filePath)
{   ///a function for reading the contents of a SF
    int fd = open(filePath, O_RDONLY);
    int version = 0; //between 123 and 158
    char magic, name[MAX_PATH_LEN][MAX_PATH_LEN]; // must be "u" and name is for the section name
    int x = 0, j; //x used for nb of sections
    int fileSize = 0; //used for header size
    int size[MAX_PATH_LEN], type[MAX_PATH_LEN], offSet[MAX_PATH_LEN]; //used for the fields in the section header

    lseek(fd, 0, SEEK_SET); //placing us at the beginning of the file

    read(fd, &magic, 1); //read magic
    if(magic != 'u')
    {
        printf("ERROR\nwrong magic\n");
        exit(-1);
    }
    
    read(fd, &fileSize, 2); //read the header size

    read(fd, &version, 1); //read version
    if(version < 123 || version > 158)
    {
        printf("ERROR\nwrong version\n");
        exit(-1);
    }
    
    read(fd, &x, 1); //read the nb of sectons
    if(x < 5 || x > 11)
    {
        printf("ERROR\nwrong sect_nr\n");
        exit(-1); 
    }

    for(j = 0; j < x; j++) //for used for reading the contens of each section
    {
        read(fd, &name[j], 8); //read name
        name[j][8] = '\0';
        read(fd, &type[j], 2); //read type
        if(type[j] != 30 && type[j] != 34 && type[j] != 63 && type[j] != 19)
        {
            printf("ERROR\nwrong sect_types\n");
            exit(-1);
        }
        read(fd, &offSet[j], 4); //read offset
        read(fd, &size[j], 4); //read size
        
    }
    //make the final successful print if the validations are met
    printf("SUCCESS\nversion=%d\nnr_sections=%d\n", version, x);
    j = x;
    for(x = 0; x < j; x++)
    {
        printf("section%d: %s %d %d\n", x + 1, name[x], type[x], size[x]);
    }
}
 int search_tree(char *dir_name, int rec, int perm, char perm_string[], int seq, char string[], int print)
{   ///used for listing the files/directories recursive or non-recursive, depending on a variable rec
    DIR *dir;
    struct stat inode;
    struct dirent *dir_ent;
    int seqLen = strlen(string);
    char name[MAX_PATH_LEN], *permission; //dynamic allocation for permission

    dir = opendir(dir_name);
    if(dir == 0)
    {
        printf("ERROR\ninvalid directory path\n");
        exit(4);
    }
    if(print == 1)
    {
        printf("SUCCESS\n");
    }
        while((dir_ent = readdir(dir)) != 0)
        {
            if(rec == 0) //used for non-recursive call
            {   
                if(strcmp(dir_ent->d_name, ".") != 0 && strcmp(dir_ent->d_name, "..") != 0)
                {
                    snprintf(name, MAX_ABS_PATH, "%s/%s", dir_name, dir_ent->d_name);
                    lstat(name, &inode); //get the info
                    
                    if(perm) //used for checking the permission of both directories or files
                    {
                        permission = permissions(inode);
                        if(strcmp(permission, perm_string) == 0)
                            printf("%s\n", name);
                        free(permission); //deallocation
                    }
                        else if(seq != 1) {printf("%s\n", name);}

                    if(seq) //used for checking the end sequence of both directories or files
                    {
                        char lastChars[MAX_LAST_CHARS];
                        strcpy(lastChars, dir_ent->d_name + strlen(dir_ent->d_name) - seqLen); //copy the last chars into the variable
                        if(strcmp(string, lastChars) == 0)
                            printf("%s\n", name);
                    }
                }
            }
            else //used for recursive call
            {   
                if(strcmp(dir_ent->d_name, ".") != 0 && strcmp(dir_ent->d_name, "..") != 0)
                {
                    snprintf(name, MAX_ABS_PATH, "%s/%s", dir_name, dir_ent->d_name);
                    lstat(name, &inode);
                    
                    if(perm) 
                    {
                        permission = permissions(inode);
                        if(strcmp(permission, perm_string) == 0)
                            printf("%s\n", name);
                       free(permission);
                    }
                        else if(seq != 1) {printf("%s\n", name);}
                    
                    if(seq)
                    {
                        char lastChars[MAX_LAST_CHARS];
                        strcpy(lastChars, dir_ent->d_name + strlen(dir_ent->d_name) - seqLen);
                        if(strcmp(string, lastChars) == 0)
                            printf("%s\n", name);
                    }
                    
                    if(S_ISDIR(inode.st_mode))
                    {
                        search_tree(name, 1, perm, perm_string, seq, string, 0);
                    }
                }
            }
            
        }
    free(dir_ent);
    closedir(dir);
    return 0;
} 

int findall(char *dir_name, int print)
{
    DIR *dir;
    struct stat inode;
    struct dirent *dir_ent;
    char name[MAX_PATH_LEN];
    
    
    dir = opendir(dir_name);
    
    if(dir == 0)
    {
        printf("ERROR\ninvalid directory path\n");
        exit(4);
    }
    
    if(print == 1)
    {
        printf("SUCCESS\n");
    }

    while((dir_ent = readdir(dir)) != 0)
    {
        if(strcmp(dir_ent->d_name, ".") != 0 && strcmp(dir_ent->d_name, "..") != 0)
        {
            snprintf(name, MAX_ABS_PATH, "%s/%s", dir_name, dir_ent->d_name);
            lstat(name, &inode);
            
            if(S_ISREG(inode.st_mode))
            {
                int fd = open(name, O_RDONLY);
                int version = 0; //between 123 and 158
                char magic, name1[MAX_PATH_LEN][MAX_PATH_LEN]; // must be "u" and name is for the section name
                int x = 0, j; //x used for nb of sections
                int fileSize = 0; //used for header size
                int size[MAX_PATH_LEN], type[MAX_PATH_LEN], offSet[MAX_PATH_LEN]; //used for the fields in the section header
                int maxx = 0, ok = 1;

                lseek(fd, 0, SEEK_SET); //placing us at the beginning of the file

                read(fd, &magic, 1); //read magic
                if(magic != 'u'){ ok = 0;}
                
                read(fd, &fileSize, 2); //read the header size

                read(fd, &version, 1); //read version
                if(version < 123 || version > 158)
                {
                    ok = 0;
                }
                
                read(fd, &x, 1); //read the nb of sectons
                if(x < 5 || x > 11)
                {
                    ok = 0;
                }

                for(j = 0; j < x; j++)
                {
                    read(fd, &name1[j], 8); //read name
                    name1[j][8] = '\0';
                    read(fd, &type[j], 2); //read type
                    if(type[j] != 30 && type[j] != 34 && type[j] != 63 && type[j] != 19)
                    {
                        ok = 0;
                    }
                    read(fd, &offSet[j], 4); //read offset
                    read(fd, &size[j], 4); //read size
                    
                    if(size[j] > maxx) maxx = size[j];//max SF size saved in a variable
                }
                if(maxx <= 1148 && ok == 1) printf("%s\n", name);
            }   
            if(S_ISDIR(inode.st_mode))
            {
                findall(name, 0);
            }
        }
    }
    free(dir_ent);
    closedir(dir);
    return 0;
}

int main(int argc, char **argv)
{   
    int perm = 0;
    int rec = 0, seq = 0;
    char perm_string[MAX_LAST_CHARS];
    char string[MAX_LAST_CHARS];
    
    if(argc >= 2)
    {
        if(strcmp(argv[1], "variant") == 0)
        {
            printf("23353\n");
        }
            else
            {
                for(int i = 1; i < argc; i++)
                {
                    if(strcmp(argv[i], "list") == 0)
                    {
                        char path[MAX_PATH_LEN];
                        path[0] = '\0';
                        for(int j = 1; j < argc; j++)
                        {
                            if(strstr(argv[j], "path="))
                            {   
                                strcpy(path, argv[j] + 5);
                                path[strlen(path)] = '\0';  
                            }
                            
                            if(strcmp(argv[j], "recursive") == 0)
                            {
                                rec = 1;
                            }
                            
                            else if(strstr(argv[j], "name_ends_with="))
                            {   
                                seq = 1;
                                strcpy(string, argv[j] + 15);  
                            }
                            else if(strstr(argv[j], "permissions="))
                            {
                                perm = 1;
                                strcpy(perm_string, argv[j] + 12);
                            }
                        }
                        
                        search_tree(path, rec, perm, perm_string, seq, string, 1);
                    }
                    else if(strcmp(argv[i], "parse") == 0) 
                    {
                        for(int j = 1; j < argc; j++)
                        {   
                            char path[MAX_PATH_LEN];
                            if(strstr(argv[j], "path="))
                            {   
                                strcpy(path, argv[j] + 5);
                                path[strlen(path)] = '\0';
                                parse(path);   
                            }
                        }
                    }
                    else if(strcmp(argv[i], "findall") == 0) 
                        {
                            for(int j = 1; j < argc; j++)
                            {   
                                char path[MAX_PATH_LEN];
                                if(strstr(argv[j], "path="))
                                {   
                                    strcpy(path, argv[j] + 5);
                                    path[strlen(path)] = '\0';
                                    findall(path, 1);   
                                }
                            }

                        }
                    
                }
            } 
    }
    return 0;
}