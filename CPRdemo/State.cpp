
#include "Arduino.h";
#include "State.h";


State::State(int id)
{
	_id = id;
}

void State::Init()
{
	
}

void State::DeInit()
{

}

int State::GetId()
{
	return _id;
}