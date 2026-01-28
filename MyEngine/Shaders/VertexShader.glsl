//
//  VertexShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/11/2025.
//


#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosShadowSpace;
out mat3 TBN;

uniform mat4 transformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 shadowMapMatrix;

void main()
{
    vec4 worldPos = transformMatrix * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(transformMatrix))) * aNormal;
    TexCoord = aTexCoord;
    
    mat3 normalMatrix = transpose(inverse(mat3(transformMatrix)));
    
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    TBN = mat3(T, B, N);
    
    FragPosShadowSpace = shadowMapMatrix * worldPos;
    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
