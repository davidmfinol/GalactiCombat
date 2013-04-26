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
int NetworkManagerClient::connect(char *host, char *name)
{
    IPaddress ip;
    
    char message[MAXLEN];
    int numready;
    
    fd_set fdset;
    int result;
    char *str;
    struct timeval tv;
//	Uint16 port = (Uint16)5172;
 	Uint16 port = (Uint16) TCP_PORT;  
    Packet pack;
    
    // Resolve the argument into an IPaddress type
    printf("Connecting to %s port %d\n", host, port);
    if(SDLNet_ResolveHost(&ip, host, port)==-1)
    {
        printf("SDLNet_ResolveHost done goofed: %s\n",SDLNet_GetError());
        std::string exception = "fail_to_connect";
        throw exception;
    }
    
	serverIP = ip;

    // open the server socket
    printf("Opening server socket.\n");
    TCPServerSock = SDLNet_TCP_Open(&ip);
    if(!TCPServerSock)
    {
        printf("SDLNet_TCP_Open done goofed: %s\n",SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        std::string exception = "fail_to_connect";
        throw exception;
    }

	UDPServerSock = SDLNet_UDP_Open(UDP_PORT+1);
	//server uses UDP_PORT; +1 is necessary when server and client run
	//on the same machine
	if(!UDPServerSock)
	{
		std::cout << "SDLNet_UDP_Open done goofed: " << SDLNet_GetError() << std::endl;
		SDLNet_Quit();
		SDL_Quit();
		std::string exception = "fail_to_connect";
		throw exception;
	}
    
    pack.type = CONNECTION;
    pack.message = name;
    // login with a name
    char* out = PacketToCharArray(pack);
    printf("Sent %s\n", out);
    if(!TCPSend(TCPServerSock, out))
    {
        SDLNet_TCP_Close(TCPServerSock);
        exit(8);
    }
    
    mName = name;
    std::cout << "Logged in as " << mName << std::endl;
    connected = true;
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
    TCPSend(TCPServerSock, PacketToCharArray(outgoing));
}

void NetworkManagerClient::sendPlayerInput(ISpaceShipController* controller)
{
    Packet outgoing;
	UDPpacket *UDPPack = SDLNet_AllocPacket(65535);
	if(!UDPPack)
	{
		std::cout << "SDLNet_AllocPacket done goofed: " << SDLNet_GetError() << std::endl;
		return;
	}
    
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
    TCPSend(serverSock, out);
/*
	UDPPack->data = (Uint8*)out;
	UDPPack->len = strlen(out) + 1;
	UDPPack->address = serverIP;
	UDPSend(UDPServerSock, -1, UDPPack);
	SDLNet_FreePacket(UDPPack);
*/	

}

bool NetworkManagerClient::isOnline()
{
    return connected;
}

void NetworkManagerClient::quit()
{
    Packet outgoing;
/*
	UDPpacket *UDPPack = SDLNet_AllocPacket(65535);
	if(!UDPPack)
	{
		std::cerr << "SDLNet_AllocPacket done goofed: " << SDLNet_GetError() << std::endl;
		return;
	}
*/
    outgoing.type = CONNECTION;
    outgoing.message = const_cast<char*>("QUIT");

	char *msg = PacketToCharArray(outgoing);
	while(!TCPSend(TCPServerSock, msg)); //FIXME: what if server crashed?
/*
	UDPPack->data = (Uint8*)msg;
	UDPPack->len = strlen(msg) + 1;
	UDPPack->address = serverIP;
	UDPSend(UDPServerSock, -1, UDPPack);
	SDLNet_FreePacket(UDPPack);
*/
    connected = false;
	std::cout << "You have quit the game." << std::endl;
}

void NetworkManagerClient::sendPlayerScore(double score)
{
    std::stringstream ss;
    std::string s;
    ss << score;
    s = ss.str();
 
    Packet outgoing;
    outgoing.type = SCORE;
    outgoing.message = const_cast<char*>(s.c_str());   
    char *incoming = NULL;
    char *out = PacketToCharArray(outgoing);
    if(TCPSend(TCPServerSock, out) && TCPReceive(TCPServerSock, &incoming)) {
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

void NetworkManagerClient::receiveData(Ogre::SceneManager* sceneManager, SoundManager* sound, std::vector<Mineral*>& minerals, std::vector<SpaceShip*>& spaceships, std::vector<GameObject*>& walls)
{
    std::cout << "Beginning of receiveData" << std::endl << std::endl;
    static int iii = 0;
    Packet outgoing;
    outgoing.type = STATE;
    outgoing.message = "NONE";
    char* incoming = NULL;
    char* out = PacketToCharArray(outgoing);
    std::cout << "About to request data" << std::endl << std::endl;

    while (!TCPSend(serverSock, out));
    std::cout << "About to receive data" << std::endl << std::endl;
    while (!TCPReceive(serverSock, &incoming));
    Packet infoPacket = charArrayToPacket(incoming);
    std::cout << iii++ << ": " << infoPacket.message << std::endl << std::endl;
    std::cout << "End of receiveData" << std::endl << std::endl;

                        /*
                        for(int i = 0; i < packs; ++i)
                        {
                            //std::cout << "We are about to receive packet number " << i << "." << std::endl;
                            if(TCPReceive(serverSock, &incoming)) {
                                //std::cout << "We received packet number " << i << "." << std::endl;
                                std::string msg(incoming);
                                std::cout << "---" << msg << std::endl;
                                Packet in = charArrayToPacket(incoming);
                                if(in.type == MINERAL)
                                {
                                    //std::cout << "It was a mineral." << std::endl;
                                    std::string message(in.message);
                                    //std::cout << "The mineral's message is " << message << std::endl;
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
                                    //std::cout << "We parsed that message." << message << std::endl;
                                    
                                    // FIXME: THIS IS BAD, oh well
                                    bool found = false;
                                    for(int j = 0; j < minerals.size(); ++j)
                                    {
                                        //std::cout << "Checking to see if " << name << " already exists." << std::endl;
                                        if(minerals[j]->getName() == name)
                                        {
                                            //std::cout << "Exists." << std::endl;
                                            found = true;
                                            minerals[j]->getSceneNode()->setPosition(pos_x, pos_y, pos_z);
                                            minerals[j]->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                                            break;
                                        }
                                    }
                                    if(!found)
                                    {
                                        //std::cout << "Doesn't exist, create it." << std::endl;
                                        minerals.push_back(new Mineral(name, sound, sceneManager->getRootSceneNode(), pos_x, pos_y, pos_z, radius));
                                        minerals.back()->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                                    }
                                    //std::cout << "Got the Mineral!" << std::endl;
                                }
                                else if(in.type == SPACESHIP)
                                {
                                    //std::cout << "It was a spaceship." << std::endl;
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
                                    double size = atof(message.substr(0, message.find(",")).c_str());
                                    
                                    // FIXME: THIS IS BAD, oh well
                                    bool found = false;
                                    if(name == mName)
                                    {
                                        spaceships[0]->getSceneNode()->getParentSceneNode()->setPosition(pos_x, pos_y, pos_z);
                                        spaceships[0]->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                                        break;
                                    }
                                    for(int j = 1; j < spaceships.size(); ++j)
                                    {
                                        if(spaceships[j]->getName() == name)
                                        {
                                            found = true;
                                            spaceships[j]->getSceneNode()->setPosition(pos_x, pos_y, pos_z);
                                            spaceships[j]->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                                            break;
                                        }
                                    }
                                    if(!found)
                                    {
                                        ISpaceShipController* controller = new ClientSpaceShipController();
                                        spaceships.push_back(new SpaceShip(name, sound, controller, sceneManager->getRootSceneNode()->createChildSceneNode(),size));
                                        spaceships.back()->getSceneNode()->getParentSceneNode()->setPosition(pos_x, pos_y, pos_z);
                                        spaceships.back()->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                                    }
                                    //std::cout << "Got the SpaceShip!" << std::endl;
                                }
                                else if(in.type == WALL)
                                {
                                    //std::cout << "It was a wall." << std::endl;
                                }
                            }
                            else {
                                connected = false; 
                                break;
                            }
                        }
                        */
}

