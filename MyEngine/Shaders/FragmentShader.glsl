//
//  FragmentShader.glsl
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/11/2025.
//

#version 330 core
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

uniform sampler2D mainTexture;
uniform sampler2D specularMap;

uniform vec3 light_position = vec3(1, 10, 12);
uniform vec4 light_ambient = vec4(0.2, 0.2, 0.2, 1);
uniform vec4 light_diffuse = vec4(1, 0.9, 0.8, 1);
uniform vec4 light_specular = vec4(0.9, 0.8, 0.7, 1);
uniform vec3 light_attenuation = vec3(1, 0.1, 0.001);


uniform Material u_Material;
uniform bool u_HasTexture;
uniform bool u_HasSpecularMap;

uniform vec3 eyePosition;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light_position - FragPos);
    vec3 viewDir = normalize(eyePosition - FragPos);
    
    vec4 texColor = u_HasTexture ? texture(mainTexture, TexCoord) : vec4(1.0);
    
    vec3 ambient = u_Material.ambient.rgb * light_ambient.rgb * texColor.rgb;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_Material.diffuse.rgb * light_diffuse.rgb * texColor.rgb;

    vec4 specularTexel = u_HasSpecularMap ? texture(specularMap, TexCoord) : vec4(1.0);
    vec3 specular = vec3(0.0);
    if (diff > 0.0)
    {
        
        vec3 halfVector = normalize(lightDir + viewDir);
        float specAngle = max(dot(norm, halfVector), 0.0);
        float spec = pow(specAngle, u_Material.shininess);
        
        specular = spec * u_Material.specular.rgb * light_specular.rgb * specularTexel.rgb;
    }

    
    float distance = length(light_position - FragPos);
    float attenuation = 1.0 / (light_attenuation.x + light_attenuation.y * distance + light_attenuation.z * (distance * distance));

    vec3 result = ambient + (diffuse + specular) * attenuation;
    
    FragColor = vec4(result, 1.0);
}
