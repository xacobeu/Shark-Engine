export module shark:renderer.backends.IShader;

import std;

export namespace shark {
class IShader {
public:
    virtual ~IShader() = default;

    IShader(const IShader&)            = delete ("GPU resources are shared via std::shared_ptr");
    IShader& operator=(const IShader&) = delete ("GPU resources are shared via std::shared_ptr");

    IShader(IShader&&)            = default;
    IShader& operator=(IShader&&) = default;

    virtual void bind()                                                                        = 0;
    virtual void unbind()                                                                      = 0;
    virtual void setMat4(const std::string& name, const float* matrixData)                     = 0;
    virtual void setInt(const std::string& name, int value)                                    = 0;
    virtual void setFloat(const std::string& name, float value)                                = 0;
    virtual void setVec2(const std::string& name, float x, float y)                            = 0;
    virtual void setVec3(const std::string& name, float x, float y, float z)                   = 0;
    virtual void setVec4(const std::string& name, float x, float y, float z, float w)          = 0;
    virtual void setVec4Array(const std::string& name, const float* values, std::size_t count) = 0;
    virtual void setUniformBlock(const std::string& name, const void* data, std::size_t size)  = 0;

protected:
    IShader() = default;
};

} // namespace shark
