#include "PhysicsSimulator.h"

//-------------------------------------------------------------------------------------
PhysicsSimulator::PhysicsSimulator(int maxSphereSize)
{
    // The following initialization is required by bullet
    // It is based off the HelloWorld.cpp Demo from bullet
    // See the comments for other initialization options
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, 0, 0));
    
    // We cache all possible shapes here for better performance
    defineCollisionShapes(maxSphereSize);
}
//-------------------------------------------------------------------------------------
PhysicsSimulator::~PhysicsSimulator(void) 
{
    // Destroy the game objects
    for(std::map<GameObject*, btRigidBody*>::iterator it=gameObjects.begin(); it!=gameObjects.end(); ++it) {
        this->removeGameObject(it->first);
        this->deleteGameObject(it->first);
    }
    gameObjects.clear();
    rigidBodies.clear();
    
    // Destroy the cached shapes
    for(std::map<std::string, btCollisionShape*>::iterator it=collisionShapes.begin(); it!=collisionShapes.end(); ++it) 
        delete it->second;
    collisionShapes.clear();
    
    // Delete physics
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::defineCollisionShapes(int maxSphereSize)
{
    // Default box
    collisionShapes["Box"] = new btBoxShape(btVector3(btScalar(1.), btScalar(1.), btScalar(1.)));
    // Planes
    collisionShapes["UNIT_X"] = new btStaticPlaneShape(btVector3(btScalar(1.),btScalar(0.),btScalar(0.)), btScalar(1.));
    collisionShapes["NEGATIVE_UNIT_X"] = new btStaticPlaneShape(btVector3(btScalar(-1.),btScalar(0.),btScalar(0.)), btScalar(1.));
    collisionShapes["UNIT_Y"] = new btStaticPlaneShape(btVector3(btScalar(0.),btScalar(1.),btScalar(0.)), btScalar(1.));
    collisionShapes["NEGATIVE_UNIT_Y"] = new btStaticPlaneShape(btVector3(btScalar(0.),btScalar(-1.),btScalar(0.)), btScalar(1.));
    collisionShapes["UNIT_Z"] = new btStaticPlaneShape(btVector3(btScalar(0.),btScalar(0.),btScalar(1.)), btScalar(1.));
    collisionShapes["NEGATIVE_UNIT_Z"] = new btStaticPlaneShape(btVector3(btScalar(0.),btScalar(0.),btScalar(-1.)), btScalar(1.));
    // Spheres
    for(int i = 1; i <= maxSphereSize; ++i) {
        std::ostringstream m;
        m << "Sphere" << i;
        collisionShapes[m.str()] = new btSphereShape(btScalar(i));
    }
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::addGameObject (GameObject* obj, double restitution, bool activeKinematic, bool allowRotation) 
{
    // Determine shape
    std::map<std::string, btCollisionShape*>::iterator it = collisionShapes.find(obj->getShapeName());
    if(it==collisionShapes.end()) 
    {
        std::cerr << "Error: Shape for " << obj->getName() << " not defined. Game Object not added." << std::endl;
        return;
    }
    btCollisionShape* shape = it->second;
    
    // Determine mass and inertia
    int mass = obj->getMass();
    btVector3 inertia(0,0,0);
    bool isDynamic = (mass != 0.0f);
    if (isDynamic)
        shape->calculateLocalInertia(mass, inertia);
    
    // Determine its position
    Ogre::Vector3 pos = obj->getSceneNode()->getPosition();
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    
    // Do we already know this game object?
    if(!gameObjects.count(obj))
    {
        // Create the rigidbody
        OgreMotionState* state = new OgreMotionState(transform, obj->getSceneNode(), allowRotation);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, state, shape, inertia);
        rbInfo.m_restitution = restitution;
        btRigidBody* body = new btRigidBody(rbInfo);
        
        // Store the GameObject-RigidBody relationshup
        gameObjects[obj] = body;
        rigidBodies[body] = obj;
    }
    else
    {
        // Update the rigidbody
        gameObjects[obj]->setCenterOfMassTransform(transform);
        gameObjects[obj]->setCollisionShape(shape);
        gameObjects[obj]->setMassProps(mass, inertia);
    }
    
    // Kinematic and dynamic objects that we want to always keep track of
    if(activeKinematic)
    {
        if(!isDynamic)
            gameObjects[obj]->setCollisionFlags( gameObjects[obj]->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT); 
        gameObjects[obj]->setActivationState(DISABLE_DEACTIVATION);
    }
    
    // Add the object to the simulator
    dynamicsWorld->addRigidBody(gameObjects[obj]);
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::removeGameObject (GameObject* obj) 
{
    dynamicsWorld->removeRigidBody(gameObjects[obj]);
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::deleteGameObject (GameObject* obj) 
{
    btRigidBody* body = gameObjects[obj];
    rigidBodies.erase(body);
    delete gameObjects[obj]->getMotionState();
    delete gameObjects[obj];
    gameObjects.erase(obj);
}
//-------------------------------------------------------------------------------------
Ogre::Vector3 PhysicsSimulator::getGameObjectPosition(GameObject* obj)
{
    btVector3 btPos = gameObjects[obj]->getCenterOfMassPosition();
    return Ogre::Vector3(btPos.x(), btPos.y(), btPos.z());
}
//-------------------------------------------------------------------------------------
Ogre::Vector3 PhysicsSimulator::getGameObjectVelocity(GameObject* obj)
{
    btVector3 btVel = gameObjects[obj]->getLinearVelocity();
    return Ogre::Vector3(btVel.x(), btVel.y(), btVel.z());
}
//-------------------------------------------------------------------------------------
Ogre::Quaternion PhysicsSimulator::getGameObjectOrientation(GameObject* obj)
{
    btQuaternion btRot = gameObjects[obj]->getOrientation();
    return Ogre::Quaternion(btRot.w(), btRot.x(), btRot.y(), btRot.z());
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::setGameObjectPosition(GameObject* obj, const Ogre::Vector3& pos)
{
    btVector3 btPos = btVector3(pos.x, pos.y, pos.z);
    btTransform transform = gameObjects[obj]->getCenterOfMassTransform();
    transform.setOrigin(btPos);
    gameObjects[obj]->setCenterOfMassTransform(transform);
    
    OgreMotionState* objectMotionState = (OgreMotionState*) gameObjects[obj]->getMotionState();
    objectMotionState->setPosition(btPos);
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::setGameObjectVelocity(GameObject* obj, const Ogre::Vector3& vel)
{
    btVector3 btVel(vel.x, vel.y, vel.z);
    gameObjects[obj]->setLinearVelocity(btVel);
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::setGameObjectOrientation(GameObject* obj, const Ogre::Quaternion& rot)
{
    btQuaternion btRot(rot.x, rot.y, rot.z, rot.w);
    btTransform transform = gameObjects[obj]->getCenterOfMassTransform();
    transform.setRotation(btRot);
    gameObjects[obj]->setCenterOfMassTransform(transform);
    
    OgreMotionState* objectMotionState = (OgreMotionState*) gameObjects[obj]->getMotionState();
    objectMotionState->setOrientation(btRot);
}
//-------------------------------------------------------------------------------------
void PhysicsSimulator::stepSimulation(const float elapsedTime, int maxSubSteps, const float fixedTimestep) 
{    
    // Step simulation
    dynamicsWorld->stepSimulation(elapsedTime, maxSubSteps, fixedTimestep);
    
    // Correct the orientation of objects that we don't allow changed orientation
    for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        OgreMotionState* motionState = (OgreMotionState*) body->getMotionState();
        if (body && motionState && !motionState->allowsRotation())
        {
            Ogre::Quaternion rot = rigidBodies[body]->getSceneNode()->getOrientation();
            this->setGameObjectOrientation(rigidBodies[body], rot);
        }
    }
    
    // Inform all game objects that have collided of that fact
    int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i=0;i<numManifolds;i++)
    {
        btPersistentManifold* contactManifold =  dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
#ifdef _WIN32
		btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());
#else
		btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
#endif
        btRigidBody* bodyA = btRigidBody::upcast(obA);
        btRigidBody* bodyB = btRigidBody::upcast(obB);
        if (bodyA && bodyB)
        {
            rigidBodies[bodyA]->collidedWith(rigidBodies[bodyB]);
            rigidBodies[bodyB]->collidedWith(rigidBodies[bodyA]);
        }
    }
}