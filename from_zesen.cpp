1: The source below is from my friend. His team is working on a dodge ball game, and the function below is called whenever the player releases the left button of his mouse (the ball will then be thrown to the direction the camera is facing). I am not quite sure if this will help our shooting feature. (mPower is a variable that stores the period of time when the button was held down, which is irrelavant.)


void Player::endThrow(){
    mHasBall = false;
    Ogre::Vector3 muldir=Ogre::Vector3(camPlayer->getDerivedDirection().x,0,camPlayer->getDerivedDirection().z);
    float mult = 20.0f/muldir.length();
    ball->setPosition(nodePlayer->getPosition().x+(camPlayer->getDerivedDirection().x*mult), nodePlayer->getPosition().y + 75, nodePlayer->getPosition().z+(camPlayer->getDerivedDirection().z*mult));
    btVector3 dir=btVector3(camPlayer->getDerivedDirection().x,camPlayer->getDerivedDirection().y,camPlayer->getDerivedDirection().z);
    ball->addToBullet(dir, mPower); 
    ball = NULL;
    mPower = 1;
    mThrowing = false;

    GUIManager::GUIControl.destroyPowerBar();
    GUIManager::GUIControl.threwBall();
}
