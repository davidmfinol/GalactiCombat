#include "GalactiCombatServer.h"

GalactiCombatServer::GalactiCombatServer() : clients(0), state(PAUSE), scoreboard(), lastFrameTime(std::time(0))
{
    if(SDL_Init(0) != 0) {
        printf("SDL_Init done goofed: %s\n", SDL_GetError());
        exit(1);
    }
    
    if(SDLNet_Init() != 0) {
        printf("SDLNet_Init done goofed: %s\n",SDLNet_GetError());
    }
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
    int i;
    for(i = 0; i < clients.size(); i++)
        if(clients[i]->sock == sock)
            return clients[i];
        return 0;
}

Client* GalactiCombatServer::findClientByName(std::string name)
{
    int i;
    for(i = 0; i < clients.size(); i++)
        if(clients[i]->name == name)
            return clients[i];
        return 0;
}

Client* GalactiCombatServer::addClient(TCPsocket sock, std::string name)
{
    printf("Adding client %s\n", const_cast<char*>(name.c_str()));
    
    clients.push_back(new Client());
    clients.back()->sock = sock;
    clients.back()->name = name;
    clients.back()->inputController = new ClientSpaceShipController();
    clients.back()->ready = false;
    
    double pos_x = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
    double pos_z = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1);
    double pos_y = (std::rand() % (ROOM_HIGH - 500)) + 250;
    spaceShips.push_back(new SpaceShip(name, mSoundMgr, clients.back()->inputController,
                                       mSceneMgr->getRootSceneNode()->createChildSceneNode(name + "ParentNode", Ogre::Vector3(pos_x, pos_y, pos_z)), NULL, 30 ));
    physicsSimulator->addGameObject(spaceShips.back(), RESTITUTION, true, true);
    
    return(clients.back());
}

void GalactiCombatServer::removeClient(int i)
{
    if(i < 0 && i >= clients.size())
        return;
    
    // Remove it from the game
    physicsSimulator->removeGameObject(spaceShips[i]);
    delete spaceShips[i];
    spaceShips.erase(spaceShips.begin() + i);
    
    // Remove it from our list of clients
    SDLNet_TCP_Close(clients[i]->sock);
    delete clients[i]->inputController;
    delete clients[i];
    clients.erase(clients.begin() + i);
}

/*
 *        sendToAll(char*, bool):
 *        This method takes the data stored in the address given in the first argument
 *        and sends it to all the clients that are connected.
 * 
 *        buf: the data to be sent to everyone
 *        TCP: a boolean that answers the question "Do you want to send this using TCP?"
 *                At the time this comment was written, UDPSend has not been written, so 
 *                always set this to true.
 */
void GalactiCombatServer::sendToAll(char *buf, bool TCP)
{
    TCP = true;	//NOTE: temporary
    
    if(!buf || !clients.size())
        return;
    
    for(int cindex = 0; cindex < clients.size(); )
    {
        if(TCP)
        {
            if(TCPSend(clients[cindex]->sock, buf)) {
                std::cout << "Sent message '" << buf << "' to " << clients[cindex]->name << std::endl;
                cindex++;
            }
            else
            {
                std::cerr << "Failed to send message '" << buf << "' to " << clients[cindex]->name << std::endl;
                this->removeClient(cindex);
                std::cerr << "Disconnected" << std::endl;
            }
        }
        else //UDP
            {
                //TODO: implement UDPSend
            }
    }
}




// This method returns an SDLNet_SocketSet containing the server socket and all the client sockets.
SDLNet_SocketSet GalactiCombatServer::createSockSet()
{
    static SDLNet_SocketSet set = NULL;
    int i;
    
    if(set)
        SDLNet_FreeSocketSet(set);
    set = SDLNet_AllocSocketSet(clients.size() + 1);
    if(!set){
        std::cerr << "SDLNet_AllocSocketSet done goofed: " << SDLNet_GetError() << std::endl;
        exit(1);
    }
    SDLNet_TCP_AddSocket(set, TCPServerSock);
    for(i = 0; i < clients.size(); i++)
        SDLNet_TCP_AddSocket(set, clients[i]->sock);
    return set;
}

void GalactiCombatServer::createServerMinerals()
{
    int radius, i, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z;
    std::srand (std::time(NULL));
    for (i = 0; i < MINERALS_AMOUNT; i++) {
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
    std::cout << "Minerals created." << std::endl;
}

void GalactiCombatServer::createServerRoom()
{
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
    
    std::cout << "Room built." << std::endl;
}

void GalactiCombatServer::startServer(long portNo)
{
    IPaddress ip; //32-bit IPv4 host, 16-bit port
    //	char *message = NULL;
    const char *host = NULL;
    Uint32 ipaddr;
    Uint16 port;
    
    port = (Uint16)portNo;
    
    isServer = true;
    
    if(SDLNet_ResolveHost(&ip, NULL, port) == -1)
    {
        printf("SDLNet_ResolveHost done goofed: %s\n", SDLNet_GetError());
        exit(3);
    }
    
    ipaddr = SDL_SwapBE32(ip.host);
    
    printf("IP Address: %d.%d.%d.%d\n",
           ipaddr>>24,
           (ipaddr>>16)&0xff,
           (ipaddr>>8)&0xff,
           ipaddr&0xff);
    //printf("Port: %d\n", port);
    host = SDLNet_ResolveIP(&ip);
    if(host == NULL)
        printf("Host: N/A\n");
    else
        printf("Host: %s\n", host);
    
    TCPServerSock = SDLNet_TCP_Open(&ip);
    if(!TCPServerSock)
    {
        printf("SDLNet_TCP_Open done goofed: %s\n", SDLNet_GetError());
        exit(4);
    }
    
    //set up the game environment
    printf("Setting up game....\n");
    printf("Creating OgreRoot.\n");
    mRoot = new Ogre::Root();
    printf("Choosing Scene Manager.\n");
    chooseSceneManager();
    printf("Creating Room.\n");
    this->createServerRoom();
    printf("Creating Minerals.\n");
    this->createServerMinerals();
    
    printf("Ready for connections.\n");
    
    serverLoop();
}

void GalactiCombatServer::serverLoop(void)
{
    printf("Starting server loop.\n");
    TCPsocket TCPsock;
    UDPsocket UDPsock;
    UDPpacket *out, *in;
    SDLNet_SocketSet set;
    
    //main loop
    while(1)
    {
        int numReady, i;
        set = this->createSockSet();
        numReady = SDLNet_CheckSockets(set, (Uint32) - 1);
        if(numReady == -1)
        {
            printf("SDLNet_CheckSockets done goofed: %s\n", SDLNet_GetError());
            break;
        }
        
        if(!numReady)
            continue;
        
        //listen for connections
        if(SDLNet_SocketReady(TCPServerSock))
        {
            numReady--;
            this->listenForConnections();
        }
        
        //for each connection, receive data
        for(i = 0; numReady && i < clients.size(); i++)
        {
            //printf("Checking sockets\n");
            if(SDLNet_SocketReady(clients[i]->sock))
            {
                //printf("%s's socket is read!\n", clients[i]->name);
                char *msg = NULL;
                std::string score;
                if(TCPReceive(clients[i]->sock, &msg))
                {
                    printf("Received message from client %s\n", const_cast<char*>(clients[i]->name.c_str()));
                    numReady--;
                    Packet incoming = charArrayToPacket(msg);
                    this->receiveData(incoming, i);
                    printf("Message processed  from %s\n", const_cast<char*>(clients[i]->name.c_str()));
                }
                free(msg);
                msg = NULL;
                
            } else {
                //FIXME: THIS SHOULD CHECK FOR DISCONNECTS, BUT DOESN'T WORK:
                //printf("Client disconnected.\n");
                //this->removeClient(i);
            }
        }
        
        //run the game loop
        std::time_t currentTime = std::time(0);
        std::time_t elapsedTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        if(state == PLAY)
        {
            std::cout << "Running the Game loop" << std::endl;
            gameLoop(elapsedTime);
            std::cout << "Game loop has been run" << std::endl;
            
            // Debugging
            std::cout << "There are " << spaceShips.size() << " SpaceShips in the scene" << std::endl;
            for(i = 0; i < spaceShips.size(); i++)
            {
                Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(spaceShips[i]);
                std::cout << "Player is at " << pos.x << " " << pos.y << " " << pos.z << std::endl;
            }
            
            //inform the clients of the status of the game
            std::cout << "Sending Minerals" << std::endl;
            for(i = 0; i < minerals.size(); i++)
                sendMineral(minerals[i]);
            std::cout << "Sending Spaceships" << std::endl;
            for(i = 0; i < spaceShips.size(); ++i)
                sendSpaceShip(spaceShips[i]);
            std::cout << "Players Updated" << std::endl;
        }
    }//end loop
    printf("Shutting down.\n");
}

void GalactiCombatServer::listenForConnections() 
{
    TCPsocket TCPsock = SDLNet_TCP_Accept(TCPServerSock);
    if(TCPsock)
    {
        char *msg = NULL;
        if(TCPReceive(TCPsock, &msg))
        {
            printf("Received a message: %s\n", msg);
            Client *client;
            Packet pack = charArrayToPacket(msg);
            
            if(pack.type != CONNECTION)
            {
                printf("Connection Error. Someone sent something other than a CONNECTION packet as a new socket.\n");
                return;
            }
            
            //add the client
            std::string name = pack.message;
            // TODO: add some checks for repeated names. Could cause crash
            client = this->addClient( TCPsock, const_cast<char*>(name.c_str()) );
            
            printf("%s has logged in.\n", const_cast<char*>(client->name.c_str()));
            printf("%d players have logged in.\n", (int)clients.size());
        }
        else
        {
            printf("Closing connection.\n");
            SDLNet_TCP_Close(TCPsock);
        }
    }
}

void GalactiCombatServer::receiveData(const Packet &incoming, int i)
{
    if(incoming.type == CONNECTION)
    {
        if(!strcmp(incoming.message, "QUIT"))
        {
            printf("%s has quit the game!\n", const_cast<char*>(clients[i]->name.c_str()));
            this->removeClient(i); 
        }
        else
            printf("WAT R U DOING\n");
    }
    else if(incoming.type == SCORE)
    {
        printf("%s ended the game with a score of %s!\n", const_cast<char*>(clients[i]->name.c_str()), incoming.message);
        
        std::stringstream ss;
        std::string name(clients[i]->name);
        std::string score = incoming.message;
        ss << scoreboard << name << "," << score << ";";
        
        scoreboard = ss.str();
        Packet outgoing;
        outgoing.type = SCORE;
        outgoing.message = const_cast<char*>(scoreboard.c_str());
        char *out = PacketToCharArray(outgoing);
        if(TCPSend(clients[i]->sock, out))
            printf("Sent back scoreboard: %s\n", out);
        else
            printf("Didn't send scoreboard\n");
    }
    else if(incoming.type == PLAYERINPUT)
    {
        char input = *incoming.message;
        printf("The input is: %d\n", input);
        clients[i]->inputController->injectInput(input);
    }
    else if(incoming.type == READY)
    {
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
            if(TCPSend(clients[i]->sock, out))
                printf("Sent back lobby list: %s\n", out);
            else
                printf("Didn't send lobby list\n");
        }
        else if(!strcmp(incoming.message, "READY")) {
            clients[i]->ready = true;
        }
        else if(!strcmp(incoming.message, "PREPARING")) {
            clients[i]->ready = false;
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
    }
}

void GalactiCombatServer::sendMineral(Mineral* mineral)
{
    Packet outgoing;
    outgoing.type = MINERAL;
    
    std::string name = mineral->getName();
    Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(mineral);
    Ogre::Vector3 vel = physicsSimulator->getGameObjectVelocity(mineral);
    Ogre::Quaternion rot = physicsSimulator->getGameObjectOrientation(mineral);
    double radius = mineral->getRadius();
    
    char buffer[100];
    sprintf(buffer,"%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f", const_cast<char*>(name.c_str()), pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, rot.w, rot.x, rot.y, rot.z, radius);
    outgoing.message = buffer;
    char* out = PacketToCharArray(outgoing);
    sendToAll(out, true);
}

void GalactiCombatServer::sendSpaceShip(SpaceShip* spaceShip)
{
    Packet outgoing;
    outgoing.type = SPACESHIP;
    
    std::string name = spaceShip->getName();
    Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(spaceShip);
    Ogre::Vector3 vel = physicsSimulator->getGameObjectVelocity(spaceShip);
    Ogre::Quaternion rot = physicsSimulator->getGameObjectOrientation(spaceShip);
    double size = spaceShip->getSize();
    
    //FIXME: MAYBE WE CAN FIND A BETTER WAY TO DO THIS?
    char buffer[100];
    sprintf(buffer,"%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f", const_cast<char*>(name.c_str()), pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, rot.w, rot.x, rot.y, rot.z, size);
    outgoing.message = buffer;
    char* out = PacketToCharArray(outgoing);
    sendToAll(out, true);
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
        server->startServer(5172);
        server->~GalactiCombatServer();
        return 0;
    }   
    
    #ifdef __cplusplus
}
#endif
