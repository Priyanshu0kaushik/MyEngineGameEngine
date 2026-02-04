//
//  FragmentShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/11/2025.
//

#version 330 core
#define MAX_LIGHTS 10

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosShadowSpace;
in mat3 TBN;

uniform sampler2D mainTexture;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D shadowMap;

uniform int u_LightCount = 0;
uniform vec4 u_Light_ambient = vec4(1,1,1,1);
uniform vec4 u_Light_diffuse[MAX_LIGHTS];
uniform vec4 u_Light_specular[MAX_LIGHTS];
uniform vec4 u_Light_position[MAX_LIGHTS];
uniform vec3 u_Light_attenuation[MAX_LIGHTS];

uniform Material u_Material;
uniform bool u_HasTexture;
uniform bool u_HasSpecularMap;
uniform bool u_HasNormalMap;
uniform vec3 eyePosition;


float CalcShadowFactor(vec4 shadowSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = shadowSpace.xyz / shadowSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) return 0.0;
    
    float currentDepth = projCoords.z;

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    
    shadow /= 25.0;
    return shadow;
}

void main()
{
    vec3 norm;
    if(u_HasNormalMap) {
        norm = texture(normalMap, TexCoord).rgb;
        norm = normalize(norm * 2.0 - 1.0);
        norm = normalize(TBN * norm);
    } else {
        norm = normalize(Normal);
    }
    vec3 viewDir = normalize(eyePosition - FragPos);
    vec4 texColor = u_HasTexture ? texture(mainTexture, TexCoord) : vec4(1.0);
    vec4 specularTexel = u_HasSpecularMap ? texture(specularMap, TexCoord) : vec4(1.0);

    vec3 ambient = u_Material.ambient * u_Light_ambient.rgb * texColor.rgb;
    vec3 totalDiffuseSpecular = vec3(0.0);


    for(int i = 0; i < u_LightCount; i++)
    {
        vec3 lightDir;
        float attenuation = 1.0;
        float currentShadow = 0.0;

        if(u_Light_position[i].w == 0.0)
        {
            lightDir = normalize(-u_Light_position[i].xyz);
            currentShadow = CalcShadowFactor(FragPosShadowSpace, norm, lightDir);;
        }
        else
        {
            lightDir = normalize(u_Light_position[i].xyz - FragPos);
            float distance = length(u_Light_position[i].xyz - FragPos);
            attenuation = 1.0 / (u_Light_attenuation[i].x +
                                 u_Light_attenuation[i].y * distance +
                                 u_Light_attenuation[i].z * (distance * distance));
        }

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * u_Material.diffuse * u_Light_diffuse[i].rgb * texColor.rgb;

        vec3 specular = vec3(0.0);
        if (diff > 0.0)
        {
            vec3 halfVector = normalize(lightDir + viewDir);
            float specAngle = max(dot(norm, halfVector), 0.0);
            float spec = pow(specAngle, u_Material.shininess);
            specular = spec * u_Material.specular * u_Light_specular[i].rgb * specularTexel.rgb;
        }

        totalDiffuseSpecular += (diffuse + specular) * attenuation * (1.0 - currentShadow);
    }

    vec3 result = ambient + totalDiffuseSpecular;
    FragColor = vec4(result, 1.0);
    
}
