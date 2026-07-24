#pragma once

#include <array>
#ifdef SHARK_NO_GLAD
	#include <GLES3/gl3.h>
#else
	#include <glad/glad.h>
#endif
#include <SharkEngine/util/Vector.hpp>

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
    Vector3 color;

    struct Attribute {
        GLuint index;
        GLint  size;
        GLenum type;
        std::size_t offset;
    };

    static const std::array<Vertex::Attribute, 3> attributes;
};
