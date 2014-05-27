#include "NetworkManagerClient.h"

//-------------------------------------------------------------------------------------
NetworkManagerClient::NetworkManagerClient(void) : connected(false)
{
    if(SDL_Init(0) != 0) {
        std::cerr << "SDL_Init done goofed: " << SDL_GetError() << std::endl;
        exit(1);
    }
    
    if(SDLNet_Init() != 0)
        std::cerr << "SDLNet_Init done goofed: " << SDLNet_GetError() << std::endl;
}
//-------------------------------------------------------------------------------------
NetworkManagerClient::~NetworkManagerClient(void)
{
    SDLNet_Quit();
    SDL_Quit();
}
//-------------------------------------------------------------------------------------
int NetworkManagerClient::connect(char *host, char *name)
{
    ////std::cout << "Entering connect" << std::endl << std::endl;
    Uint16 port = (Uint16) TCP_PORT;
    
    // Resolve the argument into an IPaddress type 
    ////std::cout << "Connecting to " << host << " port " << port << std::endl;
    IPaddress ip;
    if(NetworkUtil::ResolveHost(&ip, host, port)==-1) {
        SDLNet_Quit();
        //SDL_Quit();
        std::string exception = "fail_to_connect";
        throw exception;
    }
    
    // open the TCP socket
    ////std::cout << "Opening TCP server socket." << std::endl;
    TCPServerSock = NetworkUtil::TCPOpen(&ip);
    if(!TCPServerSock) {
        SDLNet_Quit();
        //SDL_Quit();
        std::string exception = "fail_to_connect";
        throw exception;
    }
	/*
	set = SDLNet_AllocSocketSet(1);
	if(!set){
		std::cerr<<"SDLNet_AllocSocketSet done goofed: "<<SDLNet_GetError()<<std::endl;
		std::string exception = "fail_to_connect";
		SDLNet_Quit();
		throw exception;
	}
	if(SDLNet_TCP_AddSocket(set, TCPServerSock) == -1){
		std::cerr<<"SDLNet_TCP_AddSocket done goofed: "<<SDLNet_GetError()<<std::endl;
		std::string exception = "fail_to_connect";
		SDLNet_Quit();
		throw exception;
	}
	*/	
   /* 
    // open the UDP socket
    ////std::cout << "Opening UDP server socket." << std::endl;
    UDPServerSock = NetworkUtil::UDPOpen(0);
    if(!UDPServerSock)
    {
        SDLNet_Quit();
        SDL_Quit();
        std::string exception = "fail_to_connect";
        throw exception;
    }
    */
    // login with a name
    Packet pack;
    pack.type = CONNECTION;
    pack.message = name; // FIXME: SHOULD USE? :char message[MAXLEN];
    char* out = NetworkUtil::PacketToCharArray(pack);
    if(!NetworkUtil::TCPSend(TCPServerSock, out)) {
        SDLNet_TCP_Close(TCPServerSock);
        std::cerr<<"Something done goofed when trying to connect to server."<<std::endl;
        exit(8);
    }
    ////std::cout << "Sent " << out << std::endl;
    free(out);
    
    // store our connection info
    mName = name;
    serverIP = ip;
    connected = true;
    ////std::cout << "Exiting TCPConnect" << std::endl << std::endl;
	return 0;
}
//-------------------------------------------------------------------------------------
TCPsocket& NetworkManagerClient::getSocket()
{
    return this->TCPServerSock;
}
//-------------------------------------------------------------------------------------
bool NetworkManagerClient::isOnline()
{
    return connected;
}
//-------------------------------------------------------------------------------------
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
    char *out = NetworkUtil::PacketToCharArray(outgoing);

    if(NetworkUtil::TCPSend(TCPServerSock, out) && NetworkUtil::TCPReceive(TCPServerSock, &incoming)) {
        ////std::cout << "Receving: " << incoming << std::endl;
        Packet pack = NetworkUtil::charArrayToPacket(incoming);
        if(pack.type != SCORE) {   
            ////std::cout << "Error in sendPlayerScore() in NetworkManagerClient.cpp. Score not received from server." << std::endl;
            scores = ""; 
        }   
        else{
            ////std::cout << "Received message: " << pack.message << std::endl;
            scores = pack.message;
            free(pack.message); 
        }
    }   
    else {
        connected = false;
        scores = ""; 
    }
    free(incoming);
    free(out);   
}

//-------------------------------------------------------------------------------------
std::string NetworkManagerClient::getPlayerScores()
{
    return scores; //"name,score;"
}
//-------------------------------------------------------------------------------------
void NetworkManagerClient::resetReadyState()
{
    ////std::cout << "Entering resetReadyState" << std::endl << std::endl;
    Packet outgoing;
    outgoing.type = READY;
    outgoing.message = const_cast<char*>("RESET");
    
    char* out = NULL;
    out = NetworkUtil::PacketToCharArray(outgoing);
    NetworkUtil::TCPSend(TCPServerSock, out);
    free(out);
    
    ////std::cout << "Exiting resetReadyState" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void NetworkManagerClient::quit()
{
    ////std::cout << "Entering quit" << std::endl << std::endl << std::endl;
    Packet outgoing;
    outgoing.type = CONNECTION;
    outgoing.message = const_cast<char*>("QUIT");

    char* out = NetworkUtil::PacketToCharArray(outgoing);
    NetworkUtil::TCPSend(TCPServerSock, out);
    free(out);

    connected = false;
    ////std::cout << "Exiting quit" << std::endl << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void NetworkManagerClient::sendPlayerInput(ISpaceShipController* controller)
{
    ////std::cout << "Entering sendPlayerInput" << std::endl << std::endl;
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
    char* out = NetworkUtil::PacketToCharArray(outgoing);

    ////std::cout << "Sending input" << std::endl;
    NetworkUtil::TCPSend(TCPServerSock, out);
    ////std::cout << "Sent input" << std::endl;
/*	//TODO: Use TCP to send input instead?
    UDPpacket *UDPPack = NetworkUtil::AllocPacket(sizeof(int)+1);
    if(!UDPPack) return;

    UDPPack->data = (Uint8*)out;
    UDPPack->len = strlen(out) + 1;
    UDPPack->address = serverIP;
    NetworkUtil::UDPSend(UDPServerSock, -1, UDPPack);
    ////std::cout << "UDPSend player input." << std::endl;
    SDLNet_FreePacket(UDPPack);
*/
    free(out);
    ////std::cout << "Exiting sendPlayerInput" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void NetworkManagerClient::sendPlayerRotation(const Ogre::Quaternion& rotation)
{
    ////std::cout << "Entering sendPlayerRotation" << std::endl << std::endl;
    Packet outgoing;
    outgoing.type = PLAYERROTATION;
    
    char buffer[100];
    sprintf(buffer,"%f,%f,%f,%f", rotation.w, rotation.x, rotation.y, rotation.z);
    outgoing.message = buffer;
    
    char* out = NetworkUtil::PacketToCharArray(outgoing);
    NetworkUtil::TCPSend(TCPServerSock, out);
    
    free(out);
    ////std::cout << "Exiting sendPlayerRotation" << std::endl << std::endl;
}

//-------------------------------------------------------------------------------------
void NetworkManagerClient::receiveData()
{
    if(!NetworkUtil::CheckSockets(set, 0)) return;
    ////std::cout<<"Entering receiveData."<<std::endl;
    char *inc = NULL;
    Packet incoming;
    NetworkUtil::TCPReceive(TCPServerSock, &inc);
    incoming = NetworkUtil::charArrayToPacket(inc);
    if(incoming.type == CONNECTION)
    {
        ////std::cout << incoming.message << std::endl;
    }
    free(inc);
    if(incoming.message)
        free(incoming.message);
    ////std::cout<<"Exiting receiveData."<<std::endl;
}
//-------------------------------------------------------------------------------------
void NetworkManagerClient::requestGameState(Ogre::SceneManager* sceneManager, std::vector<Mineral*>& minerals, std::vector<SpaceShip*>& spaceships, std::list<Bullet*>& bullets)
{
    ////std::cout << "Entering requestGameState" << std::endl << std::endl;
    static int iii = 0;
    Packet outgoing;
    Packet infoPacket;
    outgoing.type = STATE;
    outgoing.message = const_cast<char*>("");
    char* incoming = NULL;
    char* out = NetworkUtil::PacketToCharArray(outgoing);
/*
	//TODO: Use TCP to send request, but use UDP to receive data.
	UDPpacket *UDPPack = NetworkUtil::AllocPacket(65535);
	if(!UDPPack) return;

	if(NetworkUtil::UDPReceive(UDPServerSock, UDPPack) > 0)
	{
		////std::cout << "Received UDP Packet." << std::endl;
		infoPacket = NetworkUtil::charArrayToPacket((char*)UDPPack->data);
		SDLNet_FreePacket(UDPPack);
	}
	else
		return;
*/
    if(NetworkUtil::TCPSend(TCPServerSock, out) && NetworkUtil::TCPReceive(TCPServerSock, &incoming)) {
        infoPacket = NetworkUtil::charArrayToPacket(incoming);
        ////std::cout << iii++ << ": " << infoPacket.message << std::endl << std::endl;
        std::string message(infoPacket.message);

        // Minerals
        int mineralsAmount = atoi(message.substr(message.find(":") + 1, message.find(",")).c_str());
        for (int i = 0; i < mineralsAmount; i++)
        {
            message = message.substr(message.find(",") + 1);
            std::string name = message.substr(0, message.find(","));
            message = message.substr(message.find(",") + 1);
            double pos_x = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_y = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_z = atof(message.substr(0, message.find(",")).c_str());
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
            for(int j = 0; j < minerals.size(); ++j)
            {
                ////std::cout << "Checking to see if " << name << " already exists." << std::endl;
                if(minerals[j]->getName() == name)
                {
                    ////std::cout << "Exists." << std::endl;
                    found = true;
                    minerals[j]->getSceneNode()->setPosition(pos_x, pos_y, pos_z);
                    minerals[j]->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                    minerals[j]->adjustRadius(radius - minerals[j]->getRadius());
                    break;
                }
            }
            if(!found)
            {
                ////std::cout << "Doesn't exist, create it." << std::endl;
                minerals.push_back(new Mineral(name, sceneManager->getRootSceneNode(), pos_x, pos_y, pos_z, radius));
                minerals.back()->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
            }
        }

        // SpaceShips
        message = message.substr(message.find(",") + 1);
        int spaceShipAmount = atoi(message.substr(message.find(":") + 1, message.find(",")).c_str());
        for (int i = 0; i < spaceShipAmount; i++)
        {
            message = message.substr(message.find(",") + 1);
            std::string name = message.substr(0, message.find(","));
            message = message.substr(message.find(",") + 1);
            double pos_x = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_y = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_z = atof(message.substr(0, message.find(",")).c_str());
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
                spaceships[0]->getSceneNode()->setPosition(pos_x, pos_y, pos_z);
                spaceships[0]->adjustSize(size - spaceships[0]->getSize());
                continue;
            }
            for(int j = 0; j < spaceships.size(); ++j)
            {
                ////std::cout << "Checking to see if " << name << " already exists." << std::endl;
                if(spaceships[j]->getName() == name)
                {
                    ////std::cout << "Exists." << std::endl;
                    found = true;
                    spaceships[j]->getSceneNode()->setPosition(pos_x, pos_y, pos_z);
                    spaceships[j]->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
                    spaceships[j]->adjustSize(size - spaceships[j]->getSize());
                    break;
                }
            }
            if(!found) {
                ////std::cout << "Doesn't exist, create it." << std::endl;
                ISpaceShipController* controller = new ClientSpaceShipController();
                spaceships.push_back(new SpaceShip(name, controller, sceneManager->getRootSceneNode()->createChildSceneNode(), pos_x, pos_y, pos_z, size));
                spaceships.back()->getSceneNode()->setOrientation(rot_w, rot_x, rot_y, rot_z);
            }
        }

        // Bullets
        for(std::list<Bullet*>::iterator it = bullets.begin(); it != bullets.end(); ++it)
            (*it)->setExist(false);
        
        message = message.substr(message.find(",") + 1);
        int bulletsAmount = atoi(message.substr(message.find(":") + 1, message.find(",")).c_str());
        for (int i = 0; i < bulletsAmount; ++i) {
            message = message.substr(message.find(",") + 1);
            std::string name = message.substr(0, message.find(","));
            message = message.substr(message.find(",") + 1);
            double pos_x = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_y = atof(message.substr(0, message.find(",")).c_str());
            message = message.substr(message.find(",") + 1);
            double pos_z = atof(message.substr(0, message.find(",")).c_str());

            // FIXME: THIS IS BAD, oh well
            bool found = false;
            for(std::list<Bullet*>::iterator it = bullets.begin(); it != bullets.end(); ++it) {
                ////std::cout << "Checking to see if " << name << " already exists." << std::endl;
                if((*it)->getName() == name) {
                    ////std::cout << "Exists." << std::endl;
                    found = true;
                    (*it)->getSceneNode()->setPosition(pos_x, pos_y, pos_z);
                    (*it)->setExist(true);
                    break;
                }
            }
            if(!found) {
                ////std::cout << "Doesn't exist, create it." << std::endl;
                Bullet* newBullet = new Bullet(name, sceneManager->getRootSceneNode(), NULL, pos_x, pos_y, pos_z);		
                newBullet->setExist(true);
                bullets.push_back(newBullet);
            }
        }
        
        // Energy
        message = message.substr(message.find(",") + 1);
        spaceships[0]->setEnergy(atof(message.c_str()));
    }
    free(out);
    free(incoming);
    if(infoPacket.message)
        free(infoPacket.message);

    ////std::cout << "Exiting requestGameState" << std::endl << std::endl;
}

