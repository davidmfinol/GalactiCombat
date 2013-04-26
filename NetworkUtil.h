/*
 *      NetworkUtil.h contains methods that both GalactiCombatServer and NetworkManagerClient use.
 */

#ifndef __NetworkUtil_h
#define __NetworkUtil_h

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include "SDL/SDL.h"
#include "SDL/SDL_net.h"

#define ERROR 0xff
#define TIMEOUT 5000 //five seconds

#define TCP_PORT 5172
#define UDP_PORT 6172

// Used for the Packet struct
#define CONNECTION 10
#define STATE 11
#define INFO 12
#define PLAYERINPUT 13
#define SCORE 14
#define READY 15
#define MINERAL 13
#define SPACESHIP 14
#define WALL 16
#define NUMBER_OF_PACKETS 17

struct Packet {
    int type;
    char* message;
};
typedef struct Packet Packet; // allows Packet struct to be used in C

class NetworkUtil {
    
public:
    
    /*
    *           charArrayToPacket(char*):
    *           This method takes a char array and parses it into a Packet.
    *           The idea is that you can take the return value of TCPReceive() and use
    *           this method to turn it into a usable Packet struct.
    *
    *           It's not very robust and doesn't check to make sure that the argument can actually
    *           be converted into a Packet; it just assumes that it can. You may or may not
    *           want to FIXME.
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
        
        //free(typeFinder); // FIXME: SHOULD FREE THIS HERE
        //free(messageFinder); //TODO: CHECK THAT WHEN THE PACKET IS DISCARDED, THIS IS FREED
        return pack;
    }
    
    /*
    *           PacketToCharArray(Packet):
    *           This method takes a Packet and converts it to an
    *           array of chars. This makes it so that you can use it as the second
    *           argument in TCPSend().
    *
    *           -pack: the Packet to be converted
    *
    *           returns: the converted Packet
    */
    static char* PacketToCharArray(Packet pack)
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
    static char* TCPReceive(TCPsocket sock, char **buf)
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
                std::cerr << "SDLNet_TCP_Recv done goofed: " << SDLNet_GetError() << std::endl;
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
            std::cerr << "SDLNet_TCP_Recv done goofed: " << SDLNet_GetError() << std::endl;
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
            std::cerr << "SDLNet_TCP_Send done goofed2:" << std::endl;
            if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
                std::cerr << "\t- " << SDLNet_GetError() << std::endl;
            return(0);
        }
        
        // revert to our local byte order
        len=SDL_SwapBE32(len);
        
        // send the buffer, with the NULL as well
        result=SDLNet_TCP_Send(sock,buf,len);
        
        if(result<len) {
            std::cerr << "SDLNet_TCP_Send done goofed2:" << std::endl;
            if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
                std::cerr << "\t- " << SDLNet_GetError() << std::endl;
            return(0);
        }
            
        // return the length sent
        return(result);
    }
    
    /*
     *          UDPSend(UDPsocket, UDPpacket*):
     *          Wrapper function for SDLNet_UDP_Send.
     *
     *          -sock: The UDPsocket to send the packet through.
     *          -outgoing: The UDPpacket to send.
     */
    static int UDPSend(UDPsocket sock, UDPpacket *outgoing)
    {
        int numSent = SDLNet_UDP_Send(sock, outgoing->channel, outgoing);
        if(!numSent)
        {
            std::cerr << "SDLNet_UDP_Send done goofed: " << SDLNet_GetError() << std::endl;
            exit(4);
        }
        return numSent;
    }
    
    /*
     *          UDPReceive(UDPsocket, UDPpacket, Uint32, Uint8, int):
     *          Wrapper function for SDLNet_UDP_Recv.
     *
     *          sock: The UDPsocket to listen on.
     *          -in: The incoming UDPpacket.
     */
    static int UDPReceive(UDPsocket sock, UDPpacket *in)
    {
        int received = SDLNet_UDP_Recv(sock, in);
        if(received > 0)
        {
            std::cerr << "SDLNet_UDP_Recv done goofed: " << SDLNet_GetError() << std::endl;
            exit(4);
        }
        if(received == 0)
        {
            std::cout << "UDPReceive: No packets received." << std::endl;
        }
        return received;
    }
};

#endif //#ifndef __NetworkUtil_h
