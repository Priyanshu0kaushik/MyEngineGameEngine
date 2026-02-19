//
//  ECSSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/02/2026.
//

#include "ECS/ECSSystem.h"
#include "ECS/Coordinator.h"

void ECSSystem::SetCoordinator(Coordinator* aCoordinator)
{
    m_Coordinator = aCoordinator;
}
