#version 330 core
precision highp float;

in vec2 v_TexCoord;

out vec4 fragColor;

uniform float u_AspectRatio;
uniform float u_Time;
uniform vec3 u_CameraPos;
uniform int u_EnableShadows;
uniform int u_EnableReflections;
uniform float u_ReflectionStrength;

const int MAX_SPHERES = 128;
uniform int u_SphereCount;
struct Sphere {
    vec4 posRad;    // xyz = center, w = radius
    vec4 colShiny;  // xyz = color, w = shininess (0.0 to 1.0)
};
layout(std140) uniform SphereBlock {
    Sphere u_Spheres[MAX_SPHERES];
};

float hitSphere(vec3 center, float radius, vec3 ro, vec3 rd) {
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - c;
    if (discriminant < 0.0) {
        return -1.0;
    }
    float sqrtD = sqrt(discriminant);
    float t1 = -b - sqrtD;
    if (t1 > 0.001) return t1;
    float t2 = -b + sqrtD;
    if (t2 > 0.001) return t2;
    return -1.0;
}

bool hitPlane(vec3 ro, vec3 rd, out float t, out vec3 normal) {
    float denom = dot(rd, vec3(0.0, 1.0, 0.0));
    if (abs(denom) > 0.0001) {
        t = dot(vec3(0.0, -1.0, 0.0) - ro, vec3(0.0, 1.0, 0.0)) / denom;
        normal = denom < 0.0 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, -1.0, 0.0);
        return t >= 0.001 && t < 200.0;
    }
    return false;
}

float checkerboard(vec3 p, float scale) {
    vec2 q = fract(p.xz * 0.5 * scale) - 0.5;
    return (q.x * q.y > 0.0) ? 1.0 : 0.0;
}

vec3 trace(vec3 ro, vec3 rd) {
    vec3 color = vec3(0.0);
    float currentReflectance = 1.0;
    
    vec3 currentRo = ro;
    vec3 currentRd = rd;

    for (int depth = 0; depth < 4; depth++) {
        float minT = 1e20;
        int hitType = 0; // 0 = sky, 1 = plane, 2 = sphere
        vec3 hitNormal = vec3(0.0);
        vec3 hitColor = vec3(0.0);
        float hitShininess = 0.0;
        
        // Test ground plane
        float tPlane;
        vec3 planeNorm;
        if (hitPlane(currentRo, currentRd, tPlane, planeNorm)) {
            if (tPlane < minT) {
                minT = tPlane;
                hitType = 1;
                hitNormal = planeNorm;
                vec3 hitPos = currentRo + currentRd * tPlane;
                float c = checkerboard(hitPos, 0.75);
                hitColor = c > 0.5 ? vec3(0.55) : vec3(0.22);
                hitShininess = 0.15; // subtle reflective sheen on floor
            }
        }
        
        // Test all dynamic sphere instances
        for (int i = 0; i < MAX_SPHERES; i++) {
            if (i >= u_SphereCount) break;
            vec4 posRad = u_Spheres[i].posRad;
            float t = hitSphere(posRad.xyz, posRad.w, currentRo, currentRd);
            if (t > 0.0 && t < minT) {
                minT = t;
                hitType = 2;
                hitNormal = normalize((currentRo + currentRd * t) - posRad.xyz);
                hitColor = u_Spheres[i].colShiny.xyz;
                hitShininess = u_Spheres[i].colShiny.w;
            }
        }
        
        // Sky background
        if (hitType == 0) {
            float t = 0.5 * (currentRd.y + 1.0);
            vec3 skyColor = (1.0 - t) * vec3(1.0) + t * vec3(0.45, 0.68, 0.98);
            vec3 sunDir = normalize(vec3(2.0, 6.0, 1.0));
            float sunSpec = pow(max(dot(currentRd, sunDir), 0.0), 48.0);
            skyColor += vec3(1.0, 0.95, 0.8) * sunSpec * 1.2;
            color += currentReflectance * skyColor;
            break;
        }
        
        vec3 hitPoint = currentRo + currentRd * minT;
        
        // Shadow occlusion rays
        float shadowMult = 1.0;
        if (u_EnableShadows != 0) {
            vec3 lightDir = normalize(vec3(2.0, 6.0, 1.0));
            vec3 shadowRo = hitPoint + hitNormal * 0.002;
            for (int i = 0; i < MAX_SPHERES; i++) {
                if (i >= u_SphereCount) break;
                vec4 posRad = u_Spheres[i].posRad;
                if (hitSphere(posRad.xyz, posRad.w, shadowRo, lightDir) > 0.0) {
                    shadowMult = 0.25;
                    break;
                }
            }
        }
        
        // Direct diffuse + Blinn-Phong specular highlight for shiny balls
        vec3 lightDir = normalize(vec3(2.0, 6.0, 1.0));
        vec3 viewDir = -currentRd;
        vec3 halfDir = normalize(lightDir + viewDir);
        float diff = max(dot(hitNormal, lightDir), 0.0);
        float spec = pow(max(dot(hitNormal, halfDir), 0.0), 32.0) * hitShininess;
        vec3 localColor = hitColor * (diff * shadowMult * 0.8 + 0.15) + vec3(spec * shadowMult * 0.85);
        
        // Reflection ray bounce
        float effectiveReflectance = hitShininess * u_ReflectionStrength;
        if (u_EnableReflections != 0 && effectiveReflectance > 0.01) {
            color += currentReflectance * localColor * (1.0 - effectiveReflectance);
            currentReflectance *= effectiveReflectance;
            currentRo = hitPoint + hitNormal * 0.002;
            currentRd = reflect(currentRd, hitNormal);
        } else {
            color += currentReflectance * localColor;
            break;
        }
    }
    
    return color;
}

void main() {
    vec2 ndc = v_TexCoord * 2.0 - 1.0;
    vec3 ro = u_CameraPos;
    vec3 rd = normalize(vec3(ndc.x * u_AspectRatio, ndc.y, -1.0));

    vec3 col = trace(ro, rd);
    col = pow(col, vec3(1.0 / 2.2)); // Gamma correction

    fragColor = vec4(col, 1.0);
}
