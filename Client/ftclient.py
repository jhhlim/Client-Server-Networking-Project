#!/usr/bin/env python
# Jason Lim
# Project 2 - FTP Client / Server
# CS 372
# Last Modified: Nov 28, 2019
# Program Description: 2 connection client server network application for simple file transfer 
# Will have to establish TCP control connections and wait for client to send command
# This python file is going to be our client to get data from ftserver and file transfer
# requests. Will only finish program once ftserver closes connection.
# ftclient.py (CLIENT PROGRAM)

#Sources: Taken from Lecture 15 CS 372
#https://docs.python.org/2/library/struct.html for struct
#Other Sources include: https://docs.python.org/release/2.6.5/library/socket.html
#https://wiki.python.org/moin/TcpCommunication
#Lecture 15: Socket Programming Primer slide 9 TCP Server in Python
#borrowed send(msg) lines 57-61 structure from https://github.com/andykimmay/CS-372/blob/master/Project-1/chatserver.py
#borrowed some parts of validation ideas from https://github.com/benpauldev/OSU-CS372/blob/master/project_2/ftpclient.py
#also referenced request and get data functions from https://github.com/TylerC10/CS372/blob/master/Project%202/ftclient.py
#referenced logic from https://github.com/palmerja-osu/cs372/blob/master/project%202/ftclient.py

#Server has at least functions which perform: Start-up, HandleRequest || Client has at least functions which perform: Initiate Contact, MakeRequest, ReceiveData

from socket import *
import sys
import shutil #https://www.geeksforgeeks.org/python-shutil-copyfile-method/ for dupe handling
import os
from struct import *
from time import sleep #https://pythonspot.com/sleep/
import re #importing reg expressions for validation (are port values all numerical/integer?)

#argument validation
#as per readme.txt
#borrowed some parts of validation ideas from https://github.com/benpauldev/OSU-CS372/blob/master/project_2/ftpclient.py, fairly basic but I agreed with most of these validations in
# this resource
#python ftclient.py flip1.engr.oregonstate.edu 15300 -l 30050 (to list what is in dataport 30050)
#python ftclient.py flip1.engr.oregonstate.edu 15300 -g file.txt 30051 (to receive/get what is in dataport 30051)

#intchecker(strg)
# Description: function to check regex
# parameters: takes a string to check using reg ex if int
# Pre-Conditions: gets string value from sys.argv input into console
# Post Conditions: checks and returns true or false if all values in string were digits [0-9]
def intchecker(strg): #https://stackoverflow.com/questions/1265665/how-can-i-check-if-a-string-represents-an-int-without-using-try-except checking string if all ints (for port)
    return re.match(r"0-9", strg) is not None #https://docs.python.org/2/library/re.html

	
# argvalidation()
# Description: funciton to validate command prompt on client side
# parameters: None
# Pre-Conditions: checks arguments on command prompt or console to ensure everything is validated 
# Post Conditions: returns error message if invalid command otherwise no errors printed and the command will execute
def argvalidation():
    if (sys.argv[3] == "-l"):
        if (intchecker(sys.argv[4])):#need to check if port vals are int
            print("Port needs to be an integer value")
            exit(0)
    if (sys.argv[3] == "-g"):
        if (intchecker(sys.argv[5])):
            print("Port needs to be an integer value") #need to check if port vals are int
            exit(0)
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print("Need to have at least 5 or no more than 6 arguments in command") #base condition, checks num of args
        exit(0)
    if(sys.argv[1] != "flip1" and sys.argv[1] != "flip1.engr.oregonstate.edu"): #need valid flipX value, can use just the flipX instead of full path with engr.oregonstate.edu
        if (sys.argv[1] != "flip2" and sys.argv[1] != "flip2.engr.oregonstate.edu"):
            if (sys.argv[1] != "flip3" and sys.argv[1] != "flip3.engr.oregonstate.edu"):
                print("Must be on a valid flip such as flip1,flip2,or flip3")
                exit(0)
    if(sys.argv[3] != "-l" and sys.argv[3] !="-g"):
        print("Need to have the correct flag such as -l for list files and -g for get files")
        exit(0)
    if (int(sys.argv[2]) < 1024 or int(sys.argv[2]) > 65535): #need root permissions to bind less than 1024 so include 1024 between 65535 as valid (read this via slack CS372-400 11/22/19)
        print("Must have valid server connect port between 1024-65535")
        exit(0)
    if (sys.argv[3] == "-l"):
        if(int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024): #need root permissions to bind less than 1024 so include 1024 between 65535 as valid (read this via slack CS372-400 11/22/19)
            print("Must have data port between 1024-65535 to know which directory to look for to list files")
            exit(0)
    if (sys.argv[3] == "-g"):
        if(int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024): #need root permissions to bind less than 1024 so include 1024 between 65535 as valid (read this via slack CS372-400 11/22/19)
            print("Must have data port between 1024-65535 to know which directory to look for to get file ")
            exit(0)

#initiatecontact(hostname, portnum)
# Description: initiates contact with server
# parameters: Takes the host name such as flipX and the portnum which we will connect to server on
# Pre-Conditions:Need port num and hostname to set up the socket 
# Post Conditions: Connects client to server and returns the clientsocket conection
def initiatecontact(hostname, portnum): #connect to server
    clientsocket = socket(AF_INET, SOCK_STREAM)
    clientsocket.connect((hostname, portnum)) #connects the clientsocket based on hostname flipX and portnum 1024-65535 value
    return clientsocket

# createdatasocket(filecommand)
# Description: creates datasocket based on file command
# parameters: filecommand whether it is a get or list value for sys.argv[3]
# Pre-Conditions: Receive data from the data connection overall makes request to connect to server
# Post Conditions: Create data socket for ftp, client side supports 2 port nums for connecting to server for CONTROL and receiving connections from server for DATA 
def createdatasocket(filecommand):
    if filecommand == "-l":
        dataportindex = 4 #referenced https://docs.python.org/2/howto/sockets.html
    elif filecommand == "-g": #if get value on to the next port to determine the data socket
        dataportindex = 5
    serverport = int(sys.argv[dataportindex])
    serversocket = socket(AF_INET,SOCK_STREAM)  
    serversocket.bind(('',serverport)) #bind socket
    serversocket.listen(1)
    newdataSocket, dataaddr = serversocket.accept() #accept server socket and data socket connection
    return newdataSocket
	
	
# getlistoffiles(createdsocket)
# Description: gets list of files based on created socket this is for -l flag
# parameters: pass in an argument of the createdsocket to get list of files from that socket
# Pre-Conditions:receive from file and before it is done we will print out the file.
# Post Conditions: printed out data from the file or directory; there are data of files in the directory
def getlistoffiles(createdsocket):
    filesindir = createdsocket.recv(100)
    while filesindir != "complete": #until iteration through the dir is complete/done
        print filesindir
        filesindir = createdsocket.recv(100)
	
# dupecheckandput(fileorget, filetxt)
# Description: checks dupes and puts a tag in front of .txt file for dupe file
# parameters: argument to determine if we are listing or getting files, and then the file txt
# Pre-Conditions:determining if -g flag is checked, we will see if there is a file to get and then replace it with 0 in front of it
# Post Conditions: print a dupe file in the current dir
def dupecheckandput(fileorget, filetxt):
    if(fileorget == "-g"):
        i = 0
        checkdigit = filetxt[0]
        if (checkdigit.isdigit()): # to be able to have more than 1 dupe
            i = checkdigit
            i+=1
            if filetxt in filter(os.path.isfile, os.listdir(os.curdir )): # os path will contain the cur dir through filter 
			#https://stackoverflow.com/questions/5137497/find-current-directory-and-files-directory - current dir python
    #https://www.geeksforgeeks.org/python-os-listdir-method/
     #https://stackoverflow.com/questions/11968976/list-files-only-in-the-current-directory
                src = filetxt
                dst = str(i)+ filetxt 
				# shutil() function will make a copy and append value of i to end

                shutil.copy(src, dst)  #https://docs.python.org/3/library/shutil.html
                i += 1
        else:
            if filetxt in filter(os.path.isfile, os.listdir(os.curdir )): #https://stackoverflow.com/questions/5137497/find-current-directory-and-files-directory - current dir python
    #https://www.geeksforgeeks.org/python-os-listdir-method/
     #https://stackoverflow.com/questions/11968976/list-files-only-in-the-current-directory
                
                src = filetxt
                dst = str(i)+ filetxt 
        # shutil() function will make a copy and append value of i to end

                shutil.copy(src, dst)  #https://docs.python.org/3/library/shutil.html
                i += 1
            
# openfile(fileorget, filereceived, datasocket)
# Description: opens files and then conducts writing to console
# parameters:arguments which include to determine flag -g or -l and what file is received and the data socket we are using to traverse the data
# Pre-Conditions:determining if -g flag is checked, we will see if there is a file to get it and open it up then execute get file or list
# Post Conditions: we will either get a file or dupe or list files onto console/directory
def openfile(fileorget, filereceived, datasocket):
    dupecheckandput(fileorget, filereceived) #to check dupes and put new file if so
    if(fileorget == "-g"):
        file = open(sys.argv[4], "w") #to write to it
        receivingbuffer = datasocket.recv(1000)
        while "complete" not in receivingbuffer:	#until complete
            file.write(receivingbuffer)
            receivingbuffer = datasocket.recv(1000) #continue grabbing from buffer
        print("File Transfer complete")
    elif(fileorget == "-l"):
       getlistoffiles(datasocket) #get list of files from data socket
	
	
#I read through example client side lines 58-100 to gather ideas https://github.com/TylerC10/CS372/blob/master/Project%202/ftclient.py
# receivedata(clientsocket)
# Description: receives data from client socket and basically contains above functions defined so this is the main driver of the FTP client
# parameters:receive data function will take into account the clientsocket as argument of where to receive the data
# Pre-Conditions: we need information such as flag value, and then the socket connection
# Post Conditions: we will either get a file or dupe or list files onto console/directory
def receivedata(clientsocket):
    listorget = sys.argv[3]
    filerequested = sys.argv[4]
    if listorget == "-l": #list file from dir
        print("Client is requesting to list file directory contents from Data Port: ", (sys.argv[1]),  sys.argv[4])
        portnum = 4
    else: #file that needs to be requested from server
        print("Client is requesting this file--", filerequested, " from Data Port: ", sys.argv[1],  sys.argv[5])
        portnum = 5
	#client socket send and receive 
    clientsocket.send(sys.argv[portnum])
    clientsocket.recv(1024) #receive data
    if listorget == "-l":
        clientsocket.send("l")
    elif listorget == "-g":
        clientsocket.send("g")
	# referenced the following to find local ip https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib/25850698#25850698
    newsocket = socket(AF_INET, SOCK_DGRAM)
    newsocket.connect(("8.8.8.8", 80))
    ipaddress = newsocket.getsockname()[0] #copied this from stackoverflow link above
    clientsocket.send(ipaddress) #provide ip address to the clientsocket
    response = clientsocket.recv(1024)
    if response == "invalid":
        print("Invalid command sent to the server.")
        exit(0)
    if sys.argv[3] == "-g": #sends file when -g
        clientsocket.send(sys.argv[4])
        response1 = clientsocket.recv(500) #recv value
        if response1 == "notavailable": #if we were unable to retrieve file
            print"Server cannot find this file."
            return #leveraged data receive from https://github.com/TylerC10/CS372/blob/master/Project%202/ftclient.py lines 84-89
    createdsocket = createdatasocket(listorget)
	
    openfile(listorget, filerequested, createdsocket) #open file 
    createdsocket.close() #close the created socket
	
#this is our main execution
if __name__ == "__main__":  ##https://stackoverflow.com/questions/4041238/why-use-def-main

    argvalidation() #validate args or command line
    ftpsocket = initiatecontact(sys.argv[1], int(sys.argv[2])) #referencing https://docs.python.org/2/howto/sockets.html
    print("Connected to server") #print that we are connected
    receivedata(ftpsocket) #receive data (this is the majority of our functionality
