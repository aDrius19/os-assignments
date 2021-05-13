#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define RESP "RESP_PIPE_23353"
#define REQ "REQ_PIPE_23353"
#define SUCCESS "SUCCESS\n"
#define ERROR "ERROR"

char success = 7;
char error = 5;

int main(int argc, char *argv[])
{
    if(mkfifo(RESP, 0644) < 0)
    {
        printf("ERROR\ncannot create the response pipe");
    }

    int fdReq = open(REQ, O_RDONLY);
    if(fdReq < 0) 
    {
        printf("ERROR\ncannot open the request pipe");
    }

    int fdResp = open(RESP, O_WRONLY);
    if(fdResp < 0) 
    {
        printf("ERROR\ncannot open the response pipe");
    }
    
    char size = 7;
    write(fdResp, &size, sizeof(size));
    write(fdResp, "CONNECT", size);

    printf(SUCCESS);

    while(1)
    {
        char v[30];
        read(fdReq, &size, sizeof(size));
        read(fdReq, v, size);
        v[(int)size] = '\0';

        if(strcmp(v,"PING") == 0) 
        {
            char size1 = 4;
            int var = 23353;

            write(fdResp, &size1, sizeof(size1));
            write(fdResp, "PING", size1); 
            write(fdResp, &size1, sizeof(size1));
            write(fdResp, "PONG", size1);
            write(fdResp, &var, sizeof(var));
        }

        int shm_size;
        int shm_fd;
        if(strcmp(v,"CREATE_SHM") == 0)
        {
            char name[10] = "/TaerPC";
            char create = 10;
            shm_fd = -1;
            read(fdReq, &shm_size, sizeof(shm_size));
            
            shm_fd = shm_open(name, O_CREAT | O_RDWR, 0664);
            ftruncate(shm_fd, shm_size);

            if(shm_fd != 0)
            {
                write(fdResp, &create, sizeof(create));
                write(fdResp, "CREATE_SHM", create); 
                write(fdResp, &success, sizeof(success));
                write(fdResp, SUCCESS, success);
            }
            else
            {
                write(fdResp, &create, sizeof(create));
                write(fdResp, "CREATE_SHM", create); 
                write(fdResp, &error, sizeof(error));
                write(fdResp, ERROR, error);
            }
        }
        char* memory = NULL;
        if(strcmp(v,"WRITE_TO_SHM") == 0)
        {
            char writte = 12;
            unsigned int val = 0, offset = 0;

            read(fdReq, &offset, sizeof(offset));
            read(fdReq, &val, sizeof(val));

            if(offset > 0 && offset < 4236908 && offset + 4 < shm_size)
            {
                memory = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
                memcpy(memory + offset, &val, sizeof(val));
                write(fdResp, &writte, sizeof(writte));
                write(fdResp, "WRITE_TO_SHM", writte); 
                write(fdResp, &success, sizeof(success));
                write(fdResp, SUCCESS, success);
            }
            else
            {
                write(fdResp, &writte, sizeof(writte));
                write(fdResp, "WRITE_TO_SHM", writte); 
                write(fdResp, &error, sizeof(error));
                write(fdResp, ERROR, error);
            }
        }

        char* map_file;
        struct stat file_size;
        if(strcmp(v,"MAP_FILE") == 0)
        {
            char s, mapp = 8, path[100];
            int fd;

            read(fdReq, &s, sizeof(s));
            read(fdReq, path, s);
            path[(int)s] = '\0';

            fd = open(path, O_RDONLY, 0644);
            fstat(fd, &file_size);

            if(fd > 0 && (map_file = mmap(NULL, file_size.st_size, PROT_READ, MAP_SHARED, fd, 0)) != MAP_FAILED)
            {
                write(fdResp, &mapp, sizeof(mapp));
                write(fdResp, "MAP_FILE", mapp); 
                write(fdResp, &success, sizeof(success));
                write(fdResp, SUCCESS, success);
            }
            else
            {
                write(fdResp, &mapp, sizeof(mapp));
                write(fdResp, "MAP_FILE", mapp); 
                write(fdResp, &error, sizeof(error));
                write(fdResp, ERROR, error);
            }
        }

        if(strcmp(v,"READ_FROM_FILE_OFFSET") == 0)
        {
            char read_offset = 21;
            int nb_of_bytes = 0, offset = 0;

            read(fdReq, &offset, sizeof(offset));
            read(fdReq, &nb_of_bytes, sizeof(nb_of_bytes));

            if((offset + nb_of_bytes) + 4 < file_size.st_size)
            {
                memory = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
                memcpy(memory, map_file + offset, nb_of_bytes);
                write(fdResp, &read_offset, sizeof(read_offset));
                write(fdResp, "READ_FROM_FILE_OFFSET", read_offset); 
                write(fdResp, &success, sizeof(success));
                write(fdResp, SUCCESS, success);
            }
            else
            {
                write(fdResp, &read_offset, sizeof(read_offset));
                write(fdResp, "READ_FROM_FILE_OFFSET", read_offset); 
                write(fdResp, &error, sizeof(error));
                write(fdResp, ERROR, error);
            }
        }

        if(strcmp(v,"READ_FROM_FILE_SECTION") == 0)
        {
            char read_section = 22;
            int section_nb = 0, nb_of_bytes = 0, offset = 0, section_offset = 0, nb_of_sections, section_size, OK = 1;
            
            read(fdReq, &section_nb, sizeof(section_nb));
            read(fdReq, &offset, sizeof(offset));
            read(fdReq, &nb_of_bytes, sizeof(nb_of_bytes));

            memcpy(&nb_of_sections, map_file + 4, 1);
            if(section_nb > nb_of_sections)
            {
                OK = 0;
                write(fdResp, &read_section, sizeof(read_section));
                write(fdResp, "READ_FROM_FILE_SECTION", read_section); 
                write(fdResp, &error, sizeof(error));
                write(fdResp, ERROR, error);
            }
            
            int section = 5 + 18 * (section_nb - 1) + 10;

            memcpy(&section_offset, map_file + section, 4);
            memcpy(&section_size, map_file + section + 4, 4);
            
            if(OK == 1)
            {
                if(section_size >= offset + nb_of_bytes)
                {
                    memory = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
                    memcpy(memory, map_file + section_offset + offset, nb_of_bytes);
                    write(fdResp, &read_section, sizeof(read_section));
                    write(fdResp, "READ_FROM_FILE_SECTION", read_section); 
                    write(fdResp, &success, sizeof(success));
                    write(fdResp, SUCCESS, success);
                }
                else
                {
                    write(fdResp, &read_section, sizeof(read_section));
                    write(fdResp, "READ_FROM_FILE_SECTION", read_section); 
                    write(fdResp, &error, sizeof(error));
                    write(fdResp, ERROR, error);
                }
            }
        }

        if(strcmp(v,"EXIT") == 0) 
        {
            close(fdReq);
            close(fdResp);
            shm_unlink(memory);
            break;
        }
    }
    return 0;
}