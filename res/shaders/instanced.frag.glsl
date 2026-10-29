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

#define MAX_LIGHTS 8

uniform int u_LightCount;
uniform vec4 u_LightPosType[MAX_LIGHTS];
uniform vec4 u_LightColorRange[MAX_LIGHTS];
uniform vec3 u_CameraPos;
uniform float u_Specular;
uniform float u_Shininess;

vec3 computeLighting(vec3 base, vec3 N, vec3 fragPos) {
    vec3 V = normalize(u_CameraPos - fragPos);
    vec3 result = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (i >= u_LightCount) {
            break;
        }

        vec4 posType = u_LightPosType[i];
        vec3 radiance = u_LightColorRange[i].rgb;
        int type = int(posType.w + 0.5);

        if (type == 0) {
            result += radiance * base;
            continue;
        }

        vec3 L;
        float attenuation = 1.0;
        if (type == 1) {
            L = -posType.xyz;
        } else {
            vec3 toLight = posType.xyz - fragPos;
            float dist = length(toLight);
            L = toLight / max(dist, 1e-4);

            float x = dist / max(u_LightColorRange[i].w, 1e-4);
            float window = clamp(1.0 - (x * x * x * x), 0.0, 1.0);
            attenuation = (window * window) / ((dist * dist) + 1.0);
        }

        float NdotL = max(dot(N, L), 0.0);
        vec3 H = normalize(L + V);
        float spec = (NdotL > 0.0) ? pow(max(dot(N, H), 0.0), u_Shininess) * u_Specular : 0.0;

        result += radiance * attenuation * ((NdotL * base) + spec);
    }

    return result;
}

void main() {
    vec4 texColor = (u_HasTexture != 0) ? texture(u_Texture, v_TexCoord) : vec4(1.0);
    vec4 baseColor = texColor * v_Color;

    fragColor = vec4(computeLighting(baseColor.rgb, normalize(v_Normal), v_FragPos), baseColor.a);
}
