// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <dirent.h>

void get(int new_socket, char* fname)
{
    struct stat obj;
    stat(fname, &obj);
    char buffer[1024];
    int filehandle=open(fname, O_RDONLY);
    int size = obj.st_size;
    printf("%d %d\n",size,filehandle);
    if(filehandle==-1)
        size=0;
    send(new_socket, &size, sizeof(int), 0);
    int packets=size/1024;
    int last_packet=size%1024;
    printf("%d    %d\n",packets,last_packet);
    if(size)
    {
        FILE *fp;
        fp=fopen(fname,"r");
        for(int i=0;i<packets;i++)
        {
            fread(buffer,sizeof(char),1024,fp);
            //printf("%s\n",buffer);
            send(new_socket,buffer,1024,0);
        }
        if(last_packet>0)
        {
            fread(buffer,sizeof(char),last_packet,fp);
            //printf("%s\n",buffer);
            send(new_socket,buffer,1024,0);
        }
        fclose(fp);
        printf("upload done\n");
    }
    //     sendfile(new_socket, filehandle, NULL, size);
}

void put(int new_socket, char* fname)
{
    struct stat obj;
    int filehandle,fh;
    recv(new_socket,&fh,sizeof(int),0);
    if(fh==-1)
        return;
    filehandle=open(fname, O_RDONLY);
    send(new_socket, &filehandle, sizeof(int), 0);
    int size;
    recv(new_socket, &size, sizeof(int), 0);
    printf("%d, %d\n",filehandle,size);
    char buffer[1024]={0};
    if(!size)
        return;
    if(filehandle!=-1)
    {
        int opti;
        recv(new_socket, &opti, sizeof(int), 0);
        printf("%d\n",opti);
        if(opti==2)
            return;
        else if(opti==3)
        {
            int i=1;
            char finit[1024], *temp, fend[1024];
            temp=strtok(fname,".");
            strcpy(finit,temp);
            printf("%s\n",finit);
            temp=strtok(NULL, "");
            strcpy(fend,temp);
            printf("%s\n", fend);
            while(filehandle!=-1)
            {
                char add[100];
                sprintf(add,"%d",i);
                strcpy(fname,finit);
                strcat(fname,add);
                strcat(fname,".");
                strcat(fname,fend);
                printf("%s\n",fname);
                i++;
                filehandle = open(fname, O_RDONLY);
            }
        }
    }
    close(filehandle);
    
    int packets=size/1024;
    int last_packet=size%1024;
    FILE *fp;
    fp=fopen(fname,"w");
    for(int i=0;i<packets;i++)
    {
        recv(new_socket,buffer,1024,0);
        //printf("%s\n",buffer);
        fwrite(buffer,sizeof(char),1024,fp);
    }
    if(last_packet>0)
    {
        char buffer1[1024]={0};
        recv(new_socket,buffer1,1024,0);
        //printf("%s\n",buffer1);
        fwrite(buffer1,sizeof(char),last_packet,fp);
    }
    fclose(fp);
    printf("download done\n");

}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    struct stat obj;
    int PORT=atoi(argv[1]);
    printf("%d\n",PORT);
      
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    send(new_socket , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
    while(1)
    {
        char buffer1[1024]={0};
        valread = read( new_socket , buffer1, 1024);
        if(valread)
            printf("%s\n",buffer1);
        if(!strcmp(buffer1,"get") && valread)
        {
            char fname[1024]={0};
            recv(new_socket, fname, 1024,0);
            printf("%s\n",fname);
            get(new_socket,fname);
        }
        else if(!strcmp(buffer1,"put") && valread)
        {
            char fname[1024]={0};
            recv(new_socket, fname, 1024, 0);
            printf("%s\n",fname);
            put(new_socket,fname);
            printf("hey\n");
        }
        else if(!strcmp(buffer1,"mget") && valread)
        {
            struct dirent *de;
            DIR *dr=opendir(".");
            char ext[50];
            recv(new_socket,ext,50,0);
            printf("%s\n", ext);
            char fname[1024]={0};
            while((de=readdir(dr))!=NULL)
            {
                strcpy(fname,de->d_name);
                int i=0,j,match=1;
                while(fname[i]!='.' && i<strlen(fname))
                    i++;
                i++;
                j=i;
                while(i<strlen(fname) && (i-j)<strlen(ext))
                {
                    if(fname[i]!=ext[i-j])
                        match=0;
                    i++;
                }
                if(match && i==strlen(fname) && (i-j)==strlen(ext))
                {
                    printf("%s\n",fname);
                    send(new_socket,fname,1024,0);
                    get(new_socket,fname);
                }
            }
            strcpy(fname,"done");
            send(new_socket,fname,1024,0);
        }
        else if(!strcmp(buffer1,"mput") && valread)
        {
            while(1)
            {
                char fname[1024]={0};
                recv(new_socket, fname, 1024, 0);
                printf("%s\n",fname);
                if(!strcmp(fname,"done"))
                    break;
                put(new_socket,fname);
            }
        }
        else if(!strcmp(buffer1,"quit") && valread)
        {
            printf("quit recvd\n");
            return 0;
        }
    }
}