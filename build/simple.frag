#version 330 core

uniform vec3 objectColor;

out vec4 outColor;
uniform vec3 lightColor;
uniform vec3 lightColor2;
uniform float ambientStrength;
uniform vec3 viewPos;

in vec3 Normal; 
in vec3 Pos;
in vec2 Texture;
in vec3 tLightPos;
in vec3 tLightPos2;
in vec3 tViewPos;
in vec3 tPos;
uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform sampler2D ourTexture;
uniform sampler2D ourTextureMap;
uniform bool addNormalMap;

void main()
{

    vec3 color = texture(ourTexture, Texture).rgb;
    vec3 ambient = ambientStrength * color; //lightColor;
    vec3 ambient2 = ambientStrength * color; //lightColor2;
    vec3 norm ;
    vec3 lightDir;
    vec3 lightDir2 ;
    vec3 viewDir;

    //diffuse
    if (addNormalMap == false){
        norm = normalize(Normal);
        lightDir = normalize(lightPos - Pos);
        lightDir2 = normalize(lightPos2 - Pos);
        viewDir = normalize(viewPos - Pos);
    }
    else{
        // obtain normal from normal map in range [0,1]
        norm = texture(ourTextureMap, Texture).rgb;
        // transform normal vector to range [-1,1]
        norm = normalize(norm * 2.0 - 1.0);
        lightDir = normalize(tLightPos - tPos);
        lightDir2 = normalize(tLightPos2 - tPos);
        viewDir = normalize(tViewPos - tPos);
    }

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //diffuse2
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * lightColor2;

    //specular
    float specularStrength = 0.9;
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; 

    //specular2
    vec3 reflectDir2 = reflect(-lightDir2, norm);  
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32);
    vec3 specular2 = specularStrength * spec2 * lightColor2; 

    vec3 result = (ambient + diffuse + specular + ambient2 + diffuse2 + specular2) * objectColor;
    outColor = texture(ourTexture, Texture) *vec4(result,1);
    //outColor = vec4(result,1);
}
