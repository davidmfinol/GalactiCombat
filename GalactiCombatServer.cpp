#include "GalactiCombatServer.h"

//-------------------------------------------------------------------------------------
GalactiCombatServer::GalactiCombatServer(bool v) : clients(0), state(PAUSE), scoreboard(), verbose(v)
{
    if(SDL_Init(0) != 0)
    {
        std::cerr << "SDL_Init done goofed: " << SDL_GetError() << std::endl;
        exit(1);
    }
    
    if(SDLNet_Init() != 0)
        std::cerr << "SDLNet_Init done goofed: " << SDLNet_GetError() << std::endl;
}
//-------------------------------------------------------------------------------------
GalactiCombatServer::~GalactiCombatServer()
{
    SDLNet_Quit();
    SDL_Quit();
}
//-------------------------------------------------------------------------------------
std::vector<Client*> GalactiCombatServer::getClients()
{
    return clients;
}
//-------------------------------------------------------------------------------------
int GalactiCombatServer::findClientBySocket(TCPsocket sock)
{
    int i;
    for(i = 0; i < clients.size(); i++)
        if(clients[i]->sock == sock)
            return i;
    std::cerr << "Failed to find with findClientBySocket" << std::endl << std::endl;
    return -1;
}
//-------------------------------------------------------------------------------------
int GalactiCombatServer::findClientByName(std::string name)
{
    int i;
    for(i = 0; i < clients.size(); i++)
        if(clients[i]->name == name)
            return i;
    std::cerr << "Failed to find with findClientByName - " << name << std::endl << std::endl;
    return -1;
}
//-------------------------------------------------------------------------------------
int GalactiCombatServer::findClientByChannel(int channel)
{
    int i;
    for(i = 0; i < clients.size(); i++)
        if(clients[i]->channel == channel)
            return i;
    std::cerr << "Failed to find with findClientByChannel - "<<channel<<std::endl<<std::endl;
    return -1;
}
//-------------------------------------------------------------------------------------
Client* GalactiCombatServer::addClient(TCPsocket sock, int channel, std::string name)
{
    if(verbose) std::cout << "Entering addClient - " << name << std::endl;
    
    double pos_x = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
    double pos_z = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
    double pos_y = (std::rand() % (ROOM_SIZE - 500)) + 250;
    
    clients.push_back(new Client());
    clients.back()->sock = sock;
    clients.back()->channel = channel;
    clients.back()->name = name;
    clients.back()->ship = new SpaceShip(name, new ClientSpaceShipController(), mSceneMgr->getRootSceneNode(), NULL, pos_x, pos_y, pos_z, 300);
    clients.back()->ready = false;
    spaceShips.push_back(clients.back()->ship);
    physicsSimulator->addGameObject(spaceShips.back(), RESTITUTION, true, false);
    
    if(verbose) std::cout << "Exiting addClient -" << name << std::endl << std::endl;
    return(clients.back());
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::removeClient(int i)
{
    if(verbose) std::cout << "Entering removeClient - " << i << std::endl;
    if(i < 0 && i >= clients.size())
        return;
    
    // FIXME: Remove THE SPACESHIP from the game
    //physicsSimulator->removeGameObject(spaceShips[i]);
    //delete spaceShips[i];
    //delete clients[i]->ship->getController();
    //spaceShips.erase(spaceShips.begin() + i);
    
    //Unbind the UDP socket
    //SDLNet_UDP_Unbind(UDPServerSock, clients[i]->channel);
    
    // Remove the client from our list of clients
    SDLNet_TCP_Close(clients[i]->sock);
    delete clients[i];
    clients.erase(clients.begin() + i);
    
    if(verbose) std::cout << "Exiting removeClient - " << i << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::sendToAll(char *buf, bool useTCP)
{
    if(verbose) std::cout << "Entering sendToAll" << std::endl;
    useTCP = true;	//NOTE: temporary
    //UDPpacket *UDPPack;//TODO
    
    if(!buf || !clients.size())
        return;
    
    for(int cindex = 0; cindex < clients.size(); )
    {
        if(useTCP)
        {
            if(NetworkUtil::TCPSend(clients[cindex]->sock, buf)) {
                //if(verbose) std::cout << "Sent message '" << buf << "' to " << clients[cindex]->name << std::endl;
                cindex++;
            }
            else
            {
                std::cerr << "Failed to send message '" << buf << "' to " << clients[cindex]->name << std::endl;
                this->removeClient(cindex);
                std::cerr << "Disconnected" << std::endl;
            }
        }
		/*
        else //using UDP
        {
                UDPPack = SDLNet_AllocPacket(65535);
                UDPPack->channel = clients[cindex]->channel;
                UDPPack->data = (Uint8*)buf;
                UDPPack->len = strlen(buf) + 1;
                NetworkUtil::UDPSend(UDPServerSock, clients[cindex]->channel, UDPPack);
                if(verbose) std::cout << "UDPSend."<<std::endl;
                SDLNet_FreePacket(UDPPack);
        }
		*/
    }
    
    if(verbose) std::cout << "Exiting sendToAll" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
SDLNet_SocketSet GalactiCombatServer::createSockSet()
{
    if(verbose) std::cout << "Entering createSockSet" << std::endl;
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
//	SDLNet_UDP_AddSocket(set, UDPServerSock);	//TODO: Game hangs on login
    
    if(verbose) std::cout << "Exiting createSockSet" << std::endl << std::endl;
    return set;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::createServerMinerals()
{
    if(verbose) std::cout << "Entering createServerMinerals" << std::endl;
    int radius, i, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z;
    std::srand (std::time(NULL));
    for (i = 0; i < MINERALS_AMOUNT; i++)
    {
        std::ostringstream o;
        o << "ServerMineral" << i;
        pos_x = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
        pos_z = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
        pos_y = (std::rand() % (ROOM_SIZE - 500)) + 250;
        radius = (std::rand() % (Mineral::MAX_RADIUS - Mineral::MIN_RADIUS + 1)) + Mineral::MIN_RADIUS;
        vel_x = ((std::rand() % 400) + 600) * (std::rand() % 2 == 0 ? 1 : -1);
        vel_y = ((std::rand() % 400) + 600) * (std::rand() % 2 == 0 ? 1 : -1);
        vel_z = ((std::rand() % 400) + 600) * (std::rand() % 2 == 0 ? 1 : -1);
        minerals[i] = new Mineral(o.str(), mSceneMgr->getRootSceneNode(), NULL, pos_x, pos_y, pos_z, radius);
        physicsSimulator->addGameObject(minerals[i], RESTITUTION);
        physicsSimulator->setGameObjectVelocity(minerals[i], Ogre::Vector3(vel_x, vel_y, vel_z));
    }
   if(verbose) std::cout << "Exiting createServerMinerals" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::createServerRoom()
{
    if(verbose) std::cout << "Entering createServerRoom" << std::endl;
    // create ground
    walls[0] = new GameObject ("ground", mSceneMgr->getRootSceneNode(), NULL, 0, 0, 0, 0, "UNIT_Y");
    physicsSimulator->addGameObject(walls[0]);
    
    // create ceiling
    walls[1] = new GameObject ("ceiling", mSceneMgr->getRootSceneNode(), NULL, 0, ROOM_SIZE, 0, 0, "NEGATIVE_UNIT_Y");
    physicsSimulator->addGameObject(walls[1]);
    
    // create front wall
    walls[2] = new GameObject ("front", mSceneMgr->getRootSceneNode(), NULL, ROOM_SIZE/2, ROOM_SIZE/2, 0, 0, "NEGATIVE_UNIT_X");
    physicsSimulator->addGameObject(walls[2]);
    
    // create back wall
    walls[3] = new GameObject ("back", mSceneMgr->getRootSceneNode(), NULL, -ROOM_SIZE/2, ROOM_SIZE/2, 0, 0, "UNIT_X");
    physicsSimulator->addGameObject(walls[3]);
    
    // create left wall
    walls[4] = new GameObject ("left", mSceneMgr->getRootSceneNode(), NULL, 0, ROOM_SIZE/2, -ROOM_SIZE/2, 0, "UNIT_Z");
    physicsSimulator->addGameObject(walls[4]);
    
    // create right wall
    walls[5] = new GameObject ("right", mSceneMgr->getRootSceneNode(), NULL, 0, ROOM_SIZE/2, ROOM_SIZE/2, 0, "NEGATIVE_UNIT_Z");
    physicsSimulator->addGameObject(walls[5]);
    if(verbose) std::cout << "Exiting createServerRoom" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::startServer(long portNo)
{
    if(verbose) std::cout << "Entering startServer" << std::endl;
    IPaddress ip; //32-bit IPv4 host, 16-bit port
    const char *host = NULL;
    Uint16 port = (Uint16)portNo;
    
    isServer = true;
    
    if(NetworkUtil::ResolveHost(&ip, NULL, port)==-1) exit(3);
    
    TCPServerSock = NetworkUtil::TCPOpen(&ip);
    if(!TCPServerSock) exit(4);

    /*
	UDPServerSock = NetworkUtil::UDPOpen(0);
    if(!UDPServerSock) exit(4);
    */

    //set up the game environment
    if(verbose) std::cout << "Setting up game...." << std::endl;
    mRoot = new Ogre::Root();
    chooseSceneManager();
    this->createServerMinerals();
    this->createServerRoom();
    
    host = SDLNet_ResolveIP(&ip);
    if(host == NULL)
        host = "N/A";
    if(verbose) std::cout << "Host: " << host << std::endl;
    
    if(verbose) std::cout << "Exiting startServer" << std::endl << std::endl;
	std::cout<<"Ready!"<<std::endl;
    serverLoop();
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::serverLoop(void)
{
    if(verbose) std::cout << "Entering serverLoop" << std::endl;
    timespec prevTime, currTime;
    clock_gettime(CLOCK_MONOTONIC, &prevTime);
    while(1)
    {
        if(verbose) std::cout << "==============Server Loop Run=============" << std::endl;
        
        //run the game loop
        if(state == PLAY)
        {
            clock_gettime(CLOCK_MONOTONIC, &currTime);
            float elapsedTime = (currTime.tv_sec*1000000000 + currTime.tv_nsec) - (prevTime.tv_sec*1000000000 + prevTime.tv_nsec);
            elapsedTime /= 1000000000; //convert from nanoseconds to seconds
            if(elapsedTime < TIME_STEP/2)
                std::cerr << "Elapsed time between iterations of game loop too small.... Skipping iteration." << std::endl;
            else if(elapsedTime < MAX_SUB_STEPS*TIME_STEP) 
            {
                if(verbose) std::cout << "Running the Game loop with elapsed time: " << elapsedTime << std::endl;
                gameLoop(elapsedTime);
                if(verbose) std::cout << "Game loop has been run." << std::endl;
                prevTime = currTime;
            }
            else
            {
                std::cerr << "Elapsed time between iterations of game loop too LARGE. Skipping iteration." << std::endl;
                prevTime = currTime;
            }
        }
        
        //get sockets ready for connection
        if(verbose) std::cout << "Getting SocketSet." << std::endl;
        SDLNet_SocketSet set = this->createSockSet();
        if(verbose) std::cout << "Checking Number of Sockets Ready..." << std::endl;
        int numReady = NetworkUtil::CheckSockets(set, (Uint32) - 1); //NOTE: This will block until there is some network activity.
        if(verbose) std::cout << "Number of Sockets Ready: " << numReady << std::endl;
        if(numReady == -1) break;
        if(!numReady) continue;
        
        //listen for new connections
        if(verbose) std::cout << "Checking this server's socket." << std::endl;
        if(SDLNet_SocketReady(TCPServerSock))
        {
            numReady--;
            this->listenForConnections();
        }
        
        //for each connection, receive data
        for(int i = 0; numReady && i < clients.size(); i++)
        {
            if(verbose) std::cout << "Checking " << clients[i]->name << "'s socket." << std::endl;
            if(SDLNet_SocketReady(clients[i]->sock))
            {
                numReady--;
                this->receiveData(i);
            } else {
                //FIXME: THIS SHOULD CHECK FOR DISCONNECTS, BUT DOESN'T WORK:
                //std::cerr << clients[i]->name << "'s socket wasn't ready." << std::endl;
                //this->removeClient(i);
            }
        }
/*		//TODO: UDP messages
		for(int i = 0; i < clients.size(); i++)
		{
			if(verbose) std::cout<<"Checking for UDP messages."<<std::endl;
			this->receiveData(-1);
		}
*/
    }
    if(verbose) std::cout << "Exiting serverLoop" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::listenForConnections() 
{
    if(verbose) std::cout << "Entering listenForConnections" << std::endl;
    TCPsocket TCPsock = SDLNet_TCP_Accept(TCPServerSock);
    if(TCPsock)
    {
        char *msg = NULL;
        if(NetworkUtil::TCPReceive(TCPsock, &msg))
        {
            if(verbose) std::cout << "Received new connection with message: " <<  msg << std::endl;
            Packet pack = NetworkUtil::charArrayToPacket(msg);
            if(pack.type != CONNECTION) //FIXME: THIS SOMETIMES HAPPENS? WHY?
            {
                std::cerr << "Connection Error. Someone sent something other than a CONNECTION packet as a new socket." << std::endl;
                return;
            }
            
            //bind UDP
            //IPaddress *clientIP = SDLNet_TCP_GetPeerAddress(TCPsock);
            //int channel = NetworkUtil::UDPBind(UDPServerSock, -1, clientIP);
            //if(channel == -1) exit(4);	//error has occurred
            int channel = -1;
            //add the client
            std::string name = pack.message;
            free(pack.message);
            // TODO: FIXME: add some checks for repeated names. Could cause crash
            Client* client = this->addClient(TCPsock, channel, name);
            
            if(verbose) std::cout << name << " has logged in!" << std::endl;
//			if(verbose) std::cout << name << " has been bound to channel " << channel << "." << std::endl;
            if(verbose) std::cout << clients.size() << " players are logged in." << std::endl;
        }
        else
        {
            std::cerr << "Failed to receive from new connection. Closing connection." << std::endl;
            SDLNet_TCP_Close(TCPsock);
        }
    }
    if(verbose) std::cout << "Exiting listenForConnections" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receiveData(int clientIndex)
{
    if(verbose) std::cout << "Entering receiveData" << std::endl;
    char *msg = NULL;
    Packet incoming;
    incoming.message = NULL;
    
/*	//TODO: Not receiving client's UDP packets.
	if(clientIndex == -1)
	{
	    UDPpacket *UDPPack = NetworkUtil::AllocPacket(65535);
	    if(!UDPPack) return;
		int received = NetworkUtil::UDPReceive(UDPServerSock, UDPPack);
	    //receive data from a socket, depending on the transport protocol being used
	    if(received > 0)
	    {
	            incoming = NetworkUtil::charArrayToPacket((char*)UDPPack->data);
				int channel = UDPPack->channel;
				clientIndex = this->findClientByChannel(channel);
				std::cout<<"Received UDP message from "<<clients[clientIndex]->name<<": "<<incoming.message<<std::endl;
				SDLNet_FreePacket(UDPPack);
	    }
		else
			return;
	}
	else
	{
*/
    if(NetworkUtil::TCPReceive(clients[clientIndex]->sock, &msg))
    {
        incoming = NetworkUtil::charArrayToPacket(msg);
        if(verbose) std::cout << "Received TCP message from " << clients[clientIndex]->name << ": " << msg << std::endl;
        free(msg);
        msg = NULL;
    }
    else
        if(verbose) std::cout << "Did not receive TCP message from " << clients[clientIndex]->name << std::endl;
//}
    
    //process the received message
    switch(incoming.type)
    {
        case CONNECTION:
            receiveConnectionPacket(clientIndex, incoming); break;
        case STATE:
            receiveStatePacket(clientIndex, incoming); break;
        case PLAYERINPUT:
            receivePlayerInputPacket(clientIndex, incoming); break;
        case PLAYERROTATION:    
            receivePlayerRotationPacket(clientIndex, incoming); break;
        case READY:
            receiveReadyPacket(clientIndex, incoming); break;
        case WALLS:
            receiveWallsPacket(clientIndex, incoming); break;
        case SCORE:
            receiveScorePacket(clientIndex, incoming); break;
        default:
            std::cerr << "Unrecognized Packet type from " << clients[clientIndex]->name << std::endl;
    }
    if(incoming.message)
        free(incoming.message);
    
    if(verbose) std::cout << "Exiting receiveData" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receiveConnectionPacket(int clientIndex, Packet& incoming)
{
    if(verbose) std::cout << "Entering receiveConnectionPacket" << std::endl;
    if(!strcmp(incoming.message, "QUIT"))
    {
        if(verbose) std::cout << clients[clientIndex]->name << " has quit the game!" << std::endl;
        this->removeClient(clientIndex); 
    }
    else
        std::cerr << "Received a non-quit connection packet from " << clients[clientIndex]->name << " after first connection." << std::endl;
   if(verbose) std::cout << "Exiting receiveConnectionPacket" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receiveStatePacket(int clientIndex, Packet& incoming)
{
    if(verbose) std::cout << "Entering receiveStatePacket" << std::endl;
    Packet outgoing;
    outgoing.type = INFO;
    std::stringstream ss;

    // pack the minerals
    ss << "minerals:" << minerals.size() << ",";
    for (int in = 0; in < minerals.size(); in++) {
        std::string name = minerals[in]->getName();
        Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(minerals[in]);
        Ogre::Quaternion rot = physicsSimulator->getGameObjectOrientation(minerals[in]);
        double radius = minerals[in]->getRadius();
        char buffer[100];
        sprintf(buffer,"%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,", const_cast<char*>(name.c_str()), pos.x, pos.y, pos.z, rot.w, rot.x, rot.y, rot.z, radius);
        ss << buffer;
    }

    // pack the spaceships
    ss << "spaceships:" << spaceShips.size() << ",";
    for (int in = 0; in < spaceShips.size(); in++) {
        std::string name = spaceShips[in]->getName();
        Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(spaceShips[in]);
        Ogre::Quaternion rot = physicsSimulator->getGameObjectOrientation(spaceShips[in]);
        double size = spaceShips[in]->getSize();
        char buffer[100];
        sprintf(buffer,"%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,", const_cast<char*>(name.c_str()), pos.x, pos.y, pos.z, rot.w, rot.x, rot.y, rot.z, size);
        ss << buffer;
    }

    // pack the bullets
    ss << "bullets:" << bullets.size() << ",";
    for (std::list<Bullet*>::iterator it = bullets.begin(); it != bullets.end(); ++it) {
        std::string name = (*it)->getName();
        Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(*it);
        char buffer[100];
        sprintf(buffer,"%s,%.1f,%.1f,%.1f,", const_cast<char*>(name.c_str()), pos.x, pos.y, pos.z);
        ss << buffer;
    }

    // pack the message
    char* packetMessage = (char*)malloc(ss.str().length() + 1);
    strcpy(packetMessage, ss.str().c_str());
    outgoing.message = packetMessage;
    
    if(verbose) std::cout << "The state info is:             " << outgoing.message << std::endl;

/*	//TODO: UDP still broken

	UDPpacket *UDPPack = NetworkUtil::AllocPacket(65535);
	if(!UDPPack) return;

	char *msg = NetworkUtil::PacketToCharArray(outgoing);
	UDPPack->data = (Uint8*)msg;
	UDPPack->len = strlen(msg)+1;
	NetworkUtil::UDPSend(UDPServerSock, clients[clientIndex]->channel, UDPPack);
	if(verbose) std::cout<<"UDPSend game state."<<std::endl;
	SDLNet_FreePacket(UDPPack);
*/

    // send the message
    char* outgoingMessage = NetworkUtil::PacketToCharArray(outgoing);
    if(!NetworkUtil::TCPSend(clients[clientIndex]->sock, outgoingMessage))
        std::cerr << "Didn't send back info to " << clients[clientIndex]->name << std::endl;
    else
        if(verbose) std::cout << "-------Sent back info to " << clients[clientIndex]->name << std::endl;
    
    // clean-up
    free(outgoingMessage);
    free(packetMessage);
    if(verbose) std::cout << "Exiting receiveStatePacket" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receivePlayerInputPacket(int clientIndex, Packet& incoming)
{
    if(verbose) std::cout << "Entering receivePlayerInputPacket" << std::endl;
    char input = *incoming.message;
    if(verbose) std::cout << "The input from " << clients[clientIndex]->name << " is " << input << std::endl;
    ((ClientSpaceShipController*)clients[clientIndex]->ship->getController())->injectInput(input);
    if(verbose) std::cout << "Exiting receivePlayerInputPacket" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receivePlayerRotationPacket(int clientIndex, Packet& incoming)
{
    if(verbose) std::cout << "Entering receivePlayerRotationPacket" << std::endl;
    
    std::string message(incoming.message);
    Ogre::Real w = atof(message.substr(0, message.find(",")).c_str());
    message = message.substr(message.find(",") + 1);
    Ogre::Real x = atof(message.substr(0, message.find(",")).c_str());
    message = message.substr(message.find(",") + 1);
    Ogre::Real y = atof(message.substr(0, message.find(",")).c_str());
    message = message.substr(message.find(",") + 1);
    Ogre::Real z = atof(message.substr(0, message.find(",")).c_str());
    
    if(verbose) std::cout << "The rotation from " << clients[clientIndex]->name << " is " << w << "," << x << "," << y << "," << z << std::endl;
    physicsSimulator->setGameObjectOrientation(clients[clientIndex]->ship, Ogre::Quaternion(w, x, y, z));
    
    if(verbose) std::cout << "Exiting receivePlayerRotationPacket" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receiveReadyPacket(int clientIndex, Packet& incoming)
{
    if(verbose) std::cout << "Entering receiveReadyPacket" << std::endl;
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
        if(!NetworkUtil::TCPSend(clients[clientIndex]->sock, out))
            std::cerr << "Didn't send back lobby list to " << clients[clientIndex]->name << std::endl;
        else
            if(verbose) std::cout << "Sent back lobby list: " << out << std::endl;
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
    if(verbose) std::cout << "Exiting receiveReadyPacket" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receiveWallsPacket(int clientIndex, Packet& incoming)
{
    if(verbose) std::cout << "Entering receiveWallsPacket" << std::endl;
    // FIXME: THIS PACKET TYPE DOESN'T SEEM TO BE NECESSARY
    if(verbose) std::cout << "Exiting receiveWallsPacket" << std::endl << std::endl;
}
//-------------------------------------------------------------------------------------
void GalactiCombatServer::receiveScorePacket(int clientIndex, Packet& incoming)
{
    if(verbose) std::cout << "Entering receiveScorePacket" << std::endl;
    //TODO: THIS PACKET TYPE NEEDS TO BE REDONE
    if(verbose) std::cout << "Exiting receiveScorePacket" << std::endl;
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
        bool verbose = false;
        if(argc > 1)
        {
            if(strcmp(argv[1],"-v") || strcmp(argv[1],"--verbose"))
			{
				verbose = true;
	            std::cout << "Verbose Enabled" << std::endl;
			}
			/*//TODO: Execution never reaches here.
			else if(strcmp(argv[1],"-h") || strcmp(argv[1],"--help"))
			{
				std::cout<<std::endl<<"Usage: ./GalactiCombatServer [OPTION]"<<std::endl;
				std::cout<<"  -h, --help       display this help text"<<std::endl;
				std::cout<<"  -v, --verbose    print debug statements"<<std::endl;
				return 0;
			}
			*/
        }
        GalactiCombatServer *server = new GalactiCombatServer(verbose);
        server->startServer(TCP_PORT);
        server->~GalactiCombatServer();
        return 0;
    }   
    
    #ifdef __cplusplus
}
#endif
