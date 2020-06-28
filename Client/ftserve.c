// Jason Lim
// Project 2 - File Transfer System Client - Server
// CS 372
// Last Modified: Nov 27, 2019
// Program Description: 2 connection client server network application for simple file transfer
// Will have to establish TCP control connections and wait for client to send command
// This C file is going to be our server to get data from ftclient and file transfer
// requests. Will only finish program once ftserver closes connection.
// ftserve.c (SERVER PROGRAM)
//Sources: referenced Lecture 4.3 of CS 344 Network Server.pdf mostly page 16-20 for server.c as base for main function
//Heavily used Beej's Guide and also borrowed some structure/modularizaiton and logic from functions from my Prog 1 C program
//looked over how this creator handles requests and client and server relations https://github.com/andykimmay/CS-372/blob/master/Project-2/ftserver.c
//referenced getFiles function in lines 180-204 https://github.com/benpauldev/OSU-CS372/blob/master/project_2/ftpserver.c
//Referenced structure and modularization as well as some logic from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
//Looked at logic and how they handle modularization in https://github.com/palmerja-osu/cs372/blob/master/project%202/ftserver.c


#define _POSIX_C_SOURCE 200809L //https://stackoverflow.com/questions/37541985/storage-size-of-addrinfo-isnt-known fixing hints address size
//also had to make sure SA_RESTART still worked so referenced https://stackoverflow.com/questions/9828733/sa-restart-not-defined-under-linux-compiles-fine-in-solaris
#define _GNU_SOURCE //had issue with DT checks https://stackoverflow.com/questions/5679267/how-to-resolve-reg-eip-undeclared-first-use-in-this-function-error-on-linux-3

#include <stdio.h>
#include <stdlib.h> //used some namespaces from https://beej.us/guide/bgnet/html/#a-simple-stream-server
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h> // needed to list files in directory
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h> //https://pubs.opengroup.org/onlinepubs/7908799/xns/arpainet.h.html

#define _XOPEN_SOURCE 600// https://stackoverflow.com/questions/7571870/netdb-h-not-linking-properly to fix AI_PASSIVE not found
#define BACKLOG 10   // connections to hold in backlog as per Beejs guide

// error
// Description: function for error reporting issues
// parameters: const char *msg (pointer char constant)
// Pre-Conditions: no error msg
// Post Conditions: shows error msg exists or not
void error(const char *msg) {
    perror(msg);
    exit(0);
} // Error function used for reporting issues #copied directly from server.c in CS 344 lecture 4.3

// sigchld_handler()
// Description: function to help errors will be used later in main
// parameters: None
// Pre-Conditions: if there is an error
// Post Conditions: reaps dead processes
//req from beej's https://beej.us/guide/bgnet/html/#a-simple-stream-server
void sigchld_handler()
{
    int errnoval = errno;  //copied this from above beej guide simple stream server
    while(waitpid(-1, NULL, WNOHANG) > 0); 
        errno = errnoval;
}

// srvAddresscreate(char* ipval, char* portnum)
// Description: creates the actual address given ip
// parameters: the ip address string as well as the portnum val
// Pre-Conditions: gather valid ip address and port num
// Post Conditions: ensures that the addr info is now correctly set up and valid
struct addrinfo* srvAddresscreate(char* ipval, char* portnum){      //from lines 5-17 http://beej.us/guide/bgnet/html/#getaddrinfoman
    int currval = 0;
    struct addrinfo hints;
    struct addrinfo * infoaddr;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET6 to force IPv6 (this was modified)
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my current IP address(this was modified)
	 
    if((currval = getaddrinfo(ipval, portnum, &hints, &infoaddr)) != 0){ //if there is an error
        fprintf(stderr, "Error: Must enter valid port\n",gai_strerror(currval)); //gai_strerror Returns zero on success, 
		//or non-zero on error. If the return value is non-zero, it can be passed to gai_strerror() to get a human-readable string. 
        exit(1);
    }
    return infoaddr; //return the address info
}

// socketCreation(int sockfd, struct addrinfo * infoaddr)
// Description: creates socket and checks to make sure theres no errors with creating and connecting socket
// parameters: grab sockfd and need the current infoaddr which we may obtain from prev function
// Pre-Conditions: gets the addrinfo and then returns a socket file descriptor
// Post Conditions: returns the sockfd value which means we created the socket, this is just a socket checker
void socketCreation(int sockfd, struct addrinfo * infoaddr){               //puts the values into socket based on arg
	int currentcondition; //test connection
	//returns the file socket descriptor or -1 and exit with a error
    if ((sockfd = socket((struct addrinfo *)(infoaddr)->ai_family, infoaddr->ai_socktype, infoaddr->ai_protocol)) == -1)
	{
        printf(stderr, "Error: Issue with creating socket.\n"); //referenced this lines 1-13 http://beej.us/guide/bgnet/html/#socketman
// as well as from lines 62-69 of https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c 
        exit(1);
    }
    if ((currentcondition = connect(sockfd, infoaddr->ai_addr, infoaddr->ai_addrlen)) == -1)
	{
        fprintf(stderr, "Error in connection.\n"); //error if the connection did not work
        exit(1);
    }
}


// gatherDirFiles(char** files)
// Description: determines the number of files in directory
// parameters: need the files double pointer for multiple file list as arg
// Pre-Conditions: grabs an array or list of files if they are valid while there are files in dir
// Post Conditions: returns the number of total files in directory
int gatherDirFiles(char** files){         
    struct dirent* dir;
	DIR* filedir;                                 //leveraged answer from stackoverflow to create this function https://stackoverflow.com/questions/11736060/how-to-read-all-files-in-a-folder-using-c
    filedir = opendir(".");
    int count = 0; //counter for determining file num
    if (filedir){
        while ((dir = readdir(filedir)) != NULL){     //while not null or there are still items available, this will iterate and read file names and put them into list and augment count
            if (dir->d_type == DT_REG)
            {
                strcpy(files[count], dir->d_name);  //looked over and modeled this function after lines since was not sure how to implement gathering files in directory folder: 129-147 of https://github.com/palmerja-osu/cs372/blob/master/project%202/ftserver.c
                count++;
            }
        }
        closedir(filedir); //close the opened directory 
    }
    return count; //return the count
}

// sendFileorList(char* command, char* ip, char * portnum, char * nameFile, char** filedir, int totalFiles)
// Description: this consolidated function sends files and sends the lists in directory based on list or get -l or -g flag 
// parameters: needs command value, the ip address, the port num, and name of file for -g, the filedir for -l, and total num of files for -l
// Pre-Conditions: needs all above parameters and make sure that they are valid commands before executing
// Post Conditions: will execute the send file over ftp or list file functionality as long as send and receive between sockets are valid
void sendFileorList(char* command, char* ip, char * portnum, char * nameFile, char** filedir, int totalFiles){                //This is what we'll use to send the file
   if (strcmp(command,"g") == 0)
   {
	   // sleep(3);
		struct addrinfo * infoaddr = srvAddinfoaddrscreate(ip, portnum);      // creates addr so we can use this later on
		int datasocket;
        socketCreation(datasocket, infoaddr);                                               //creates socket with addrinfo then connect it to server
		char tempbuffer[2000];     //Create a tempbuffer for file
		memset(tempbuffer, 0, sizeof(tempbuffer)); //clear with memset
		int fd = open(nameFile, O_RDONLY);                                                      //read only for file descriptor  https://linux.die.net/man/3/open
		while (1) {
			int totalbytes = 0;
            int totalbytesWritten = 0;
            totalbytes = read(fd, tempbuffer, sizeof(tempbuffer) - 1);                        //referenced for reading from fd from here: https://linux.die.net/man/2/read
			if (totalbytes < 0)
            {
				fprintf(stderr, "Error: File cannot be read. Something wrong has occurred. Negative bytes.\n");
				return;
			}
            if (totalbytes == 0)
				break; //no bytes
			
			void* temp = tempbuffer; //need a temporary file to add total bytes
			while (totalbytes > 0) {
                totalbytesWritten = send(datasocket, temp, sizeof(tempbuffer),0);
				if (totalbytesWritten < 0) {
					fprintf(stderr, "Error: There is an error in sending over file\n");
					return;
				}
				totalbytes -= totalbytesWritten;
				temp += totalbytesWritten;
			}
			memset(tempbuffer, 0, sizeof(tempbuffer));              //clear buffer
		}

		memset(tempbuffer, 0, sizeof(tempbuffer)); //ensures to clear buffer
		strcpy(tempbuffer, "complete"); //sends complete msg
		send(datasocket, tempbuffer, sizeof(tempbuffer),0); //sends this message to datasocket

		close(datasocket);          //close the socket and end client
		freeaddrinfo(infoaddr);   //frees created infoaddr pointer
	   
   }
   else{ //for list directory
	  //  sleep(3);
        int datasocket
        char* completed = "complete";
		struct addrinfo * infoaddr = srvAddinfoaddrscreate(ip, portnum);      //setup server addr
		socketCreation(datasocket, infoaddr);      //creates socket
		int i =0;
		for (i; i < totalFiles; i++){
			send(datasocket, filedir[i], 100 ,0);                 //Send for the total number of filedir
		}
		send(datasocket, completed, strlen(completed), 0); //sends over msg that its been complete

		close(datasocket); //listing done so close socket
		freeaddrinfo(infoaddr);  //free the addr
   }
}

// ftpAcceptClient(int fdNEW)
// Description: this consolidated function sends files and sends the lists in directory based on list or get -l or -g flag 
// parameters: need the files double poiner for multiple file list as arg
// Pre-Conditions: grabs an array or list of files if they are valid while there are files in dir
// Post Conditions: returns the number of total files in directory
void ftpAcceptClient(int fdNEW){	            //for this, referenced 209-285 for some guidance and structure from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
    char ip[300];
    char portnum[300];
    char command[500];
    char hostbuffer[256]; //https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
	char* pass = "pass";
    char* invalid = "invalid";
	int hostname;
    struct hostent *hostentry; // referenced link for next 15 lines: hostent, hostname and hostname checker https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
    memset(portnum, 0, sizeof(portnum));              //Make sure to clear before send
    recv(fdNEW, portnum, sizeof(portnum)-1, 0);  //receive portnum from client
    send(fdNEW, pass, strlen(pass), 0);   //send over if this portnum is valid or passed
    memset(command, 0, sizeof(command));           
    recv(fdNEW, command, sizeof(command)-1, 0);   // receive which command -l or -g
    send(fdNEW, pass, strlen(pass),0);  //send if this valid or passed
    memset(ip, 0, sizeof(ip));
    recv(fdNEW, ip, sizeof(ip)-1,0);   //receive over ip
    send(fdNEW, pass, strlen(pass),0);  //send if this valid or passed
    hostname = gethostname(hostbuffer, sizeof(hostbuffer)); //referenced man page for gethostname http://man7.org/linux/man-pages/man2/gethostname.2.html
	if (hostname == -1)  // Returns hostname for the local computer 
    { 
        perror("gethostname"); 
        exit(1); 
    } 
    hostentry = gethostbyname(hostbuffer); // Returns host information corresponding to host name 
	if (hostentry == 0)  //https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
    {  
        perror("gethostbyname"); 
        exit(1); 
    } 
    printf("Received connection from %s\n", hostbuffer); //print flipX received connection
    if(strcmp(command,"l") == 0){                  //Directory request so get the number of files and send them

       // send(fdNEW, pass, strlen(pass),0); //sends pass
        int filenamesize = 200;
        int tempsize = 500;
		char** temparr = malloc(tempsize*sizeof(char *));
		char filename[filenamesize];
        printf("List directory requested from client\n");
        printf("Sending directory contents list to %s \n", hostbuffer);
		int i;
		for(i = 0; i < tempsize; i++){
            temparr[i] = malloc(filenamesize*sizeof(char));  //creates temparr for each filename
            memset(temparr[i],0,sizeof(temparr[i]));
		}
		char** files = temparr;
        int filenum = gatherDirFiles(files);
		sendFileorList(command, ip, portnum, filename, files, filenum);
		int j=0;
		for (j; j < tempsize; j++)
		{  //free up the files in temparr
			free(files[j]);
		}
		free(files); //free up files at top
    }
	else if(strcmp(command, "g") == 0){              //if get command is true
      //  send(fdNEW, pass, strlen(pass), 0);
        int filenamesize = 200;
        int tempsize = 500;
        int validfile = 0;                //if file is found
        char filename[filenamesize];
        memset(filename, 0, sizeof(filename));
        recv(fdNEW, filename, sizeof(filename) - 1, 0); //receive the filename from client
        printf("File: %s requested from portnum: %s\n", filename, portnum);
		char** temparry = malloc(tempsize*sizeof(char *));
		int i;
		for(i = 0; i < tempsize; i++){
            temparry[i] = malloc(filenamesize*sizeof(char)); //create temparr each index
            memset(temparry[i],0,sizeof(temparry[i])); //cleanse it and make sure no items in it
		}
		char** files = temparry;
        int filenum = gatherDirFiles(files);         //Use the function to check if the file is there
		int k=0;
		for (k; k < filenum; k++){             //Loop through all the files to check the name
			if(strcmp(files[k], filename) == 0)  // once we have found the file name in dir
            {
				validfile = 1; //set that we found the file
                printf("File found, sending %s to client\n", filename);
                char filename[100];
                char * validfile = "valid";
                send(fdNEW, validfile, strlen(validfile), 0);
                memset(filename, 0, sizeof(filename));  //referenced lines 249-253 for this part from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
                strcpy(filename, "./");  //copy over 
                char* strfile = filename + strlen(filename);
                strfile += sprintf(strfile, "%s", filename);
                sendFileorList(command, ip, portnum, filename, files, filenum);
			}
        }
        if (validfile = 0){
            printf("Server unable to find file. Sending error message over to client.\n");
            char * invalidfile = "notavailable";
            send(fdNEW, invalidfile, tempsize, 0);      //send value that could not find file over to client
        }
		int l=0;
		for (l; l < tempsize; l++)
		{
			free(files[l]); //free file to maintain mem
		}
		free(files);
    }
    else{
        send(fdNEW, invalid, strlen(invalid), 0);
        printf("Please send a valid command. Try again\n"); //invalid command
    }

   // printf("The server is now awaiting new client connections.\n"); //new connections to await
}


// startup(struct sockaddr_in address, int portnum)
// Description: starts our socket as initiator
// parameters: needs sockaddr_in and portnum vals
// Pre-Conditions: obtains the address port and portnum first to know what to bind and listen for
// Post Conditions: creates soket binds it and listens for any clients to server
void startup(struct sockaddr_in address, int portnum){
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);  //referenced CS 344 lecture slides for this function
    int option = 1;
    address.sin_family = AF_INET;
    address.sin_port = htons(portnum);
    address.sin_addr.s_addr = INADDR_ANY;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // https://linux.die.net/man/2/setsockopt
    bind(listenSocket, (struct sockaddr *) &address, sizeof((address)));
    listen(listenSocket, 10);
}


// handlerequest(int sock_fd)
// Description: handles the requests for waiting (backlog is 10 so we can have up to 10)
// parameters: takes the sock file descriptor as input
// Pre-Conditions: a valid sock fd connection made
// Post Conditions: returns the sockfd and then closes client fd, can only handle up to 10 otherwise returns error
void handlerequest(int sockfd){	              
    int sizeofsocket;
	listen(sockfd, 1); //leveraged stackoverflow answer from https://stackoverflow.com/questions/55042257/accept-function-doesnt-wait-for-connectiondoesnt-block
    while (1) {
        int clientfd; //client socket
        clientfd = accept(sockfd, NULL, NULL);
        if (clientfd == -1) {
            printf("No more new connections can be made to server"); //referenced this for coding wait for connections https://stackoverflow.com/questions/55042257/accept-function-doesnt-wait-for-connectiondoesnt-block
            exit(0);
        }
		ftpAcceptClient(clientfd);
        printf("The server is now awaiting new client connections.\n"); //new connections to await
        close(clientfd);
    }
}

//this is our main function
//Description: runs and executes C program
// parameters: argc and argv which we receive from command prompt, will validate the program executing and port num as second var
// Pre-conditions: needs two args only and an executable program and valid port
// Post Conditions: returns executed ftp server
//Resource used: copied from beej's guide lines 44-133 and modified it to match reqs in server.c ex) https://beej.us/guide/bgnet/html/#a-simple-stream-server
int main(int argc, char *argv[])
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd all taken from https://beej.us/guide/bgnet/html/#a-simple-stream-server
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int valid = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    struct sockaddr_in address;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my current IP
    int portMAIN = *argv[1];
    if (argc != 2)
    {
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); //if there are not 2 args, we only want 2 arguments
        exit(0);
    } 
	if (atoi(argv[1]) < 1024 or atoi(argv[1] > 65535)
	{
		//need root permissions to bind less than 1024 so include 1024 between 65535 as valid (read this via slack CS372-400 11/22/19)
        fprintf(stderr,"Must have valid server connect port between 1024-65535");// Check usage & args
	}
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("server: socket error"); //error with socket  
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { //bind socket 
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
	freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n"); //if failed to bind
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) { 
        perror("listening error");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap dead proccesses
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; //used beej's guide for this  lines 89-102 https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction error");
        exit(1);
    }
    printf("Server is now open on port: %s\n", argv[1]);
    startup(address, portMAIN);
    handlerequest(sockfd);
    return 0;
}

