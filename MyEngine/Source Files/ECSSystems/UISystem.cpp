//
//  UISystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 22/02/2026.
//

#include "ECSSystems/UISystem.h"
#include "ECS/Coordinator.h"
#include "GLAD/include/glad/glad.h"
#include <ft2build.h>
#include <freetype/freetype.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void UISystem::Init()
{
    glGenVertexArrays(1, &m_TextVAO);
    glGenBuffers(1, &m_TextVBO);
    glBindVertexArray(m_TextVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);

    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position (Location 0), TexCoords (Location 1) 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    
    LoadFont("EngineAssets/Fonts/Open_Sans/OpenSans_Condensed-Light.ttf", 64);
}

void UISystem::Update(float deltaTime, glm::vec2 mousePos, bool mouseClicked)
{
    for (auto const& entity : mEntities) {
        auto* transform = m_Coordinator->GetComponent<TransformComponent>(entity);
        auto* baseUi = m_Coordinator->GetComponent<UIBaseComponent>(entity);
        auto* button = m_Coordinator->GetComponent<UIButtonComponent>(entity);
        
        if(baseUi && !baseUi->isVisible) continue;
        
        glm::vec2 position = {baseUi->position.x + viewportSize.x/2, baseUi->position.y + viewportSize.y/2};
        if (button)
        {
            bool insideX = mousePos.x >= position.x && mousePos.x <= position.x + button->size.x;
            bool insideY = mousePos.y >= position.y && mousePos.y <= position.y + button->size.y;
            
            button->isHovered = insideX && insideY;
            if (button->isHovered && mouseClicked) {
                
                std::cout<<"Clickedd\n";
                button->isPressed = true;
                
                auto* script = m_Coordinator->GetComponent<ScriptComponent>(entity);
                                
                if (script && script->initialized)
                {
                    sol::protected_function func = script->env["OnButtonClicked"];
                    sol::set_environment(script->env, func);
                    
                    if (!func.valid()) continue;
                    auto result = func();
                    if (!result.valid())
                    {
                        sol::error err = result;
                        std::cerr << "LUA ERROR: " << err.what() << std::endl;
                    }
                }
            }
            else{
                button->isPressed = false;
            }
        }
    }
}

void UISystem::Render(Shader& uiShader, Shader& textShader)
{
    
    std::vector<Entity> sortedEntities = mEntities;

    std::sort(sortedEntities.begin(), sortedEntities.end(), [this](Entity a, Entity b) {
        auto* uiA = m_Coordinator->GetComponent<UIBaseComponent>(a);
        auto* uiB = m_Coordinator->GetComponent<UIBaseComponent>(b);
        
        return uiA->zOrder < uiB->zOrder;
    });
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    glm::mat4 projection = glm::ortho(0.0f, viewportSize.x, 0.0f, viewportSize.y);
    
    for (auto const& entity : sortedEntities)
    {
        auto* baseUi = m_Coordinator->GetComponent<UIBaseComponent>(entity);
        if (!baseUi || !baseUi->isVisible) continue;
        
        // 1. Render Button
        auto* button = m_Coordinator->GetComponent<UIButtonComponent>(entity);
        if (button) {
            uiShader.Use();
            uiShader.SetMatrix4(projection, "projection");
            uiShader.SetBool(false, "u_UseTexture");
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            
            glm::vec3 color;
            if(button->isPressed) color = button->clickColor;
            else color = button->isHovered ? button->hoverColor : button->normalColor;
            
            uiShader.SetVec3("u_Color", color);
            
            float centerX = viewportSize.x / 2.0f;
            float centerY = viewportSize.y / 2.0f;
            glm::vec2 finalPos = glm::vec2(centerX, centerY) + baseUi->position;
            
            DrawStaticQuad(uiShader, finalPos, button->size);
        }

        // 2. Render Text
        auto* text = m_Coordinator->GetComponent<UITextComponent>(entity);
        if (text)
        {
            textShader.Use();
            textShader.SetMatrix4(projection, "projection");
            textShader.SetMatrix4(glm::mat4(1.0f), "model");
            
            float centerX = viewportSize.x / 2.0f;
            float centerY = viewportSize.y / 2.0f;
            glm::vec2 finalPos = glm::vec2(centerX, centerY) + baseUi->position;
            
            RenderText(textShader, text->text, finalPos.x, finalPos.y, text->scale, text->color);
        }
    }
    glEnable(GL_DEPTH_TEST);
}

void UISystem::LoadFont(const std::string& path, unsigned int fontSize) {
    FT_Library ft;
    // FreeType Initialize
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // Font face loading
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font at " << path << std::endl;
        return;
    }

    // Pixel size set
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glBindTexture(GL_TEXTURE_2D, 0);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        m_Characters.insert(std::pair<char, Character>(c, character));
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    std::cout << "Font loaded and textures generated" << std::endl;
}

void UISystem::RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    shader.Use();
    shader.SetVec3("textColor", color);
    
    GLint texLoc = glGetUniformLocation(shader.shaderProgram, "textTexture");
    if (texLoc != -1) {
        glUniform1i(texLoc, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_TextVAO);

    for (auto c = text.begin(); c != text.end(); c++)
    {
        Character ch = m_Characters[*c];

        // position calculate
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advancing cursor for next character
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void UISystem::DrawStaticQuad(Shader& shader, glm::vec2 pos, glm::vec2 size) {
    glm::mat4 model = glm::mat4(1.0f);
    
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    shader.SetMatrix4(model, "model");

    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
