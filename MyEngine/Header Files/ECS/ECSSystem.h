//
//  System.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 16/11/2025.
//


#pragma once
#include <vector>
#include "ECS.h"


class Coordinator;

class ECSSystem{
public:
    virtual void Init(){};
    std::vector<Entity> mEntities;
    void SetCoordinator(Coordinator* aCoordinator);

protected:
    Coordinator* m_Coordinator;

};
