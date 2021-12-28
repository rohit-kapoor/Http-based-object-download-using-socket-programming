#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>      // Importing the header files
#include <errno.h>
#include <arpa/inet.h>
#include<iostream>
#include <string.h>
#include<sys/stat.h>

int Status(int sock);
int Header(int sock);



int main()
{
    FILE* file = fopen("input.txt", "r"); //reading the give input file
    struct stat sb;
    stat("input.txt",&sb);
    char *file_contents=(char*)malloc(sb.st_size);
    char counter[]="0";

    while ((fscanf(file,"%[^\n] ",file_contents)!=EOF)) {       
        // The loop will keep on creating connections till we have links 
        // in the input.txt file
        counter[0]++;
        char domain[] = "images.unsplash.com";
         // Specifying the base domain and path of the object

        int sock, bytes_received;  
        char send_data[1024],recv_data[1024], *p;
        struct sockaddr_in server_addr;
        struct hostent *he;


        he = gethostbyname(domain);
        if (he == NULL){
           printf("Error in the host name\n");
           exit(1);
        }
        
        // To get the ip address of the domain specified

        if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1){
           printf("Error in creating the socket\n");
           exit(1);
        } 
        
        // create the socket exit if there is error in creating the 
        // socket

        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(80);
        server_addr.sin_addr = *((struct in_addr *) he->h_addr);
        bzero(&(server_addr.sin_zero),8);

	// Bind the socket to the given address
	
        if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
           printf("Error in connecting to the given server\n");
           exit(1); 
        }

	// establish the connection
	
        snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", file_contents, domain);
	
	// Send the get request to obtain the object
	
        if(send(sock, send_data, strlen(send_data), 0)==-1){
            printf("Error in sending the request\n");
            exit(1); 
        }
        printf("Request for object sent\n");

        int contentlengh;

    if(Status(sock) && (contentlengh=Header(sock))){

        int bytes=0;
        char img[25]="img";
        
        strcat(img,counter);
        strcat(img,".jpg");
       // counter for naming the image
        
        FILE* fd=fopen(img,"wb");
    	
    	// open the file

        while(bytes_received = recv(sock, recv_data, 1024, 0)){
            if(bytes_received==-1){
                printf("Object cannot be received\n");
                exit(1);
            }
	// Start receving the data in the loop and write onto the image

            fwrite(recv_data,1,bytes_received,fd);
            bytes+=bytes_received;
            if(bytes==contentlengh) // Break when bytes is equal to conent 
            break;                  // length
        }
        fclose(fd);
        // close the image
    }


	// Close the socket
    close(sock);

            
    
    }
    

    fclose(file);

    return 0;
}

int Status(int sock){
    char c;
    char buff[1024]="",*ptr=buff+1;
    int bytes_received, status;
    while(bytes_received = recv(sock, ptr, 1, 0)){
        if(bytes_received==-1){
            printf("No data received from the server\n");
            exit(1);
        }

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;

    sscanf(ptr,"%*s %d ", &status);
    // print the status of the request

    printf("%s\n",ptr);
    printf("Request status=%d\n",status);
    return (bytes_received>0)?status:0;

}

int Header(int sock){
    char c;
    char buff[1024]="",*ptr=buff+4;
    int bytes_received, status;
    printf("Begin HEADER ..\n");
    while(bytes_received = recv(sock, ptr, 1, 0)){
        if(bytes_received==-1){
            perror("Parse Header");
            exit(1);
        }
	// If not bytes recieved the exit
        if(
            (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
            (ptr[-1]=='\r')  && (*ptr=='\n' )
        ) break;
        ptr++;
    }

    *ptr=0;
    ptr=buff+4;


    if(bytes_received){
        ptr=strstr(ptr,"Content-Length:");
        if(ptr){
            sscanf(ptr,"%*s %d",&bytes_received);

        }else
            bytes_received=-1; //unknown size

       printf("Content-Length: %d\n",bytes_received);
    }
    printf("End HEADER ..\n");
    return  bytes_received ;

}

