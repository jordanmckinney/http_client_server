#HTTP Client/Server

##Overview

This project consists of a simple HTTP client and HTTP server
written in C.

The client takes a URI as it's argument, and performs a GET 
request. The response is printed to the terminal.

The server takes as arguments the desired port number on which
it will listen, and the directory in which it will search for 
requested files. A directory called 'web' has been included with
a small 'index.html' file inside for testing purposes.

##Compilation/Execution

###Compilation

Dependencies: libpcap (Ubuntu: sudo apt-get install libpcap)

Note: This project was done as a class assignment and has 
limited functionality. The client should work, but the server
was made to run with a specific lab router and IP. 

To build the client and server at the same time type 'make' 
into the terminal. 
To build just the client type 'make client' into the terminal.
To build just the server type 'make server' into the terminal.

###Execution

To run the client type './SimpClient' into the terminal. 
You will then be prompted to enter the desired URI. Details 
regarding the request and response will be printed to the 
terminal. 

To run the server type './SimpServer PORT FOLDER' where PORT 
is replaced with the desired port number, and FOLDER is 
replaced with the desired folder (use 'web' for the default
folder). The server will run continuously, serving requests 
until the process is terminated.

##Testing

###Client & Server

1. Run the server as described above. 
   Make note of the chosen PORT number, and use the 'web' folder
   if you wish to see the server successfully deliver a file. 
   For example to use the default port number and folder, type:

   ./SimpServer 80 web

2. To run the client type:

   ./SimpClient

   If the server was run using the default port number 80, then
   the sample html file stored on the server can be fetched by
   typing the following when prompted for a URI:
   
   http://10.10.1.100:80/index.html

   This should result in the html file contents being printed to 
   the client terminal. 

3. Testing 404 error.
   To see how the server responds when a file is requested that 
   is not in the specified folder run the server as in step 1, 
   but give the client the following URI:

   http://10.10.1.100:80/test.txt

4. Testing 501 error.
   To see how the server responds to an unsupported HTTP method
   or version edit line 141 in SimpClient.c. Change "GET" to 
   "POST", or any other method. Alternatively on the same line 
   "HTTP/1.0" can be changed to "HTTP/1.1" or any other version 
   number and the server will respond with a 501 error.

###Client Only

1. Run the client as described above. 
   When prompted for the URI, type:

   http://www.csc.uvic.ca/About/message_from_the_chair.htm

2. Using the IP address rather than hostname you can type:

   http://142.104.70.172/About/message_from_the_chair.htm

##Sources

The skeleton files for SimpServer.c and SimpClient.c, as well as
util.c and the makefile were provided by Dr. Kui Wu (posted on connex).

Portions of the client and server code were borrowed from tcp_client 
and tcp_server by Seyed Dawood Sajjadi Torshizi (posted on connex).

The scan_dir function in SimpServer.c was borrowed from user Enkeli on
StackExchange. Link provided below.

[http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer]

The server timestamp code was borrowed from user Adam Rosenfield on 
StackExchange. Link provided below.

[http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program]
