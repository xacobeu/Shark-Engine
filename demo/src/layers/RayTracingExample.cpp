#include "RayTracingExample.hpp"
#include "imgui.h"
#include <SharkEngine/Application.hpp>
#include <SharkEngine/renderer/geometry/Vertex.hpp>
#include <iostream>
#include <cmath>

void RayTracingExample::onAttach() {
    std::cout << "[DEBUG] GPU Raytracer layer attached" << std::endl;

    // Dynamically select GLSL version based on target platform (Desktop vs WebGL2)
#ifdef __EMSCRIPTEN__
    const std::string versionDirective = "#version 300 es\nprecision highp float;\n";
#else
    const std::string versionDirective = "#version 330 core\n";
#endif

    const std::string vertexShader = versionDirective + R"(
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            TexCoord = aTexCoord;
            gl_Position = vec4(aPos.xy, 0.0, 1.0);
        }
    )";

    const std::string fragmentShader = versionDirective + R"(
        out vec4 FragColor;
        in vec2 TexCoord;

        uniform float u_AspectRatio;
        uniform float u_Time;
        uniform vec3 u_CameraPos;
        uniform vec3 u_SpherePos;
        uniform float u_SphereRadius;
        uniform bool u_EnableShadows;
        uniform bool u_EnableReflections;
        uniform float u_ReflectionStrength;

        struct Sphere {
            vec3 center;
            float radius;
            vec3 color;
            bool isMirror;
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
                return t >= 0.001;
            }
            return false;
        }

        vec3 trace(vec3 ro, vec3 rd) {
            Sphere s1 = Sphere(u_SpherePos, u_SphereRadius, vec3(0.9, 0.1, 0.15), u_EnableReflections);
            Sphere s2 = Sphere(vec3(1.5, 0.5, -6.0), 0.7f, vec3(0.1, 0.8, 0.2), false);
            Sphere s3 = Sphere(vec3(-1.8, -0.2, -4.0), 0.5f, vec3(0.1, 0.2, 0.8), false);
            
            vec3 color = vec3(0.0);
            float currentReflectance = 1.0;
            
            vec3 currentRo = ro;
            vec3 currentRd = rd;

            for (int depth = 0; depth < 3; depth++) {
                float minT = 1e20;
                int hitType = 0; // 0 = sky, 1 = plane, 2 = sphere1, 3 = sphere2, 4 = sphere3
                vec3 hitNormal;
                vec3 hitColor = vec3(0.0);
                bool hitMirror = false;
                
                // Test plane
                float tPlane;
                vec3 planeNorm;
                if (hitPlane(currentRo, currentRd, tPlane, planeNorm)) {
                    if (tPlane < minT) {
                        minT = tPlane;
                        hitType = 1;
                        hitNormal = planeNorm;
                        // Procedural checkerboard floor pattern
                        vec3 hitPos = currentRo + currentRd * tPlane;
                        float checker = mod(floor(hitPos.x) + floor(hitPos.z), 2.0);
                        hitColor = checker > 0.5 ? vec3(0.3) : vec3(0.55);
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
                    // Procedural sky gradient color background
                    float t = 0.5 * (currentRd.y + 1.0);
                    vec3 skyColor = (1.0 - t) * vec3(1.0) + t * vec3(0.5, 0.7, 1.0);
                    color += currentReflectance * skyColor;
                    break;
                }
                
                // Intersection point calculation
                vec3 hitPoint = currentRo + currentRd * minT;
                
                // Shadow ray calculation
                float shadowMult = 1.0;
                if (u_EnableShadows) {
                    vec3 lightDir = normalize(vec3(2.0, 6.0, 1.0));
                    vec3 shadowRo = hitPoint + hitNormal * 0.001; // offset along normal to avoid self-intersection
                    
                    if (hitSphere(s1.center, s1.radius, shadowRo, lightDir) > 0.0 ||
                        hitSphere(s2.center, s2.radius, shadowRo, lightDir) > 0.0 ||
                        hitSphere(s3.center, s3.radius, shadowRo, lightDir) > 0.0) {
                        shadowMult = 0.25; // in shadow
                    }
                }
                
                // Apply diffuse shading calculations (Lambertian reflectance)
                vec3 lightDir = normalize(vec3(2.0, 6.0, 1.0));
                float diff = max(dot(hitNormal, lightDir), 0.0);
                vec3 localColor = hitColor * (diff * shadowMult * 0.8 + 0.2); // ambient component of 0.2
                
                if (hitMirror) {
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
            vec2 ndc = TexCoord * 2.0 - 1.0;
            vec3 ro = u_CameraPos;
            vec3 rd = normalize(vec3(ndc.x * u_AspectRatio, ndc.y, -1.0));

            vec3 col = trace(ro, rd);

            // Gamma Correction
            col = pow(col, vec3(1.0 / 2.2));

            FragColor = vec4(col, 1.0);
        }
    )";

    raytraceShader = app->getDevice()->createShaderFromSource(vertexShader, fragmentShader);

    // Quad geometry (NDC covering [-1, 1])
    Vertex quadVertices[] = {
        { {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
        { { 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },

        { {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
        { { 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
        { { 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }
    };

    quadVbo = app->getDevice()->createVertexBuffer(quadVertices, sizeof(quadVertices));
}

void RayTracingExample::onUpdate(float deltaTime) {
    time += deltaTime;

    // Dynamic sphere motion
    sphere1Pos.x = std::sin(time) * 1.5f;
    sphere1Pos.y = std::cos(time * 1.5f) * 0.5f;
}

void RayTracingExample::onRender(Renderer& renderer) {
    renderer.begin(app->getWidth(), app->getHeight());
    raytraceShader->bind();

    float aspectRatio = static_cast<float>(app->getWidth()) / static_cast<float>(app->getHeight());
    raytraceShader->setFloat("u_AspectRatio", aspectRatio);
    raytraceShader->setFloat("u_Time", time);
    raytraceShader->setVec3("u_CameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
    raytraceShader->setVec3("u_SpherePos", sphere1Pos.x, sphere1Pos.y, sphere1Pos.z);
    raytraceShader->setFloat("u_SphereRadius", sphere1Radius);
    raytraceShader->setInt("u_EnableShadows", enableShadows ? 1 : 0);
    raytraceShader->setInt("u_EnableReflections", enableReflections ? 1 : 0);
    raytraceShader->setFloat("u_ReflectionStrength", reflectionStrength);

    quadVbo->bind();
    app->getDevice()->drawArrays(6);
    quadVbo->unbind();
    raytraceShader->unbind();

    ImGui::SetNextWindowSize(ImVec2(350, 360), ImGuiCond_FirstUseEver);
    ImGui::Begin("GPU Raytracer Settings");

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Checkbox("Enable Shadows", &enableShadows);
    ImGui::Checkbox("Enable Reflections", &enableReflections);
    if (enableReflections) {
        ImGui::SliderFloat("Reflection Strength", &reflectionStrength, 0.0f, 1.0f);
    }

    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::SliderFloat3("Camera Position", &cameraPos.x, -5.0f, 5.0f);

    ImGui::Separator();
    ImGui::Text("Red Sphere Settings");
    ImGui::SliderFloat3("Sphere Center", &sphere1Pos.x, -5.0f, 5.0f);
    ImGui::SliderFloat("Sphere Radius", &sphere1Radius, 0.1f, 3.0f);

    ImGui::End();
}
