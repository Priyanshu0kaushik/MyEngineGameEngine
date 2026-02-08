//
//  Project.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 04/01/2026.
//

#include "Project.h"
#include <iostream>
#include <fstream>
#include <sstream>


namespace fs = std::filesystem;

std::shared_ptr<Project> Project::New(const std::filesystem::path& baseDir, const std::string& projectName)
{
    fs::path projectRoot = baseDir / projectName;
    if (fs::exists(projectRoot))
    {
        std::cerr << "Project already exists at this path!" << std::endl;
        return nullptr;
    }
    fs::create_directories(projectRoot);
    fs::create_directories(projectRoot / "Assets");
    fs::create_directories(projectRoot / "Assets"/ "Scenes");
    

    s_ActiveProject = std::make_shared<Project>();
    s_ActiveProject->m_Config.Name = projectName;
    s_ActiveProject->m_Config.AssetDirectory = "Assets";
    s_ActiveProject->m_ProjectDirectory = projectRoot;
    
    
    fs::path projectFile = projectRoot / (projectName + ".meproject");
    std::ofstream out(projectFile);
    if (out.is_open()) {
        out << "ProjectName: " << projectName << "\n";
        out << "AssetDir: Assets" << "\n";
        out << "StartScene: " << s_ActiveProject->m_Config.StartScene << "\n";
        out.close();
    }
    std::cout << "New Project Created: " << projectRoot << std::endl;
    return s_ActiveProject;
}

std::shared_ptr<Project> Project::Load(const std::filesystem::path& path)
{
    std::shared_ptr<Project> project = std::make_shared<Project>();
    project->m_ProjectDirectory = path.parent_path();
    
    std::ifstream in(path);
    if (!in.is_open())
    {
        std::cerr << "Failed to open project file: " << path << std::endl;
        return nullptr;
    }

    std::string line;
    while (std::getline(in, line))
    {
        std::stringstream ss(line);
        std::string key;
        
        if(std::getline(ss, key, ':'))
        {
            std::string value;
            if(std::getline(ss, value))
            {
                size_t firstChar = value.find_first_not_of(' ');
                if (firstChar != std::string::npos) value = value.substr(firstChar);

                if (key == "ProjectName") project->m_Config.Name = value;
                else if (key == "AssetDir") project->m_Config.AssetDirectory = value;
                else if (key == "StartScene") project->m_Config.StartScene = value;
            }
        }
    }
    
    in.close();

    s_ActiveProject = project;
    project->m_ActiveScenePath = project->m_Config.StartScene;
    
    std::cout << "Project Loaded: " << project->m_Config.Name << std::endl;
    return project;
}

bool Project::SaveActive(const std::filesystem::path& path)
{
    if (!s_ActiveProject) {
        std::cerr << "No active project to save!" << std::endl;
        return false;
    }

    std::ofstream out(path);
    if (!out.is_open())
    {
        std::cerr << "Failed to open project file for writing: " << path << std::endl;
        return false;
    }

    const ProjectConfig& config = s_ActiveProject->m_Config;

    out << "ProjectName: " << config.Name << "\n";
    out << "AssetDir: " << config.AssetDirectory << "\n";
    out << "StartScene: " << config.StartScene << "\n";

    out.close();
    std::cout << "Project Saved Successfully!" << std::endl;
    return true;
}
