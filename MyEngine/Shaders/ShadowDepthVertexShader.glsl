//
//  ShadowDepthVertexShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 24/01/2026.
//


#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 transformMatrix;
uniform mat4 shadowMapMatrix;

void main() {
    gl_Position = shadowMapMatrix * transformMatrix * vec4(aPos, 1.0);
}
