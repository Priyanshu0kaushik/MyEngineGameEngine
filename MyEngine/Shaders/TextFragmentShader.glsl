//
//  TextFragmentShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 22/02/2026.
//

#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D textTexture;
uniform vec3 textColor = vec3(1,0,0);

void main() {
    float alpha = texture(textTexture, TexCoords).r;
    FragColor = vec4(textColor, alpha);
}
