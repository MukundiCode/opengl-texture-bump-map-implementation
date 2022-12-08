#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 mvp;
uniform mat4 trans;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightPos2;

out vec3 Normal;
out vec3 Pos;
out vec2 Texture;

out vec3 tLightPos;
out vec3 tLightPos2;
out vec3 tViewPos;
out vec3 tPos;

void main()
{

    Normal = mat3(trans) * normal ;
    vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(Normal,    0.0)));
    mat3 TBN = transpose(mat3(T, B, N));

    tLightPos = TBN * lightPos;
    tLightPos2 = TBN * lightPos2;
    tViewPos  = TBN * viewPos;
    tPos  = TBN * vec3(model * vec4(position, 1.0));

    Pos = vec3(model *vec4(position, 1.0));
    Normal = mat3(trans) * normal ;
    Texture = texture;
    //TextureMap = textureMap;
    gl_Position = mvp * vec4(Pos,1.0f);

}
