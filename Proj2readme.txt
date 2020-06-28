#Jason Lim
#CS 372
#12/1/19
#Prog 2 - FTP Client / Server

1) Start the server by doing
gcc -std=gnu99 -o ftserver ftserve.c (need gnu99 for some features to work)

The port must be valid. I used 15330-15335 while testing this.

Find hostname by using command "hostname" in putty or look at the console on left hand side of command prompt for flipX host

When I tested this I had flip1.engr.oregonstate.edu as my hostname

Command to start server will be the following as an example:
ftserver 15300

2) To test python client connection to server do following:

python ftclient.py flipX.engr.oregonstate.edu 15300 

*Note: flipX.engr.oregonstate.edu can be shorthanded to just flipX

then to run the FTP component make sure to use the commands for -l (list) and -g (get) and specify files
python ftclient.py hostname serverport -l|-g filename dataport

Command example:

python ftclient.py flipX.engr.oregonstate.edu 15300 -l 15333 (to list what is in dataport 15333)

python ftclient.py flipX.engr.oregonstate.edu 15300 -g file.txt 15333 (to receive/get what is in dataport 15333)

Test two files:
The small will be on the order or 1-10kB
The large will be around 10MB

We will have validation checks to ensure the user enters the correct commands in the correct positions separated by space

5) Continue until server quits by CTRL-C or after client executes their command
Thus, after server validates the clients request, the server will send a response to the command then close the client's data connection. 
If the client wants more than one file, they will have to connect again.
Sending directory data for -l list command or file -g command will close data-connection at the client