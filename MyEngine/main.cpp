//
//  main.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 27/10/2025.
//

#include <iostream>
#include "Header Files/Shader.h"
#include "Header Files/EngineContext.h"


int main(void)
{
    EngineContext* engineContext = new EngineContext(1000,700,"MyEngine");

    engineContext->Draw();
    engineContext->Shutdown();
    
    
    return 0;
}
