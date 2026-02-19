//
//  InputManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 19/02/2026.
//

#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>

class InputManager {
public:
    static InputManager& Get() {
        if(!m_Instance) m_Instance = new InputManager();
        return *m_Instance;
    }

    void UpdateKeyState(int key, int action){
        if (action == GLFW_PRESS) {
            m_Keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            m_Keys[key] = false;
        }
    }

    bool IsKeyPressed(int key){
        return m_Keys[key];
    }

private:
    static InputManager* m_Instance;
    InputManager() = default;
    std::unordered_map<int, bool> m_Keys;
};
