#include "NetworkManagerClient.h"
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>

#else
#include <windows.h>
#include <time.h>

#endif

NetworkManagerClient::NetworkManagerClient(void) : connected(false), scores()
{
    if(SDL_Init(0) != 0){
        printf("SDL_Init done goofed: %s\n", SDL_GetError());
        exit(1);
    }
    
    if(SDLNet_Init() != 0){
        printf("SDLNet_Init done goofed: %s\n",SDLNet_GetError());
    }
}

NetworkManagerClient::~NetworkManagerClient(void)
{
    SDLNet_Quit();
    SDL_Quit();
}

//main method from tcpmulticlient.c in the SDLNet demos
int NetworkManagerClient::TCPConnect(char *host, long portNo, char *name)
{
    IPaddress ip;
    //	TCPsocket sock;
    char message[MAXLEN];
    int numready;
    SDLNet_SocketSet set;
    fd_set fdset;
    int result;
    char *str;
    struct timeval tv;
    Uint16 port = (Uint16)portNo;
    
    Packet pack;
    
    set = SDLNet_AllocSocketSet(1);
    if(!set)
    {
        printf("SDLNet_AllocSocketSet done goofed: %s\n", SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        exit(4); /*most of the time this is a major error, but do what you want. */
    }
    
    
    /* Resolve the argument into an IPaddress type */
    printf("Connecting to %s port %d\n", host, port);
    if(SDLNet_ResolveHost(&ip, host, port)==-1)
    {
        printf("SDLNet_ResolveHost done goofed: %s\n",SDLNet_GetError());
        std::string exception = "fail_to_connect";
        throw exception;
    }
    
    printf("Opening server socket.\n");
    /* open the server socket */
    serverSock = SDLNet_TCP_Open(&ip);
    if(!serverSock)
    {
        printf("SDLNet_TCP_Open done goofed: %s\n",SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        std::string exception = "fail_to_connect";
        throw exception;
    }
    
    
    if(SDLNet_TCP_AddSocket(set,serverSock) == -1)
    {
        printf("SDLNet_TCP_AddSocket done goofed: %s\n",SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        exit(7);
    }
    
    
    pack.type = CONNECTION;
    pack.message = name;
    /* login with a name */
    char* out = PacketToCharArray(pack);
    printf("Sent %s\n", out);
    if(!TCPSend(serverSock, out))
    {
        SDLNet_TCP_Close(serverSock);
        exit(8);
    }
    
    printf("Logged in as %s\n",name);
    connected = true;
}

void NetworkManagerClient::sendPlayerScore(double score)
{
    Packet outgoing;
    std::stringstream ss;
    std::string s;
    ss << score;
    s = ss.str();
    outgoing.type = SCORE;
    outgoing.message = const_cast<char*>(s.c_str());
    
    char *incoming = NULL;
    char *out = PacketToCharArray(outgoing);
    //printf("Sending: %s\n", out);
    if(TCPSend(serverSock, out) && TCPReceive(serverSock, &incoming)) {
        printf("Receving: %s\n", incoming);
        Packet pack = charArrayToPacket(incoming);
        if(pack.type != SCORE)
        {
            printf("Error in sendPlayerScore() in NetworkManagerClient.cpp. Score not received from server.\n");
            scores = "";
        }
        else{
            printf("Received message: %s\n", pack.message);
            scores = pack.message;
        }
    }
    else {
        connected = false;
        scores = "";
    }
}

std::string NetworkManagerClient::getPlayerScores()
{
    return scores; //"name,score;"
}

void NetworkManagerClient::resetReadyState()
{
    Packet outgoing;
    outgoing.type = READY;
    outgoing.message = const_cast<char*>("RESET");
    TCPSend(serverSock, PacketToCharArray(outgoing));
}

void NetworkManagerClient::sendPlayerInput(ISpaceShipController* controller)
{
    Packet outgoing;
    
    bool left = controller->left();
    bool right = controller->right();
    bool up = controller->up();
    bool down = controller->down();
    bool forward = controller->forward();
    bool back = controller->back();
    bool shoot = controller->shoot();
    
    char result = left;
    result = (result << 1) + right;
    result = (result << 1) + up;
    result = (result << 1) + down;
    result = (result << 1) + forward;
    result = (result << 1) + back;
    result = (result << 1) + shoot;
    
    outgoing.type = PLAYERINPUT;
    outgoing.message = &result;
    
    char *out = PacketToCharArray(outgoing);
    if(!TCPSend(serverSock, out))
        connected = false;
}

bool NetworkManagerClient::isOnline()
{
    return connected;
}

void NetworkManagerClient::quit()
{
    Packet outgoing;
    outgoing.type = CONNECTION;
    outgoing.message = const_cast<char*>("QUIT");
    while(!TCPSend(serverSock, PacketToCharArray(outgoing))); // FIXME: what if server crashed?
    connected = false;
}

void NetworkManagerClient::receiveData(Ogre::SceneManager* sceneManager, SoundManager* sound, std::vector<Mineral*>& minerals, std::vector<SpaceShip*>& spaceships, std::vector<GameObject*>& walls)
{
    Packet outgoing;
    outgoing.type = STATE;
    outgoing.message = "";
    char *out = PacketToCharArray(outgoing);
    if(!TCPSend(serverSock, out))
    {
        connected = false;
        return;
    }
    
    char* incoming;
    if(!TCPReceive(serverSock, &incoming))
    {
        connected = false;
        return;
    }
    Packet numPackets = charArrayToPacket(incoming);
    int packs = atoi(numPackets.message);
    std::cout << "We are expecting to receive " << packs << " number of packets" << std::endl;
    
    for(int i = 0; i < packs; ++i)
    {
        std::cout << "We are processing packet number " << i << "." << std::endl;
        if(!TCPReceive(serverSock, &incoming)) {
            connected = false; break;}
        Packet in = charArrayToPacket(incoming);
        if(in.type == MINERAL)
        {
            std::string message(in.message);
            std::string name = message.substr(0, message.find(","));
            message = message.substr(message.find(",") + 1);
            double pos_x = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_y = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_z = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double vel_x = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double vel_y = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double vel_z = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double rot_w = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double rot_x = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double rot_y = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double rot_z = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double radius = atof(message.substr(0, message.find(",")).c_str());
            
            // FIXME: THIS IS BAD, oh well
            bool found = false;
            for(int j = 0; j < minerals.size(); ++i)
            {
                if(minerals[j]->getName() == name)
                {
                    found = true;
                    minerals[j]->getSceneNode()->setPosition(pos_x, pos_y, pos_z);
                    minerals[j]->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                    break;
                }
            }
            if(!found)
            {
                minerals.push_back(new Mineral(name, sound, sceneManager->getRootSceneNode(), pos_x, pos_y, pos_z, radius));
                minerals.back()->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
            }
        }
        else if(in.type == SPACESHIP)
        {
            
        }
        else if(in.type == WALL)
        {
            
        }
    }
}

