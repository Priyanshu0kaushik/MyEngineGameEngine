cmake_minimum_required(VERSION 3.15)
project(MyEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # Version 1.0.1
)

FetchContent_MakeAvailable(glm)

file(GLOB_RECURSE MAIN_SRC_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Source Files/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Source Files/*.h"
)

file(GLOB_RECURSE MAIN_HEADER_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Header Files/*.h"
)

set(GLAD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/GLAD/glad.c")

file(GLOB IMGUI_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/ImGui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/ImGui/*.h"
)
file(GLOB_RECURSE SHADER_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Shaders/*.glsl"
)

set(ENTRY_POINT "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/main.cpp")

add_executable(${PROJECT_NAME} 
    ${ENTRY_POINT}
    ${MAIN_SRC_FILES} 
    ${MAIN_HEADER_FILES} 
    ${GLAD_SRC} 
    ${IMGUI_SOURCES}
    ${SHADER_FILES}
)


target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Source Files"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Header Files"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/GLAD/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/GLFW/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/ImGui"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External"
    "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/GLFW/include/GLFW"
)


target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

if(APPLE)
    set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/GLFW/lib-mac/libglfw3.a")
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${GLFW_LIB}
        "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo"
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
elseif(WIN32)
    set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/External/GLFW/lib-win/glfw3.lib")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _USE_MATH_DEFINES)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIB} opengl32.lib user32.lib gdi32.lib shell32.lib)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/EngineAssets")

if(EXISTS "${ASSETS_DIR}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/EngineAssets"
        COMMENT "Copying assets to build directory..."
    )
else()
    message(WARNING "Assets directory not found at: ${ASSETS_DIR}")
endif()

set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Shaders")

if(EXISTS "${SHADERS_DIR}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADERS_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders"
        COMMENT "Copying shaders to build directory..."
    )
else()
    message(WARNING "Shaders directory not found at: ${SHADERS_DIR}")
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Source Files" PREFIX "Source" FILES ${MAIN_SRC_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Header Files" PREFIX "Headers" FILES ${MAIN_HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/Shaders" PREFIX "Shaders" FILES ${SHADER_FILES})
source_group("External\\GLAD" FILES ${GLAD_SRC})
source_group("External\\ImGui" FILES ${IMGUI_SOURCES})

