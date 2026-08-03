#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform bool u_HasTexture;
uniform vec4 u_Color;

void main()
{
    // Simple lighting calculation using the normal
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    
    vec4 texColor = u_HasTexture ? texture(u_Texture, TexCoord) : vec4(1.0);
    vec4 finalColor = texColor * u_Color;
    
    vec3 diffuse = diff * finalColor.rgb;
    vec3 ambient = 0.3 * finalColor.rgb;

    FragColor = vec4(ambient + diffuse, finalColor.a);
}
