#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>   // Importing the header files
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include<iostream>
#include <string.h>
#include<sys/stat.h>


int Status(int sock);  // Checking the status code 
int Header(int sock);  // parsing the header

int main()
{
char domain[] = "images.unsplash.com";  // specifying the domain name

int sock, bytes_received;  
        char send_data[1024],recv_data[1024], *p;
        struct sockaddr_in server_addr;
        struct hostent *he;
        
he = gethostbyname(domain);   // Getting the ip address of the host
        if (he == NULL){
           printf("Error in the host name\n");
           exit(1);
        }
        
if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1){
           printf("Error in creating the socket\n");
           exit(1);
        }
	// create a socket and bind it to the given address
        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(80);
        server_addr.sin_addr = *((struct in_addr *) he->h_addr);
        bzero(&(server_addr.sin_zero),8);
        
        if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
           printf("Error in connecting to the given server\n");
           exit(1); 
        }
        
        // created the connection
        
        FILE* file = fopen("input.txt", "r"); //reading the give input file
    struct stat sb;
    stat("input.txt",&sb);
    char *file_contents=(char*)malloc(sb.st_size);
    char counter[]="0";
    // Go through the links one by one in the input.txt file
    while ((fscanf(file,"%[^\n] ",file_contents)!=EOF)) { 
    
    counter[0]++;
    snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", file_contents, domain);
    
    // send the get request
    
    if(send(sock, send_data, strlen(send_data), 0)==-1){
            printf("Error in sending the request\n");
            exit(1); 
        }
        printf("Request for object sent\n");

        int contentlengh;
      // read the data till the content length  
      if(Status(sock) && (contentlengh=Header(sock))){

        int bytes=0;
        char img[25]="img";
        strcat(img,counter);
        strcat(img,".jpg");
       
        
        FILE* fd=fopen(img,"wb");
    
    	// Create the image file

        while(bytes_received = recv(sock, recv_data, 1024, 0))
        {
        
            if(bytes_received==-1){
                printf("Object cannot be received\n");
                exit(1);
            }
	fwrite(recv_data,1,bytes_received,fd);
            bytes+=bytes_received;
            if(bytes==contentlengh)
            break;
        }
        fclose(fd);
	// close the image
    
    }
    
    }
fclose(file);
// close the file

 close(sock);
 // Close the socket
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

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) break; // break end of request
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;

    sscanf(ptr,"%*s %d ", &status);

    printf("%s\n",ptr);
    printf("Request status=%d\n",status); // Print the status code
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
    printf("End HEADER ..\n"); // print length of the bytes received
    return  bytes_received ;

}

