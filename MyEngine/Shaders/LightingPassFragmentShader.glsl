#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D shadowMap;

#define MAX_LIGHTS 100
uniform int u_LightCount;
uniform vec4 u_Light_ambient = vec4(0.1, 0.1, 0.1, 1.0);
uniform vec4 u_Light_diffuse[MAX_LIGHTS];
uniform vec4 u_Light_specular[MAX_LIGHTS];
uniform vec4 u_Light_position[MAX_LIGHTS];   // .w == 0 directional, .w == 1 point
uniform vec3 u_Light_attenuation[MAX_LIGHTS];

uniform vec3 eyePosition;
uniform mat4 shadowMapMatrix;


float CalcShadowFactor(vec3 worldPos) {
    
    vec4 shadowSpace = shadowMapMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = shadowSpace.xyz / shadowSpace.w;

    if(projCoords.z > 1.0) return 0.0;

    float shadowSample = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.005;
    return (currentDepth - bias > shadowSample) ? 1.0 : 0.0;
}

void main() {
    
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normal  = normalize(texture(gNormal, TexCoords).rgb);
    vec4 albedoSample = texture(gAlbedoSpec, TexCoords);
    vec3 albedo = albedoSample.rgb;
    float specMask = albedoSample.a;

    vec3 viewDir = normalize(eyePosition - fragPos);
    vec3 ambient = u_Light_ambient.rgb * albedo;
    vec3 totalDiffuseSpecular = vec3(0.0);

    float shadowFactor = CalcShadowFactor(fragPos);


    for(int i = 0; i < u_LightCount; i++) {
        vec3 lightDir;
        float attenuation = 1.0;
        float currentShadow = 0.0;

        if(u_Light_position[i].w == 0.0) {
            lightDir = normalize(-u_Light_position[i].xyz);
            currentShadow = shadowFactor;
        }
        else {
            lightDir = normalize(u_Light_position[i].xyz - fragPos);
            float distance = length(u_Light_position[i].xyz - fragPos);
            attenuation = 1.0 / (u_Light_attenuation[i].x +
                                 u_Light_attenuation[i].y * distance +
                                 u_Light_attenuation[i].z * (distance * distance));
        }

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * albedo * u_Light_diffuse[i].rgb;

        vec3 specular = vec3(0.0);
        if (diff > 0.0) {
            vec3 halfVector = normalize(lightDir + viewDir);
            float specAngle = max(dot(normal, halfVector), 0.0);
            float spec = pow(specAngle, 32.0);
            specular = spec * u_Light_specular[i].rgb * specMask;
        }

        totalDiffuseSpecular += (diffuse + specular) * attenuation * (1.0 - currentShadow);
    }

    vec3 result = ambient + totalDiffuseSpecular;
    FragColor = vec4(result, 1.0);

}
