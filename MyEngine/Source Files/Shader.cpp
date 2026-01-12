//
//  Shader.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 02/11/2025.
//

#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "glfw3.h"
#include <glm/gtc/type_ptr.hpp>

std::string Shader::LoadShader(const char *aPath){
    std::string shaderCode;
    std::ifstream shaderFile;
    
    shaderFile.open(aPath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open file: " << aPath << std::endl;
        return "";
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    
    shaderFile.close();
    shaderCode = shaderStream.str();
    
    return shaderCode;
}

unsigned int Shader::LoadVertexShader(){
    int result;
    char Log[512];
    
    std::string shaderCodeString = LoadShader(m_VertexPath.c_str());
    const char* shaderCode = shaderCodeString.c_str();
    
    unsigned int shaderObject;

    shaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderObject, 1, &shaderCode , NULL);
    glCompileShader(shaderObject);
    
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);
    
    if(!result){
        glGetShaderInfoLog(shaderObject, 512, NULL, Log);
        std::cout<<"Failed to compile vertex shader: "<< Log << std::endl;
    }
    return shaderObject;
}

unsigned int Shader::LoadFragmentShader(){
    int result;
    char Log[512];
    
    std::string shaderCodeString = LoadShader(m_FragmentPath.c_str());
    const char* shaderCode = shaderCodeString.c_str();
    
    unsigned int shaderObject;

    shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderObject, 1, &shaderCode , NULL);
    glCompileShader(shaderObject);
    
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);
    
    if(!result){
        glGetShaderInfoLog(shaderObject, 512, NULL, Log);
        std::cout<<"Failed to compile vertex shader: "<< Log << std::endl;
    }
    return shaderObject;
}


Shader::Shader(const char* aVertexPath, const char* aFragmentPath)
{
    m_VertexPath = aVertexPath;
    m_FragmentPath = aFragmentPath;
    
    Reload();
}

void Shader::Reload()
{
    
    int result;
    char Log[512];
    
    unsigned int VertexShader = LoadVertexShader();
    unsigned int FragmentShader = LoadFragmentShader();
    
    unsigned int newProgram = glCreateProgram();
    glAttachShader(newProgram, VertexShader);
    glAttachShader(newProgram, FragmentShader);
    glLinkProgram(newProgram);
    
    glGetProgramiv(newProgram, GL_LINK_STATUS, &result);
    
    if(!result){
        glGetProgramInfoLog(newProgram, 512, NULL, Log);
        std::cout<<"Failed to compile vertex shader: "<< Log << std::endl;
        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);
        glDeleteProgram(newProgram);
        return;
    }
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
    }
    shaderProgram = newProgram;
    std::cout << "Shader Hot Reloaded Successfully!" << std::endl;
    
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

}

void Shader::Use(){
    glUseProgram(shaderProgram);
}

void Shader::SetMatrix4(glm::mat4 aMatrix, const std::string& aName) const{
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, aName.c_str()), 1, GL_FALSE, glm::value_ptr(aMatrix));
}

void Shader::SetBool(bool booleanToSet, const std::string &aName) const{
    glUniform1i(glGetUniformLocation(shaderProgram, aName.c_str()), (int)booleanToSet);
}

void Shader::SetVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &value[0]);
}

void Shader::SetFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}
