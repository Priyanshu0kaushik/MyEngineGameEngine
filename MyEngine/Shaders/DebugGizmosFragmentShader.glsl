//
//  DebugGizmosFragmentShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 11/02/2026.
//

#version 410 core
in vec3 vertColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertColor, 1.0);
}
