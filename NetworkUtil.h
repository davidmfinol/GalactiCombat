/*
 *	NetworkUtil.h contains methods that both GalactiCombatServer and NetworkManagerClient use.
 *	The majority of the send and receive code was copied from the SDLNet demos.
 */

#ifndef __NetworkUtil_h
#define __NetworkUtil_h 1

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include "SDL/SDL.h"
#include "SDL/SDL_net.h"

#define ERROR 0xff
#define TIMEOUT 5000 //five seconds

// Used for the Packet struct
#define CONNECTION 10
#define SCORE 11
#define PLAYERINPUT 12
#define MINERAL 13
#define SPACESHIP 14
#define READY 15
#define WALL 16
#define NUMBER_OF_PACKETS 17

typedef struct{
    int type;
    char* message;
} Packet;

/*
 *        charArrayToPacket(char*):
 *        This method takes a char array and parses it into a Packet (defined in GalactiCombat.h).
 *        The idea is that you can take the return value of TCPReceive() and use
 *        this method to turn it into a usable Packet struct.
 *        
 *        It's not very robust and doesn't check to make sure that the argument can actually
 *        be converted into a Packet; it just assumes that it can. You may or may not
 *        want to FIXME.
 */
static Packet charArrayToPacket(char* msg)
{
    Packet pack;
    int messageLength = strlen(msg) + 1;
    
    char* typeFinder = (char*)malloc(2);
    memmove(typeFinder, msg, 2);
    int type = atoi(typeFinder);
    pack.type = type;

    if(type == PLAYERINPUT)
        messageLength = 3;
    char* messageFinder =  (char*)malloc(messageLength-2);
    
    for(int i = 2; i < messageLength; ++i) {
        messageFinder[i-2] = msg[i];
    }
    pack.message = messageFinder;

//  free(typeFinder); //FIXME: FREE?
//  free(messageFinder);
    return pack;
}

/*
 *        PacketToCharArray(Packet):
 *        This method takes a Packet (defined in NetworkManagerClient.h) and converts it to an
 *        array of chars. This makes it so that you can use it as the second
 *        argument in TCPSend(). 
 * 
 *        -pack: the Packet to be converted
 *                
 *        returns: the converted Packet
 */
static char *PacketToCharArray(Packet pack)
{
    int messageLength = strlen(pack.message) + 1;
    if(pack.type == PLAYERINPUT)
        messageLength = 1;
    
    char* charArray = (char*)malloc(2 + messageLength);
    //sprintf(charArray, "%2d", pack.type);
    //TODO: may want to fix, division is expensive
    char digit1 = pack.type/10 + 48;
    char digit2 = pack.type%10 + 48;
    memmove(charArray, &digit1, 1);
    memmove(charArray+1, &digit2, 1);
    memmove(charArray+2, pack.message, messageLength);
    
    return charArray;
}


/*
 *        TCPReceive(TCPsocket, char**):
 *        This method listens for a message on the specified socket.
 *        This was copied from the method getMsg() from tcputil.h in the SDLNet example code.
 * 
 *        -sock: the socket to listen on
 *        -buf: a pointer to a block of memory where the data can be stored
 */
static char *TCPReceive(TCPsocket sock, char **buf)
{
    Uint32 len,result;
    static char *_buf;
    
    // allow for a NULL buf, use a static internal one...
    if(!buf)
        buf=&_buf;
    
    // free the old buffer 
    if(*buf)
        free(*buf);
    *buf=NULL;
    
    // receive the length of the string message
    result=SDLNet_TCP_Recv(sock,&len,sizeof(len));
    if(result<sizeof(len))
    {   
        if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            printf("SDLNet_TCP_Recv done goofed: %s\n", SDLNet_GetError());
        return(NULL);
    }   
    
    // swap byte order to our local order
    len=SDL_SwapBE32(len);
    
    // check if anything is strange, like a zero length buffer
    if(!len)
        return(NULL);
    
    // allocate the buffer memory
    *buf=(char*)malloc(len);
    if(!(*buf))
        return(NULL);
    
    // get the string buffer over the socket
    result = SDLNet_TCP_Recv(sock,*buf,len);
    if(result < len)
    {   
        if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            printf("SDLNet_TCP_Recv done goofed: %s\n", SDLNet_GetError());
        free(*buf);
        buf=NULL;
    }   
    
    // return the new buffer
    return(*buf);
}

/*
 *        TCPSend(TCPsocket, char*):
 *        This method is basically copy-pasted from the SDLNet example code.    
 *        The method copied is putMsg() from tcputil.h.
 *        
 *        -sock: the TCPsocket through which to send the data
 *        -buf: the data to be sent
 * 
 *        returns: the number of bytes sent, or 0 if error, or 1 if the second argument is empty
 */
static int TCPSend(TCPsocket sock, char *buf)
{
    Uint32 len,result;

    if(!buf || !strlen(buf))
        return(1);

    // determine the length of the string 
    len=strlen(buf)+1; // add one for the terminating NULL 
    // change endianness to network order 
    len=SDL_SwapBE32(len);

    // send the length of the string 
    result=SDLNet_TCP_Send(sock,&len,sizeof(len));

    if(result<sizeof(len)) {
        printf("SDLNet_TCP_Send done goofed1:\n");
        if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            printf("\t- %s\n", SDLNet_GetError());
        return(0);
    }

    // revert to our local byte order
    len=SDL_SwapBE32(len);

    // send the buffer, with the NULL as well
    result=SDLNet_TCP_Send(sock,buf,len);

    if(result<len) {
        printf("SDLNet_TCP_Send done goofed2:\n");
        if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            printf("\t- %s\n", SDLNet_GetError());
        return(0);
    }

    // return the length sent
    return(result);
}


/*
 *		UDPSend(UDPsocket, int, UDPpacket, UDPpacket, Uint32, Uint8, int):
 *		Sends information throught a network using UDP.
 *
 *		-sock: The UDPsocket to send the packet through.
 *		-channel: The channel to send packets through. Set to -1 if not using channels.
 *		-out: The outgoing UDPpacket.
 *		-in: The incoming UDPpacket.
 *		-delay: Number of milliseconds to wait if packet received is empty.
 *		-expect: I'm not sure what this does (NOTE)
 *		-timeout: Number of milliseconds to wait before declaring a timeout.
 */
static int UDPSend(UDPsocket sock, int channel, UDPpacket *out, UDPpacket *in, Uint32 delay, Uint8 expect, int timeout)
{
    Uint32 t,t2;
    int err;

    in->data[0] = 0;
    t = SDL_GetTicks();
    do
    {
        t2 = SDL_GetTicks();
        if(t2-t>(Uint32)timeout)
        {
            printf("UDPSend timed out.\n");
            return(0);
        }   
        if(!SDLNet_UDP_Send(sock, channel, out))
        {   
            printf("SDLNet_UDP_Send: %s\n",SDLNet_GetError());
            exit(1);
        }   
        err = SDLNet_UDP_Recv(sock, in);
        if(!err)
            SDL_Delay(delay);
    } while(!err || (in->data[0] != expect && in->data[0] != ERROR));
    if(in->data[0] == ERROR)
        printf("UDPSend received error code.\n");
    return(in->data[0] == ERROR? -1:1);	//returns -1 on error
}

/*
 *		UDPReceive(UDPsocket, UDPpacket, Uint32, Uint8, int):
 *		Receives packets through UDP.
 *
 *		-sock: The UDPsocket to listen on.
 *		-in: The incoming UDPpacket.
 *		-delay: Number of milliseconds to wait when receiving an empty packet.
 *		-expect: I'm not sure what this does (NOTE)
 *		-timeout: Number of milliseconds to wait until declaring a timeout.
 */
static int UDPReceive(UDPsocket sock, UDPpacket *in, Uint32 delay, Uint8 expect, int timeout)
{
    Uint32 t,t2;
    int err;
    
    in->data[0] = 0;
    t = SDL_GetTicks();
    do  
    {   
        t2 = SDL_GetTicks();
        if(t2-t>(Uint32)timeout)
        {
            printf("timed out\n");
            return(0);
        }
        err = SDLNet_UDP_Recv(sock, in);
        if(!err)
            SDL_Delay(delay);
    } while(!err || (in->data[0] != expect && in->data[0] != ERROR));
    if(in->data[0] == ERROR)
        printf("received error code\n");
    return(in->data[0] == ERROR?-1:1);
}

#endif //#ifndef __NetworkUtil_h
