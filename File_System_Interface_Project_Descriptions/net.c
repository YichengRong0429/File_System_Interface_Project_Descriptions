#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "net.h"
#include "jbod.h"

/* the client socket descriptor for the connection to the server */
int cli_sd = -1;
/* attempts to read n (len) bytes from fd; returns true on success and false on failure. 
It may need to call the system call "read" multiple times to reach the given size len. 
*/

static bool nread(int fd, int len, uint8_t *buf) 
{
  if(read(fd, buf, len) == len)  //try to read the same amount as you need to read
  {
    return true;  // if the size is equal return true
  }
  return false;
}

/* attempts to write n bytes to fd; returns true on success and false on failure 
It may need to call the system call "write" multiple times to reach the size len.
*/
static bool nwrite(int fd, int len, uint8_t *buf) 
{
  if(write(fd, buf, len)== len) //try to write the same amount as you need to write
  {
    return true;  // if the size is equal return true
  }
  return false;
}

/* Through this function call the client attempts to receive a packet from sd 
(i.e., receiving a response from the server.). It happens after the client previously 
forwarded a jbod operation call via a request message to the server.  
It returns true on success and false on failure. 
The values of the parameters (including op, ret, block) will be returned to the caller of this function: 

op - the address to store the jbod "opcode"  
ret - the address to store the return value of the server side calling the corresponding jbod_operation function.
block - holds the received block content if existing (e.g., when the op command is JBOD_READ_BLOCK)

In your implementation, you can read the packet header first (i.e., read HEADER_LEN bytes first), 
and then use the length field in the header to determine whether it is needed to read 
a block of data from the server. You may use the above nread function here.  
*/
static bool recv_packet(int sd, uint32_t *op, uint16_t *ret, uint8_t *block) 
{
  uint8_t temp[HEADER_LEN]; // set the temp value storage
  if(nread(sd,8,temp))  // if nread success do following
  {
    uint16_t len;
    memcpy(&len,&temp[0],2);//get the length
    memcpy(op,temp+2,4); // get the op code
    *op=ntohl(*op); // exchange the value  so host can understand it
    memcpy(ret,temp+6,2); //ge the regester
    *ret=0;
    uint16_t nlen=ntohs(len); // exchange the value  so host can understand it
    if(nlen==264) // make sure it's the correct size
    {
      if(nread(sd,256,block)==false)  //if nread fail return false
      {
        return false;
      }
    }
  }
  else
  {
    return false;
  }
  return true;  //if n read success return true
}



/* The client attempts to send a jbod request packet to sd (i.e., the server socket here); 
returns true on success and false on failure. 

op - the opcode. 
block- when the command is JBOD_WRITE_BLOCK, the block will contain data to write to the server jbod system;
otherwise it is NULL.

The above information (when applicable) has to be wrapped into a jbod request packet (format specified in readme).
You may call the above nwrite function to do the actual sending.  
*/
static bool send_packet(int sd, uint32_t op, uint8_t *block) 
{
  if((op>>26)==JBOD_WRITE_BLOCK) //compare the op code if it's same with write do following
  {
    uint16_t len= 264;  // set the len equal to 264
    uint8_t bag[264]; //set the temp size for information we need to pass
    uint16_t tranLen = htons(len);  //translate the value so network could understand 
    uint32_t tranOp = htonl(op); 
    memcpy(&bag[0],&tranLen,2); //memcpy to the correct position, store data in bag
    memcpy(bag+2,&tranOp,4);  
    memcpy(bag+8,block,256);
    if(nwrite(sd,HEADER_LEN+JBOD_BLOCK_SIZE,bag)==false) //call the write function if return false means unsucces
    {
      return false;
    }
  }
  else
  {
    uint16_t len= HEADER_LEN; //just need the header length
    uint8_t bag[264];// set the len equal to 264
    uint16_t tranLen = htons(len);//translate the value so network could understand 
    uint32_t tranOp = htonl(op);
    memcpy(&bag[0],&tranLen,2);//memcpy to the correct position, store data in bag
    memcpy(bag+2,&tranOp,4);
    if(nwrite(sd,HEADER_LEN,bag)==false)//call the write function if return false means unsucces
    {
      return false;
    }
  }
  return true;
}



/* attempts to connect to server and set the global cli_sd variable to the
 * socket; returns true if successful and false if not. 
 * this function will be invoked by tester to connect to the server at given ip and port.
 * you will not call it in mdadm.c
*/
bool jbod_connect(const char *ip, uint16_t port) 
{
  cli_sd = socket(AF_INET, SOCK_STREAM, 0); //create a socket base on the ppt's introduction
  if(cli_sd == -1)  // if cli_sd still not changed return false
  {
    return false;
  }
  struct sockaddr_in caddr; //set the server information struct
  caddr.sin_family = AF_INET; //assign the AF INET for this struct
  caddr.sin_port = htons(port); //assign the port value
  if(inet_aton(ip, &caddr.sin_addr)==0) // can't find the ip addresss for struct
  {
    return false;
  }
  if(connect(cli_sd, (const struct sockaddr*)&caddr,sizeof(caddr))==-1) //if the connection fail returan false 
  {
    return false;
  }
  return true;
}




/* disconnects from the server and resets cli_sd */
void jbod_disconnect(void) 
{
  close(cli_sd);  // close the cli_sd
  cli_sd = -1;  //set the value to original value
}



/* sends the JBOD operation to the server (use the send_packet function) and receives 
(use the recv_packet function) and processes the response. 

The meaning of each parameter is the same as in the original jbod_operation function. 
return: 0 means success, -1 means failure.
*/
int jbod_client_operation(uint32_t op, uint8_t *block) 
{
  if(send_packet(cli_sd,op,block)==false)//if send unsucceful return -1
  {
    return -1;  //return -1
  }
  else
  {
    u_int16_t check;  //set a return value
    recv_packet(cli_sd,&op, &check,block);  //send the return value in there and check the value back
    if(check!=0)
    {
      return -1; //if value !=0 return -1
    }
    else
    {
      return 0;
    }
  }
}
