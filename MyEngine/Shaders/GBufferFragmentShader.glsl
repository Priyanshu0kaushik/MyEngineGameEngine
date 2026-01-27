//
//  GBufferFragmentShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 27/01/2026.
//

#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform bool u_HasTexture;
uniform bool u_HasSpecularMap;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedoSpec.rgb = u_HasTexture ? texture(texture_diffuse, TexCoords).rgb : vec3(1.0);
    gAlbedoSpec.a = u_HasSpecularMap? texture(texture_specular, TexCoords).r : 1.0;
}
