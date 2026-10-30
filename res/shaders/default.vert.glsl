#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

#define MAX_LIGHTS 8

layout(std140) uniform FrameData {
    mat4 u_ViewProjection;
    vec4 u_CameraPos; // xyz
    vec4 u_LightPosType[MAX_LIGHTS];
    vec4 u_LightColorRange[MAX_LIGHTS];
    int u_LightCount;
};

layout(std140) uniform ObjectData {
    mat4 u_Model;
    mat4 u_NormalMatrix;
    vec4 u_Color;
    float u_Specular;
    float u_Shininess;
    int u_HasTexture;
};

void main() {
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    v_FragPos = worldPos.xyz;
    v_Normal = mat3(u_NormalMatrix) * aNormal;
    v_TexCoord = aUV;
    gl_Position = u_ViewProjection * worldPos;
}
