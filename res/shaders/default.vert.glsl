#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

uniform mat4 u_Transform;

void main() {
    v_FragPos = aPos;
    v_Normal = aNormal;
    v_TexCoord = aUV;
    gl_Position = u_Transform * vec4(aPos, 1.0);
}
