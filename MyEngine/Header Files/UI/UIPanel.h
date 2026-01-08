//
//  UIPanel.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/12/2025.
//

#pragma once

#include "EngineContext.h"
#include "ECS/Coordinator.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


struct EditorDrawContext {
    Coordinator* coordinator;
    EngineContext* engine;
    Entity* selectedEntity;
};

class UIPanel{
public:
    virtual ~UIPanel() = default;
    virtual void Init(EditorDrawContext& context);
    virtual void Draw(EditorDrawContext& context) = 0;
};
