//
//  UIFragmentShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 22/02/2026.
//

#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 u_Color = vec3(1,0,0);
uniform sampler2D u_ButtonTexture;
uniform bool u_UseTexture;

void main() {
    if (u_UseTexture) {
        FragColor = texture(u_ButtonTexture, TexCoords) * vec4(u_Color, 1.0);
    } else {
        FragColor = vec4(u_Color, 1.0);
    }
}
