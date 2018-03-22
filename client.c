// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <dirent.h>

void get(int sock, char* filename)
{
    char buffer[1024]={0};
    int size;
    //send(sock , filename , strlen(filename) , 0 );
    recv(sock, &size, sizeof(int), 0);
    if(!size)
    {
        printf("kya chutiyap\n");
        return;
    }
    int packets=size/1024;
    int last_packet=size%1024;
    printf("%d  %d\n",packets,last_packet);
    FILE *fp;

    int filehandle = open(filename, O_RDONLY);
    if(filehandle != -1)
    {
        int opti,rem;
        printf("here what to do\n1-overwrite\n2-skip\n3-new file with index\n");
        scanf("%d",&opti);
        if(opti==2)
            return;
        else if(opti==3)
        {
            int i=1;
            char finit[1024]={0}, *temp, fend[1024]={0};
            temp=strtok(filename,".");
            strcpy(finit,temp);
            printf("%s\n",finit);
            temp=strtok(NULL, "");
            strcpy(fend,temp);
            printf("%s\n", fend);
            while(filehandle!=-1)
            {
                char add[100];
                sprintf(add,"%d",i);
                strcpy(filename,finit);
                strcat(filename,add);
                strcat(filename,".");
                strcat(filename,fend);
                printf("%s\n",filename);
                i++;
                filehandle = open(filename, O_RDONLY);
            }
        }
    }
    close(filehandle);
    
    fp=fopen(filename,"w");
    for(int i=0;i<packets;i++)
    {
        recv(sock,buffer,1024,0);
        //printf("%s\n",buffer);
        fwrite(buffer,sizeof(char),1024,fp);
    }
    if(last_packet>0)
    {
        recv(sock,buffer,1024,0);
        //printf("%s\n",buffer);
        fwrite(buffer,sizeof(char),last_packet,fp);
    }
    fclose(fp);
    printf("download done\n");
}

void put(int sock, char* filename)
{
    struct stat obj;
    stat(filename, &obj);
    int filehandle=open(filename, O_RDONLY);
    int size = obj.st_size;
    char buffer[1024]={0};
    printf("%d %d\n",size,filehandle);
    send(sock,&filehandle,sizeof(int),0);
    if(filehandle==-1)
    {
        printf("no such file chut\n");
        return;
    }

    //send(sock , filename , strlen(filename) , 0 );
    int fh;
    recv(sock, &fh, sizeof(int), 0);
    printf("%d\n",fh);
    send(sock, &size, sizeof(int), 0);
    if(!size)
        return;
    int packets=size/1024;
    int last_packet=size%1024;
    
    if(fh!=-1)
    {
        printf("here what to do\n1-overwrite\n2-skip\n3-new file with index\n");
        int opti;
        scanf("%d",&opti);
        send(sock, &opti, sizeof(int), 0);
        if(opti==2)
            return;
    }
    FILE *fp=fopen(filename,"r");
    printf("%d %d\n",packets,last_packet);
    for(int i=0;i<packets;i++)
    {
        fread(buffer,sizeof(char),1024,fp);
        //printf("%s\n",buffer);
        send(sock,buffer,1024,0);
    }
    if(last_packet>0)
    {
        char buffer1[1024]={0};
        fread(buffer1,sizeof(char),last_packet,fp);
        //printf("%s\n",buffer1);
        send(sock,buffer1,1024,0);
    }
    fclose(fp);
    printf("upload done\n");
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    char* ip=argv[1];
    printf("%s\n",ip);
    int PORT=atoi(argv[2]);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    char input[1024];
    do
    {
        scanf("%s",input);
        send(sock , input , strlen(input) , 0 );
        if(!strcmp(input, "get"))
        {
            char filename[1024];
            scanf("%s",filename);
            send(sock , filename , 1024 , 0 );
            get(sock,filename);
        }
        else if(!strcmp(input,"put"))
        {
            char filename[1024]={0};
            scanf("%s",filename);
            send(sock , filename , 1024 , 0 );
            put(sock,filename);
        }
        else if(!strcmp(input,"mget"))
        {
            printf("which extensions?\n");
            char ext[50];
            scanf("%s",ext);
            send(sock,ext,50,0);
            while(1)
            {
                char fname[1024]={0};
                recv(sock, fname, 1024,0);
                printf("%s\n",fname);
                if(!strcmp(fname,"done"))
                    break;
                get(sock,fname);
            }
        }
        else if(!strcmp(input,"mput"))
        {
            struct dirent *de;
            DIR *dr=opendir(".");
            printf("which extensions?\n");
            char ext[50];
            scanf("%s",ext);
            while((de=readdir(dr))!=NULL)
            {
                char filename[1024]={0};
                strcpy(filename,de->d_name);
                int i=0,j,match=1;
                while(filename[i]!='.' && i<strlen(filename))
                    i++;
                i++;
                j=i;
                while(i<strlen(filename) && (i-j)<strlen(ext))
                {
                    if(filename[i]!=ext[i-j])
                        match=0;
                    i++;
                }
                if(match && i==strlen(filename) && (i-j)==strlen(ext))
                {
                    send(sock , filename , 1024 , 0 );
                    printf("%s\n", filename);
                    put(sock,filename);
                }
            }
            char filename[1024]={0};
            strcpy(filename,"done");
            send(sock , filename , 1024 , 0 );
            closedir(dr);
        }
    }
    while(strcmp(input,"quit"));
    printf("quit sent\n");
    return 0;
}