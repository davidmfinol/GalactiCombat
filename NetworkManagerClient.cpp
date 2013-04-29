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

NetworkManagerClient::NetworkManagerClient(void) : connected(false)
{
    if(SDL_Init(0) != 0)
    {
        std::cerr << "SDL_Init done goofed: " << SDL_GetError() << std::endl;
        exit(1);
    }
    
    if(SDLNet_Init() != 0)
        std::cerr << "SDLNet_Init done goofed: " << SDLNet_GetError() << std::endl;
}

NetworkManagerClient::~NetworkManagerClient(void)
{
    SDLNet_Quit();
    SDL_Quit();
}

int NetworkManagerClient::connect(char *host, char *name)
{
    //std::cout << "Entering TCPConnect" << std::endl << std::endl;
    Uint16 port = (Uint16) TCP_PORT;
    
    SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);
    if(!set)
    {
        std::cerr << "SDLNet_AllocSocketSet done goofed: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        SDL_Quit();
        exit(4); /*most of the time this is a major error, but do what you want. */
    }
    
    // Resolve the argument into an IPaddress type 
    std::cout << "Connecting to " << host << " port " << port << std::endl;
    IPaddress ip;
    if(SDLNet_ResolveHost(&ip, host, port)==-1)
    {
        std::cerr << "SDLNet_ResolveHost done goofed: " << SDLNet_GetError() << std::endl;
        std::string exception = "fail_to_connect";
        throw exception;
    }
    
    // open the server socket
    std::cout << "Opening server socket." << std::endl;
    TCPServerSock = SDLNet_TCP_Open(&ip);
    if(!TCPServerSock)
    {
        std::cerr << "SDLNet_TCP_Open done goofed: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        SDL_Quit();
        std::string exception = "fail_to_connect";
        throw exception;
    }
    
    
    UDPServerSock = SDLNet_UDP_Open(0);
    if(!UDPServerSock)
    {
        std::cerr << "SDLNet_UDP_Open done goofed: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        SDL_Quit();
        std::string exception = "fail_to_connect";
        throw exception;
    }
    
    
    // login with a name
    Packet pack;
    pack.type = CONNECTION;
    pack.message = name; // FIXME: SHOULD USE? :char message[MAXLEN];
    char* out = NetworkUtil::PacketToCharArray(pack);
    std::cout << "Sent " << out << std::endl;
    if(!NetworkUtil::TCPSend(TCPServerSock, out))
    {
        SDLNet_TCP_Close(TCPServerSock);
        exit(8);
    }
    
    // store our connection info
    mName = name;
    serverIP = ip;
    connected = true;
    std::cout << "Logged in as " << mName << std::endl;
    //std::cout << "Exiting TCPConnect" << std::endl << std::endl;
}

TCPsocket& NetworkManagerClient::getSocket()
{
    return this->TCPServerSock;
}

bool NetworkManagerClient::isOnline()
{
    return connected;
}

void NetworkManagerClient::resetReadyState()
{
    //std::cout << "Entering resetReadyState" << std::endl << std::endl;
    Packet outgoing;
    outgoing.type = READY;
    outgoing.message = const_cast<char*>("RESET");
    NetworkUtil::TCPSend(TCPServerSock, NetworkUtil::PacketToCharArray(outgoing));
    //std::cout << "Exiting resetReadyState" << std::endl << std::endl;
}

void NetworkManagerClient::quit()
{
    //std::cout << "Entering quit" << std::endl << std::endl << std::endl;
    Packet outgoing;

    outgoing.type = CONNECTION;
    outgoing.message = const_cast<char*>("QUIT");
	while(!NetworkUtil::TCPSend(TCPServerSock, NetworkUtil::PacketToCharArray(outgoing))); // FIXME: what if server crashed?

/*	//FIXME: Server doesn't receive any UDP packets.
    UDPpacket *UDPPack = SDLNet_AllocPacket(65535);
    if(!UDPPack)
    {
        std::cerr << "SDLNet_AllocPacket done goofed: " << SDLNet_GetError() << std::endl;
        return;
    }
	char *msg = NetworkUtil::PacketToCharArray(outgoing);
    UDPPack->data = (Uint8*)msg;
    UDPPack->len = strlen(msg) + 1;
    UDPPack->address = serverIP;
    NetworkUtil::UDPSend(UDPServerSock, -1, UDPPack);
    SDLNet_FreePacket(UDPPack);
*/

    connected = false;
    std::cout << "You have quit the game." << std::endl;
    //std::cout << "Exiting quit" << std::endl << std::endl << std::endl;
}

void NetworkManagerClient::sendPlayerInput(ISpaceShipController* controller)
{
    //std::cout << "Entering sendPlayerInput" << std::endl << std::endl;
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
    
    char *out = NetworkUtil::PacketToCharArray(outgoing);
	NetworkUtil::TCPSend(TCPServerSock, out);

/*	//FIXME: Server doesn't receive any UDP packets.
    UDPpacket *UDPPack = SDLNet_AllocPacket(sizeof(int)+1);
    if(!UDPPack)
    {
            std::cout << "SDLNet_AllocPacket done goofed: " << SDLNet_GetError() << std::endl;
            return;
    }
    UDPPack->data = (Uint8*)out;
    UDPPack->len = strlen(out) + 1;
    UDPPack->address = serverIP;
    NetworkUtil::UDPSend(UDPServerSock, -1, UDPPack);
	std::cout << "UDPSend player input." << std::endl;
    SDLNet_FreePacket(UDPPack);
*/
    //std::cout << "Exiting sendPlayerInput" << std::endl << std::endl;
}

void NetworkManagerClient::receiveData(Ogre::SceneManager* sceneManager, SoundManager* sound, std::vector<Mineral*>& minerals, std::vector<SpaceShip*>& spaceships, std::vector<GameObject*>& walls)
{
    std::cout << "Entering receiveData" << std::endl << std::endl;
    static int iii = 0;
    Packet outgoing;
	Packet infoPacket;
    outgoing.type = STATE;
    outgoing.message = const_cast<char*>("NONE");
    char* incoming = NULL;
    char* out = NetworkUtil::PacketToCharArray(outgoing);

    std::cout << "About to request and request data" << std::endl << std::endl;
    if(NetworkUtil::TCPSend(TCPServerSock, out) && NetworkUtil::TCPReceive(TCPServerSock, &incoming)) {
        infoPacket = NetworkUtil::charArrayToPacket(incoming);
        std::cout << iii++ << ": " << infoPacket.message << std::endl << std::endl;
    }

/*	//FIXME: Fix the other UDP bugs first.
	UDPpacket *UDPPack = SDLNet_AllocPacket(65535);
	if(!UDPPack)
	{
		std::cerr << "SDLNet_AllocPacket done goofed: " << SDLNet_GetError() << std::endl;
		return;
	}

	if(NetworkUtil::UDPReceive(UDPServerSock, UDPPack) > 0)
	{
		std::cout << "Received UDP Packet." << std::endl;
		infoPacket = NetworkUtil::charArrayToPacket((char*)UDPPack->data);
		SDLNet_FreePacket(UDPPack);
	}
	else
		return;
*/


                        /*
                        for(int i = 0; i < packs; ++i)
                        {
                            //std::cout << "We are about to receive packet number " << i << "." << std::endl;
                            if(NetworkUtil::TCPReceive(TCPServerSock, &incoming)) {
                                //std::cout << "We received packet number " << i << "." << std::endl;
                                std::string msg(incoming);
                                std::cout << "---" << msg << std::endl;
                                Packet in = NetworkUtil::charArrayToPacket(incoming);
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
    std::cout << "Exiting receiveData" << std::endl << std::endl;
}

