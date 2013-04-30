#include "ComputerSpaceShipController.h"

//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::left() const
{
    return false;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::right() const
{
    return true;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::forward() const
{
    return true;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::back() const
{
    return false;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::up() const
{
    return true;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::down() const
{
    return false;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::shoot() const
{
    return false;
}