#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// Per-instance attributes (mat4 across locations 4-7, vec4 at location 8)
layout(location = 4) in mat4 aInstanceModel;
layout(location = 8) in vec4 aInstanceColor;

uniform mat4 u_ViewProjection;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;
out vec4 v_Color;

void main() {
    vec4 worldPos = aInstanceModel * vec4(aPos, 1.0);
    v_FragPos = worldPos.xyz;
    v_Normal = mat3(aInstanceModel) * aNormal;
    v_TexCoord = aUV;
    v_Color = aInstanceColor;
    gl_Position = u_ViewProjection * worldPos;
}
