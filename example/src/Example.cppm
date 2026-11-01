export module examples.base;

import shark;

export namespace examples {

class Example : public shark::Layer {
public:
    virtual void renderControls() {}

    [[nodiscard]] virtual bool hasControls() const { return true; }
};

} // namespace examples
