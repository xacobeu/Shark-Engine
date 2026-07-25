#include "RayTracingExample.hpp"

#include "imgui.h"
#include <SharkEngine/Application.hpp>

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
        layout (location = 0) in vec2 aPos;
        out vec2 TexCoord;
        void main() {
            TexCoord = aPos * 0.5 + 0.5;
            gl_Position = vec4(aPos, 0.0, 1.0);
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

        vec3 trace(vec3 ro, vec3 rd) {
            Sphere s1 = Sphere(u_SpherePos, u_SphereRadius, vec3(0.9, 0.2, 0.2), u_EnableReflections);
            Sphere s2 = Sphere(vec3(0.0, -101.0, -5.0), 100.0, vec3(0.0), false); // Checkerboard floor

            float t1 = hitSphere(s1.center, s1.radius, ro, rd);
            float t2 = hitSphere(s2.center, s2.radius, ro, rd);

            float t = -1.0;
            vec3 baseColor = vec3(0.0);
            vec3 sphereCenter = vec3(0.0);
            float sphereRadius = 0.0;
            bool isMirror = false;

            if (t1 > 0.0 && (t2 <= 0.0 || t1 < t2)) {
                t = t1;
                baseColor = s1.color;
                sphereCenter = s1.center;
                sphereRadius = s1.radius;
                isMirror = s1.isMirror;
            } else if (t2 > 0.0) {
                t = t2;
                sphereCenter = s2.center;
                sphereRadius = s2.radius;
                isMirror = s2.isMirror;

                // Procedural checkerboard floor
                vec3 hitPoint = ro + rd * t;
                int checkX = int(floor(hitPoint.x * 0.5));
                int checkZ = int(floor(hitPoint.z * 0.5));
                if ((checkX + checkZ) % 2 == 0) {
                    baseColor = vec3(0.8);
                } else {
                    baseColor = vec3(0.3);
                }
            }

            if (t > 0.0) {
                vec3 hitPoint = ro + rd * t;
                vec3 normal = normalize(hitPoint - sphereCenter);
                vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));

                // Shadow check
                bool inShadow = false;
                if (u_EnableShadows) {
                    float shadowT1 = hitSphere(s1.center, s1.radius, hitPoint + normal * 0.001, lightDir);
                    float shadowT2 = hitSphere(s2.center, s2.radius, hitPoint + normal * 0.001, lightDir);
                    inShadow = (shadowT1 > 0.0 || shadowT2 > 0.0);
                }

                float diff = inShadow ? 0.0 : max(dot(normal, lightDir), 0.0);
                float ambient = 0.15;
                vec3 diffuseColor = baseColor * (diff * 0.8 + ambient);

                if (isMirror) {
                    // Reflection bounce
                    vec3 reflectedDir = normalize(reflect(rd, normal));
                    vec3 refRo = hitPoint + normal * 0.001;

                    float rt1 = hitSphere(s1.center, s1.radius, refRo, reflectedDir);
                    float rt2 = hitSphere(s2.center, s2.radius, refRo, reflectedDir);

                    float rt = -1.0;
                    vec3 refBaseColor = vec3(0.0);
                    vec3 refSphereCenter = vec3(0.0);

                    if (rt1 > 0.0 && (rt2 <= 0.0 || rt1 < rt2)) {
                        rt = rt1;
                        refBaseColor = s1.color;
                        refSphereCenter = s1.center;
                    } else if (rt2 > 0.0) {
                        rt = rt2;
                        refSphereCenter = s2.center;

                        vec3 refHitPoint = refRo + reflectedDir * rt;
                        int checkX = int(floor(refHitPoint.x * 0.5));
                        int checkZ = int(floor(refHitPoint.z * 0.5));
                        if ((checkX + checkZ) % 2 == 0) {
                            refBaseColor = vec3(0.8);
                        } else {
                            refBaseColor = vec3(0.3);
                        }
                    }

                    vec3 reflectionColor;
                    if (rt > 0.0) {
                        vec3 refHitPoint = refRo + reflectedDir * rt;
                        vec3 refNormal = normalize(refHitPoint - refSphereCenter);

                        bool refInShadow = false;
                        if (u_EnableShadows) {
                            float shadowT1 = hitSphere(s1.center, s1.radius, refHitPoint + refNormal * 0.001, lightDir);
                            float shadowT2 = hitSphere(s2.center, s2.radius, refHitPoint + refNormal * 0.001, lightDir);
                            refInShadow = (shadowT1 > 0.0 || shadowT2 > 0.0);
                        }

                        float refDiff = refInShadow ? 0.0 : max(dot(refNormal, lightDir), 0.0);
                        reflectionColor = refBaseColor * (refDiff * 0.8 + 0.15);
                    } else {
                        // Sky
                        float a = 0.5 * (reflectedDir.y + 1.0);
                        reflectionColor = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), a);
                    }

                    return mix(diffuseColor, reflectionColor, u_ReflectionStrength);
                }

                return diffuseColor;
            }

            // Sky
            float a = 0.5 * (rd.y + 1.0);
            return mix(vec3(1.0), vec3(0.5, 0.7, 1.0), a);
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

    m_RaytraceShader.compile(vertexShader, fragmentShader);

    // Quad geometry (NDC covering [-1, 1])
    float quadVertices[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    m_QuadVAO.create();
    m_QuadVBO.create();

    m_QuadVAO.bind();
    m_QuadVBO.bind();
    m_QuadVBO.data(sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    m_QuadVAO.unbind();
}

void RayTracingExample::onUpdate(float deltaTime) {
    m_Time += deltaTime;

    // Dynamic sphere motion
    m_Sphere1Pos.x = std::sin(m_Time) * 1.5f;
    m_Sphere1Pos.y = std::cos(m_Time * 1.5f) * 0.5f;
}

void RayTracingExample::onRender(Renderer& renderer) {
    // 1. Clear window
    renderer.begin(app->getWidth(), app->getHeight());

    // 2. Draw the full-screen raytraced background quad
    m_RaytraceShader.bind();

    float aspectRatio = static_cast<float>(app->getWidth()) / static_cast<float>(app->getHeight());
    m_RaytraceShader.setFloat("u_AspectRatio", aspectRatio);
    m_RaytraceShader.setFloat("u_Time", m_Time);
    m_RaytraceShader.setVec3("u_CameraPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
    m_RaytraceShader.setVec3("u_SpherePos", m_Sphere1Pos.x, m_Sphere1Pos.y, m_Sphere1Pos.z);
    m_RaytraceShader.setFloat("u_SphereRadius", m_Sphere1Radius);
    m_RaytraceShader.setInt("u_EnableShadows", m_EnableShadows ? 1 : 0);
    m_RaytraceShader.setInt("u_EnableReflections", m_EnableReflections ? 1 : 0);
    m_RaytraceShader.setFloat("u_ReflectionStrength", m_ReflectionStrength);

    m_QuadVAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_QuadVAO.unbind();
    m_RaytraceShader.unbind();

    // 3. Draw ImGui interface as a floating overlay window
    ImGui::SetNextWindowSize(ImVec2(350, 360), ImGuiCond_FirstUseEver);
    ImGui::Begin("GPU Raytracer Settings");

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Checkbox("Enable Shadows", &m_EnableShadows);
    ImGui::Checkbox("Enable Reflections", &m_EnableReflections);
    if (m_EnableReflections) {
        ImGui::SliderFloat("Reflection Strength", &m_ReflectionStrength, 0.0f, 1.0f);
    }

    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::SliderFloat3("Camera Position", &m_CameraPos.x, -5.0f, 5.0f);

    ImGui::Separator();
    ImGui::Text("Red Sphere Settings");
    ImGui::SliderFloat3("Sphere Center", &m_Sphere1Pos.x, -5.0f, 5.0f);
    ImGui::SliderFloat("Sphere Radius", &m_Sphere1Radius, 0.1f, 3.0f);

    ImGui::End();
}
