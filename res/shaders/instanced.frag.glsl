#version 330 core
precision highp float;
precision highp int;

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;
in vec4 v_Color;

out vec4 fragColor;

uniform int u_HasTexture;
uniform sampler2D u_Texture;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(v_Normal), lightDir), 0.0);

    vec4 texColor = (u_HasTexture != 0) ? texture(u_Texture, v_TexCoord) : vec4(1.0);
    vec4 baseColor = texColor * v_Color;

    vec3 diffuse = diff * baseColor.rgb;
    vec3 ambient = 0.3 * baseColor.rgb;

    fragColor = vec4(ambient + diffuse, baseColor.a);
}
