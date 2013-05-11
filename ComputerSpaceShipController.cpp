#include "ComputerSpaceShipController.h"

//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::left() const
{
	
    return rand()%100 < 25;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::right() const
{
    return rand()%100 < 25;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::forward() const
{
    return rand()%100 < 25;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::back() const
{
    return rand()%100 < 25;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::up() const
{
    return rand()%100 < 25;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::down() const
{
    return rand()%100 < 25;
}
//-------------------------------------------------------------------------------------
bool ComputerSpaceShipController::shoot() const
{
    return rand()%100 < 25;
}
