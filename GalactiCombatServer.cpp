#include "GalactiCombatServer.h"

GalactiCombatServer::GalactiCombatServer() : clients(0), state(PAUSE), scoreboard()
{
    if(SDL_Init(0) != 0)
    {
        std::cerr << "SDL_Init done goofed: " << SDL_GetError() << std::endl;
        exit(1);
    }
    
    if(SDLNet_Init() != 0)
        std::cerr << "SDLNet_Init done goofed: " << SDLNet_GetError() << std::endl;
}

GalactiCombatServer::~GalactiCombatServer()
{
    SDLNet_Quit();
    SDL_Quit();
}

std::vector<Client*> GalactiCombatServer::getClients()
{
    return clients;
}

Client* GalactiCombatServer::findClientBySocket(TCPsocket sock)
{
    for(int i = 0; i < clients.size(); i++)
        if(clients[i]->sock == sock)
            return clients[i];
    std::cerr << "Failed to find with findClientBySocket" << std::endl << std::endl;
    return 0;
}

Client* GalactiCombatServer::findClientByName(std::string name)
{
    for(int i = 0; i < clients.size(); i++)
        if(clients[i]->name == name)
            return clients[i];
    std::cerr << "Failed to find with findClientByName - " << name << std::endl << std::endl;
    return 0;
}

Client* GalactiCombatServer::addClient(TCPsocket sock, int channel, std::string name)
{
    std::cout << "Entering addClient - " << name << std::endl;
    
    clients.push_back(new Client());
    clients.back()->sock = sock;
    clients.back()->channel = channel;
    clients.back()->name = name;
    clients.back()->inputController = new ClientSpaceShipController();
    clients.back()->ready = false;
    
    double pos_x = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
    double pos_z = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
    double pos_y = (std::rand() % (ROOM_HIGH - 500)) + 250;
    spaceShips.push_back(new SpaceShip(name, mSoundMgr, clients.back()->inputController,
                                       mSceneMgr->getRootSceneNode()->createChildSceneNode(name + "ParentNode", Ogre::Vector3(pos_x, pos_y, pos_z)), NULL, 30 ));
    physicsSimulator->addGameObject(spaceShips.back(), RESTITUTION, true, true);
    
    std::cout << "Exiting addClient -" << name << std::endl << std::endl;
    return(clients.back());
}

void GalactiCombatServer::removeClient(int i)
{
    std::cout << "Entering removeClient - " << i << std::endl;
    if(i < 0 && i >= clients.size())
        return;
    
    // Remove it from the game
    physicsSimulator->removeGameObject(spaceShips[i]);
    delete spaceShips[i];
    spaceShips.erase(spaceShips.begin() + i);
    
    //Unbind the UDP socket
    SDLNet_UDP_Unbind(UDPServerSock, clients[i]->channel);
    
    // Remove it from our list of clients
    SDLNet_TCP_Close(clients[i]->sock);
    delete clients[i]->inputController;
    delete clients[i];
    clients.erase(clients.begin() + i);
    
    std::cout << "Exiting removeClient - " << i << std::endl << std::endl;
}

void GalactiCombatServer::sendToAll(char *buf, bool useTCP)
{
    std::cout << "Entering sendToAll" << std::endl;
    useTCP = true;	//NOTE: temporary
    UDPpacket *UDPPack;
    
    if(!buf || !clients.size())
        return;
    
    for(int cindex = 0; cindex < clients.size(); )
    {
        if(useTCP)
        {
            if(NetworkUtil::TCPSend(clients[cindex]->sock, buf)) {
                //std::cout << "Sent message '" << buf << "' to " << clients[cindex]->name << std::endl;
                cindex++;
            }
            else
            {
                std::cerr << "Failed to send message '" << buf << "' to " << clients[cindex]->name << std::endl;
                this->removeClient(cindex);
                std::cerr << "Disconnected" << std::endl;
            }
        }
        else //using UDP
            {
                UDPPack = SDLNet_AllocPacket(65535);
                UDPPack->channel = clients[cindex]->channel;
                UDPPack->data = (Uint8*)buf;
                UDPPack->len = strlen(buf) + 1;
                NetworkUtil::UDPSend(UDPServerSock, clients[cindex]->channel, UDPPack);
				std::cout << "UDPSend."<<std::endl;
                SDLNet_FreePacket(UDPPack);
            }
    }
    
    std::cout << "Exiting sendToAll" << std::endl << std::endl;
}

SDLNet_SocketSet GalactiCombatServer::createSockSet()
{
    std::cout << "Entering createSockSet" << std::endl;
    static SDLNet_SocketSet set = NULL;
    
    if(set)
        SDLNet_FreeSocketSet(set);
    set = SDLNet_AllocSocketSet(clients.size() + 1);
    if(!set)
    {
        std::cerr << "SDLNet_AllocSocketSet done goofed: " << SDLNet_GetError() << std::endl;
        exit(1);
    }
    SDLNet_TCP_AddSocket(set, TCPServerSock);
    for(int i = 0; i < clients.size(); i++)
        SDLNet_TCP_AddSocket(set, clients[i]->sock);
    
    std::cout << "Exiting createSockSet" << std::endl << std::endl;
    return set;
}

void GalactiCombatServer::createServerMinerals()
{
    std::cout << "Entering createServerMinerals" << std::endl;
    int radius, i, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z;
    std::srand (std::time(NULL));
    for (i = 0; i < MINERALS_AMOUNT; i++)
    {
        std::ostringstream o;
        o << "RandomSphere" << i;
        pos_x = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
        pos_z = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
        pos_y = (std::rand() % (ROOM_HIGH - 500)) + 250;
        radius = (std::rand() % (Mineral::MAX_RADIUS - Mineral::MIN_RADIUS + 1)) + Mineral::MIN_RADIUS;
        vel_x = ((std::rand() % 400) + 600) * (std::rand() % 2 == 0 ? 1 : -1);
        vel_y = ((std::rand() % 400) + 600) * (std::rand() % 2 == 0 ? 1 : -1);
        vel_z = ((std::rand() % 400) + 600) * (std::rand() % 2 == 0 ? 1 : -1);
        minerals[i] = new Mineral(o.str(), mSoundMgr, mSceneMgr->getRootSceneNode(), NULL, pos_x, pos_y, pos_z, radius);
        physicsSimulator->addGameObject(minerals[i], RESTITUTION);
        physicsSimulator->setGameObjectVelocity(minerals[i], Ogre::Vector3(vel_x, vel_y, vel_z));
    }
   std::cout << "Exiting createServerMinerals" << std::endl << std::endl;
}

void GalactiCombatServer::createServerRoom()
{
    std::cout << "Entering createServerRoom" << std::endl;
    // create ground
    walls[0] = new GameObject ("ground", mSceneMgr->getRootSceneNode(), NULL, 0, 0, 0, 0, "UNIT_Y");
    physicsSimulator->addGameObject(walls[0]);
    
    // create ceiling
    walls[1] = new GameObject ("ceiling", mSceneMgr->getRootSceneNode(), NULL, ROOM_HIGH, 0, 0, 0, "NEGATIVE_UNIT_Y");
    physicsSimulator->addGameObject(walls[1]);
    
    // create front wall
    walls[2] = new GameObject ("front", mSceneMgr->getRootSceneNode(), NULL, ROOM_SIZE/2, ROOM_HIGH/2, 0, 0, "NEGATIVE_UNIT_X");
    physicsSimulator->addGameObject(walls[2]);
    
    // create back wall
    walls[3] = new GameObject ("back", mSceneMgr->getRootSceneNode(), NULL, -ROOM_SIZE/2, ROOM_HIGH/2, 0, 0, "UNIT_X");
    physicsSimulator->addGameObject(walls[3]);
    
    // create left wall
    walls[4] = new GameObject ("left", mSceneMgr->getRootSceneNode(), NULL, ROOM_HIGH/2, -ROOM_SIZE/2, 0, 0, "UNIT_Z");
    physicsSimulator->addGameObject(walls[4]);
    
    // create right wall
    walls[5] = new GameObject ("right", mSceneMgr->getRootSceneNode(), NULL, ROOM_HIGH/2, ROOM_SIZE/2, 0, 0, "NEGATIVE_UNIT_Z");
    physicsSimulator->addGameObject(walls[5]);
    std::cout << "Exiting createServerRoom" << std::endl << std::endl;
}

void GalactiCombatServer::startServer(long portNo)
{
    std::cout << "Entering startServer" << std::endl;
    IPaddress ip; //32-bit IPv4 host, 16-bit port
    const char *host = NULL;
    Uint16 port = (Uint16)portNo;
    
    isServer = true;
    
    if(SDLNet_ResolveHost(&ip, NULL, port) == -1)
    {
        std::cerr << "SDLNet_ResolveHost done goofed: " << SDLNet_GetError() << std::endl;
        exit(3);
    }
    
    TCPServerSock = SDLNet_TCP_Open(&ip);
    if(!TCPServerSock)
    {
        std::cerr << "SDLNet_TCP_Open done goofed: " << SDLNet_GetError() << std::endl;
        exit(4);
    }
    
    UDPServerSock = SDLNet_UDP_Open(0);
    if(!UDPServerSock)
    {
        std::cerr << "SDLNet_UDP_Open done goofed: " << SDLNet_GetError() << std::endl;
        exit(4);
    }
    
    //set up the game environment
    std::cout << "Setting up game...." << std::endl;
    mRoot = new Ogre::Root();
    chooseSceneManager();
    this->createServerRoom();
    this->createServerMinerals();
    
    host = SDLNet_ResolveIP(&ip);
    if(host == NULL)
        std::cout << "Host: N/A" << std::endl;
    else
        std::cout << "Host: " << host << std::endl;
    
    std::cout << "Exiting startServer" << std::endl << std::endl;
    serverLoop();
}

void GalactiCombatServer::serverLoop(void)
{
    std::cout << "Entering serverLoop" << std::endl;
    while(1)
    {
        std::cout << "==============Server Loop Run=============" << std::endl;
        static std::clock_t prev_t = std::clock();
        std::clock_t curr_t = std::clock();
        
        //run the game loop
        if(state == PLAY)
        {
            std::cout << "Running the Game loop." << std::endl;
            float elapsedTime = ( ((float)curr_t) - ((float)prev_t) ) / CLOCKS_PER_SEC;;
            gameLoop(elapsedTime);
            //std::cout << "Game loop has been run" << std::endl;
            
            // Debugging
            //std::cout << "There are " << spaceShips.size() << " SpaceShips in the scene" << std::endl;
            //for(i = 0; i < spaceShips.size(); i++)
            //{
            //    Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(spaceShips[i]);
                //std::cout << "Player is at " << pos.x << " " << pos.y << " " << pos.z << std::endl;
            //}
        }
        prev_t = curr_t;
        
        //get sockets ready for connection
        std::cout << "Getting SocketSet." << std::endl;
        SDLNet_SocketSet set = this->createSockSet();
        std::cout << "Checking Number of Sockets Ready..." << std::endl;
        int numReady = SDLNet_CheckSockets(set, (Uint32) - 1); //NOTE: This will block until there is some network activity.
        std::cout << "Number of Sockets Ready: " << numReady << std::endl;
        if(numReady == -1)
        {
            std::cerr << "SDLNet_CheckSockets done goofed: " << SDLNet_GetError() << std::endl;
            break;
        }
        if(!numReady)
            continue;
        
        //listen for new connections
        std::cout << "Checking this server's socket." << std::endl;
        if(SDLNet_SocketReady(TCPServerSock))
        {
            numReady--;
            this->listenForConnections();
        }
        
        //for each connection, receive data
        for(int i = 0; numReady && i < clients.size(); i++)
        {
            std::cout << "Checking " << clients[i]->name << "'s socket." << std::endl;
            if(SDLNet_SocketReady(clients[i]->sock))
            {
                numReady--;
                this->receiveData(i);
            } else {
                std::cerr << clients[i]->name << "'s socket wasn't ready." << std::endl;
                //FIXME: THIS SHOULD CHECK FOR DISCONNECTS, BUT DOESN'T WORK:
                //this->removeClient(i);
            }
        }
    }
    std::cout << "Exiting serverLoop" << std::endl << std::endl;
}

void GalactiCombatServer::listenForConnections() 
{
    std::cout << "Entering listenForConnections" << std::endl;
    TCPsocket TCPsock = SDLNet_TCP_Accept(TCPServerSock);
    if(TCPsock)
    {
        char *msg = NULL;
        if(NetworkUtil::TCPReceive(TCPsock, &msg))
        {
            std::cout << "Received new connection with message: " <<  msg << std::endl;
            Packet pack = NetworkUtil::charArrayToPacket(msg);
            if(pack.type != CONNECTION) //FIXME: THIS SOMETIMES HAPPENS? WHY?
            {
                std::cerr << "Connection Error. Someone sent something other than a CONNECTION packet as a new socket." << std::endl;
                return;
            }
            
            //bind UDP
            IPaddress *clientIP = SDLNet_TCP_GetPeerAddress(TCPsock);
            int channel = SDLNet_UDP_Bind(UDPServerSock, -1, clientIP);
            if(channel == -1)
            {
                std::cerr << "SDLNet_UDP_Bind done goofed: " << SDLNet_GetError() << std::endl;
                exit(4);
            }
            
            //add the client
            std::string name = pack.message;
            // TODO: FIXME: add some checks for repeated names. Could cause crash
            Client* client = this->addClient(TCPsock, channel, name);
            
            std::cout << name << " has logged in!" << std::endl;
            std::cout << name << " has been bound to channel " << channel << "." << std::endl;
            std::cout << clients.size() << " players are logged in." << std::endl;
        }
        else
        {
            std::cerr << "Failed to receive from new connection. Closing connection." << std::endl;
            SDLNet_TCP_Close(TCPsock);
        }
    }
    std::cout << "Exiting listenForConnections" << std::endl << std::endl;
}

void GalactiCombatServer::receiveData(int clientIndex)
{
    std::cout << "Entering receiveData" << std::endl;
    char *msg = NULL;
    Packet incoming;
/*	//FIXME: Not receiving client's UDP packets.
    UDPpacket *UDPPack = SDLNet_AllocPacket(65535);
    if(!UDPPack)
    {
            std::cerr << "SDLNet_AllocPacket done goofed: " << SDLNet_GetError() << std::endl;
            return;
    }

    //receive data from a socket, depending on the transport protocol being used
    if(NetworkUtil::UDPReceive(UDPServerSock, UDPPack) > 0)
    {
            std::cout << "Received UDP packet from client " << clients[clientIndex]->name << std::endl;
            incoming = NetworkUtil::charArrayToPacket((char*)UDPPack->data);
			SDLNet_FreePacket(UDPPack);
    }

    else if(NetworkUtil::TCPReceive(clients[clientIndex]->sock, &msg))
*/
	if(NetworkUtil::TCPReceive(clients[clientIndex]->sock, &msg))
    {
        incoming = NetworkUtil::charArrayToPacket(msg);
        std::cout << "Received TCP message from " << clients[clientIndex]->name << ": " << msg << std::endl;
        free(msg);
        msg = NULL;
    }


    //process the received message
    switch(incoming.type)
    {
        case CONNECTION:
            receiveConnectionPacket(clientIndex, incoming); break;
        case STATE:
            receiveStatePacket(clientIndex, incoming); break;
        case PLAYERINPUT:
            receivePlayerInputPacket(clientIndex, incoming); break;
        case SCORE:
            receiveScorePacket(clientIndex, incoming); break;
        case READY:
            receiveReadyPacket(clientIndex, incoming); break;
        default:
            std::cerr << "Unrecognized Packet type from " << clients[clientIndex]->name << std::endl;
    }
    std::cout << "Exiting receiveData" << std::endl << std::endl;
}

void GalactiCombatServer::receiveConnectionPacket(int clientIndex, Packet& incoming)
{
    std::cout << "Entering receiveConnectionPacket" << std::endl;
    if(!strcmp(incoming.message, "QUIT"))
    {
        std::cout << clients[clientIndex]->name << " has quit the game!" << std::endl;
        this->removeClient(clientIndex); 
    }
    else
        std::cerr << "Received a non-quit connection packet from " << clients[clientIndex]->name << " after first connection." << std::endl;
   std::cout << "Exiting receiveConnectionPacket" << std::endl << std::endl;
}

void GalactiCombatServer::receiveStatePacket(int clientIndex, Packet& incoming)
{
    std::cout << "Entering receiveStatePacket" << std::endl;
    Packet out;
    out.type = INFO;
    std::stringstream ss;

    ss << "minerals:" << minerals.size() << ",";
    for (int in = 0; in < minerals.size(); in++) {
        std::string name = minerals[in]->getName();
        Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(minerals[in]);
        Ogre::Vector3 vel = physicsSimulator->getGameObjectVelocity(minerals[in]);
        Ogre::Quaternion rot = physicsSimulator->getGameObjectOrientation(minerals[in]);
        double radius = minerals[in]->getRadius();
        char buffer[100];
        sprintf(buffer,"%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,", const_cast<char*>(name.c_str()), pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, rot.w, rot.x, rot.y, rot.z, radius);
        ss << buffer;
    }

    ss << "spaceships:" << spaceShips.size() << ",";
    for (int in = 0; in < spaceShips.size(); in++) {
        std::string name = spaceShips[in]->getName();
        Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(spaceShips[in]);
        Ogre::Vector3 vel = physicsSimulator->getGameObjectVelocity(spaceShips[in]);
        Ogre::Quaternion rot = physicsSimulator->getGameObjectOrientation(spaceShips[in]);
        double size = spaceShips[in]->getSize();
        char buffer[100];
        sprintf(buffer,"%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f", const_cast<char*>(name.c_str()), pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, rot.w, rot.x, rot.y, rot.z, size);
        ss << buffer;
    }

    out.message = const_cast<char*>(ss.str().c_str());

    std::cout << "StateMessage:     " << out.message << std::endl;

/*	FIXME: Still workin on it
	UDPpacket *UDPPack = SDLNet_AllocPacket(65535);
	if(!UDPPack)
	{
		std::cout<<"SDLNet_AllocPacket done goofed: " << SDLNet_GetError() << std::endl;
		return;
	}

	char *msg = NetworkUtil::PacketToCharArray(out);
	UDPPack->data = (Uint8*)msg;
	UDPPack->len = strlen(msg)+1;
	NetworkUtil::UDPSend(UDPServerSock, clients[clientIndex]->channel, UDPPack);
	std::cout<<"UDPSend game state."<<std::endl;
	SDLNet_FreePacket(UDPPack);
*/

    if(NetworkUtil::TCPSend(clients[clientIndex]->sock, NetworkUtil::PacketToCharArray(out)))
        std::cout << "Sent back info to " << clients[clientIndex]->name << std::endl;
    else
        std::cerr << "Didn't send back info to " << clients[clientIndex]->name << std::endl;

    std::cout << "Exiting receiveStatePacket" << std::endl << std::endl;
}

void GalactiCombatServer::receivePlayerInputPacket(int clientIndex, Packet& incoming)
{
    std::cout << "Entering receivePlayerInputPacket" << std::endl;
    char input = *incoming.message;
    std::cout << "The input from " << clients[clientIndex]->name << " is " << input << std::endl;
    clients[clientIndex]->inputController->injectInput(input);
    std::cout << "Exiting receivePlayerInputPacket" << std::endl << std::endl;
}

void GalactiCombatServer::receiveScorePacket(int clientIndex, Packet& incoming)
{
    std::cout << "Entering receiveScorePacket" << std::endl;
    //TODO: THIS PACKET TYPE NEEDS TO BE REDONE
    std::cout << "Exiting receiveScorePacket" << std::endl;
}

void GalactiCombatServer::receiveReadyPacket(int clientIndex, Packet& incoming)
{
    std::cout << "Entering receiveReadyPacket" << std::endl;
    if(!strcmp(incoming.message, "LIST_REQUEST"))
    {
        int readyCount = 0;
        std::string result("Game will start when all players (more than one) are ready:\n\n");
        for (int count = 0; count < clients.size(); count++) {
            std::stringstream ss;
            std::string name(clients[count]->name);
            std::string ready;
            if (clients[count]->ready) {
                ready = "READY!";
                readyCount++;
            }
            else ready = "Preparing";
            ss << name << ".................." << ready << "\n";
            result += ss.str();
        }
        
        if((readyCount == clients.size()) && (clients.size() != 1))
            result += "\nAll players ready, game is starting in:\n";

        char* out = const_cast<char*>(result.c_str());
        if(NetworkUtil::TCPSend(clients[clientIndex]->sock, out))
            std::cout << "Sent back lobby list: " << out << std::endl;
        else
            std::cerr << "Didn't send back lobby list: " << std::endl;
    }
    else if(!strcmp(incoming.message, "READY")) {
        clients[clientIndex]->ready = true;
    }
    else if(!strcmp(incoming.message, "PREPARING")) {
        clients[clientIndex]->ready = false;
    }
    else if(!strcmp(incoming.message, "RESET")) {
        for (int count = 0; count < clients.size(); count++) {
            clients[count]->ready = false;
            state = PAUSE;
        }
    }
    else if(!strcmp(incoming.message, "START")) {
        state = PLAY;
    }
    std::cout << "Exiting receiveReadyPacket" << std::endl << std::endl;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
    #endif
    
    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
    #else
    int main(int argc, char *argv[])
    #endif
    {   
        GalactiCombatServer *server = new GalactiCombatServer();
        server->startServer(TCP_PORT);
        server->~GalactiCombatServer();
        return 0;
    }   
    
    #ifdef __cplusplus
}
#endif
