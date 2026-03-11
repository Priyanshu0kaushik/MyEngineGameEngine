//
//  UISystem.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 22/02/2026.
//

#pragma once
#include "ECS/ECSSystem.h"
#include "Components.h"
#include "Shader.h"

struct Character {
    unsigned int TextureID;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    unsigned int Advance;
};

class UISystem : public ECSSystem{
    
public:
    void Init() override;
    void Update(float deltaTime, glm::vec2 mousePos, bool mouseClicked);
    void Render(Shader& uiShader, Shader& textShader);
    void SetViewportSize(glm::vec2 aViewportSize){viewportSize = aViewportSize;}
    
private:
    bool IsMouseOver(const glm::vec2& mousePos, const glm::vec2& uiPos, const glm::vec2& uiSize);
    
    void DrawStaticQuad(Shader& shader, glm::vec2 pos, glm::vec2 size);
    void LoadFont(const std::string& path, unsigned int fontSize);
    void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);
    
    glm::vec2 viewportSize;
    std::map<char, Character> m_Characters;
    unsigned int m_QuadVAO, m_QuadVBO;
    unsigned int m_TextVAO, m_TextVBO;
};
