#pragma once

#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/Renderer.hpp>
#include <SharkEngine/renderer/GL/GLObject.hpp>
#include <SharkEngine/util/Vector.hpp>

class RayTracingExample : public Layer {
public:
    void onAttach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    GL::Shader m_RaytraceShader;
    GL::VertexArray m_QuadVAO;
    GL::VertexBuffer m_QuadVBO;
    
    float m_Time = 0.0f;
    bool m_EnableShadows = true;
    bool m_EnableReflections = true;
    float m_ReflectionStrength = 0.2f;
    
    // Scene objects
    Vector3 m_Sphere1Pos{0.0f, 0.0f, -5.0f};
    float m_Sphere1Radius = 1.0f;
    Vector3 m_CameraPos{0.0f, 0.0f, 0.0f};
};
