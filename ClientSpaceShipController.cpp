#include "ClientSpaceShipController.h"

//-------------------------------------------------------------------------------------
ClientSpaceShipController::ClientSpaceShipController(void) : _input(0)
{
}
//-------------------------------------------------------------------------------------
ClientSpaceShipController::~ClientSpaceShipController(void)
{
}
//-------------------------------------------------------------------------------------
void ClientSpaceShipController::injectInput(char input)
{
    _input = input;
}
//-------------------------------------------------------------------------------------
bool ClientSpaceShipController::left() const
{
    return (_input & 64) >> 6;
}
//-------------------------------------------------------------------------------------
bool ClientSpaceShipController::right() const
{
    return (_input & 32) >> 5;
}
//-------------------------------------------------------------------------------------
bool ClientSpaceShipController::forward() const
{
    return (_input & 4) >> 2;
}
//-------------------------------------------------------------------------------------
bool ClientSpaceShipController::back() const
{
    return (_input & 2) >> 1;
}
//-------------------------------------------------------------------------------------
bool ClientSpaceShipController::up() const
{
    return (_input & 16) >> 4;
}
//-------------------------------------------------------------------------------------
bool ClientSpaceShipController::down() const
{
    return (_input & 8) >> 3;
}
//-------------------------------------------------------------------------------------
bool ClientSpaceShipController::shoot() const
{
    return _input & 1;
}