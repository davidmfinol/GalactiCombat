#ifndef __PhysicsSimulator_h_
#define __PhysicsSimulator_h_

#include <map>
#include <OgreSceneManager.h>
#include <OgreVector3.h>
#include "btBulletDynamicsCommon.h"
#include "GameObject.h"
#include "OgreMotionState.h"

class PhysicsSimulator {
    
public:
    PhysicsSimulator(int maxSphereSize);
    virtual ~PhysicsSimulator(void);
    void addGameObject (GameObject* obj, double restitution = 1.0, bool activeKinematic = false, bool allowRotation = true);
    void removeGameObject(GameObject* obj);
    void deleteGameObject(GameObject*);
    Ogre::Vector3 getGameObjectPosition(GameObject* obj);
    Ogre::Vector3 getGameObjectVelocity(GameObject* obj);
    Ogre::Quaternion getGameObjectOrientation(GameObject* obj);
    void setGameObjectPosition(GameObject* obj, const Ogre::Vector3& pos);
    void setGameObjectVelocity(GameObject* obj, const Ogre::Vector3& vel);
    void setGameObjectOrientation(GameObject* obj, const Ogre::Quaternion& rot);
    void stepSimulation(const float elapsedTime, int maxSubSteps = 1, const float fixedTimestep = 1.0f/60.0f);
    
protected:
    std::map<GameObject*, btRigidBody*> gameObjects;
    std::map<btRigidBody*, GameObject*> rigidBodies;
    std::map<std::string, btCollisionShape*> collisionShapes;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* overlappingPairCache;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    
private:
    void defineCollisionShapes(int maxSphereSize);
};

#endif // #ifndef __PhysicsSimulator_h_
