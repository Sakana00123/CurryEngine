#include "pch.h"
#include "BaseInputModule.h"
#include "EventSystem.h"

BaseInputModule::BaseInputModule() : eventSystem(EventSystem::GetCurrent())
{
}