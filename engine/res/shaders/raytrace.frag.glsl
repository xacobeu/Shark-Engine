#version 330 core
precision highp float;

in vec2 v_TexCoord;

out vec4 fragColor;

layout(std140) uniform RaytraceParams {
    float u_AspectRatio;
    float u_Time;
    vec3 u_CameraPos;
    vec3 u_SpherePos;
    float u_SphereRadius;
    int u_EnableShadows;
    int u_EnableReflections;
    float u_ReflectionStrength;
};

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    int isMirror;
};

float hitSphere(vec3 center, float radius, vec3 ro, vec3 rd) {
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    if (c < 0.0) return -1.0;
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
    Sphere s1 = Sphere(u_SpherePos, u_SphereRadius, vec3(0.9, 0.1, 0.15), u_EnableReflections);
    Sphere s2 = Sphere(vec3(1.5, 0.5, -6.0), 0.7, vec3(0.1, 0.8, 0.2), 0);
    Sphere s3 = Sphere(vec3(-1.8, -0.2, -4.0), 0.5, vec3(0.1, 0.2, 0.8), 0);
    
    vec3 color = vec3(0.0);
    float currentReflectance = 1.0;
    
    vec3 currentRo = ro;
    vec3 currentRd = rd;

    for (int depth = 0; depth < 3; depth++) {
        float minT = 1e20;
        int hitType = 0; // 0 = sky, 1 = plane, 2 = sphere1, 3 = sphere2, 4 = sphere3
        vec3 hitNormal;
        vec3 hitColor = vec3(0.0);
        int hitMirror = 0;
        
        // Test plane
        float tPlane;
        vec3 planeNorm;
        if (hitPlane(currentRo, currentRd, tPlane, planeNorm)) {
            if (tPlane < minT) {
                minT = tPlane;
                hitType = 1;
                hitNormal = planeNorm;
                vec3 hitPos = currentRo + currentRd * tPlane;
                float c = checkerboard(hitPos, 1.0);
                hitColor = c > 0.5 ? vec3(0.55) : vec3(0.3);
            }
        }
        
        // Test sphere 1
        float tS1 = hitSphere(s1.center, s1.radius, currentRo, currentRd);
        if (tS1 > 0.0 && tS1 < minT) {
            minT = tS1;
            hitType = 2;
            hitNormal = normalize((currentRo + currentRd * tS1) - s1.center);
            hitColor = s1.color;
            hitMirror = s1.isMirror;
        }
        
        // Test sphere 2
        float tS2 = hitSphere(s2.center, s2.radius, currentRo, currentRd);
        if (tS2 > 0.0 && tS2 < minT) {
            minT = tS2;
            hitType = 3;
            hitNormal = normalize((currentRo + currentRd * tS2) - s2.center);
            hitColor = s2.color;
            hitMirror = s2.isMirror;
        }
        
        // Test sphere 3
        float tS3 = hitSphere(s3.center, s3.radius, currentRo, currentRd);
        if (tS3 > 0.0 && tS3 < minT) {
            minT = tS3;
            hitType = 4;
            hitNormal = normalize((currentRo + currentRd * tS3) - s3.center);
            hitColor = s3.color;
            hitMirror = s3.isMirror;
        }
        
        if (hitType == 0) {
            float t = 0.5 * (currentRd.y + 1.0);
            vec3 skyColor = (1.0 - t) * vec3(1.0) + t * vec3(0.5, 0.7, 1.0);
            color += currentReflectance * skyColor;
            break;
        }
        
        vec3 hitPoint = currentRo + currentRd * minT;
        
        float shadowMult = 1.0;
        if (u_EnableShadows != 0) {
            vec3 lightDir = normalize(vec3(2.0, 6.0, 1.0));
            vec3 shadowRo = hitPoint + hitNormal * 0.001;
            
            if (hitSphere(s1.center, s1.radius, shadowRo, lightDir) > 0.0 ||
                hitSphere(s2.center, s2.radius, shadowRo, lightDir) > 0.0 ||
                hitSphere(s3.center, s3.radius, shadowRo, lightDir) > 0.0) {
                shadowMult = 0.25;
            }
        }
        
        vec3 lightDir = normalize(vec3(2.0, 6.0, 1.0));
        float diff = max(dot(hitNormal, lightDir), 0.0);
        vec3 localColor = hitColor * (diff * shadowMult * 0.8 + 0.2);
        
        if (hitMirror != 0) {
            color += currentReflectance * localColor * (1.0 - u_ReflectionStrength);
            currentReflectance *= u_ReflectionStrength;
            currentRo = hitPoint + hitNormal * 0.001;
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
    col = pow(col, vec3(1.0 / 2.2));

    fragColor = vec4(col, 1.0);
}
