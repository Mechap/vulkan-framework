#pragma once

#include <memory>
#include <span>

#include "renderer/graphics/ressources/Mesh.hpp"

class Instance;
class Device;
class Swapchain;
class RenderPass;

class DescriptorSetLayout;
class DescriptorPool;
class DescriptorSet;

class Buffer;

class GraphicsPipeline;
struct VertexInputDescription;
struct ShaderResource;

class Window;

enum class DrawPrimitive {
    TRIANGLE,
    RECTANGLE,
};

class Renderer {
  public:
    struct RendererInfo {
        std::shared_ptr<Instance> instance{nullptr};
        std::shared_ptr<Device> device{nullptr};
        std::shared_ptr<Swapchain> swapchain{nullptr};
        std::shared_ptr<RenderPass> render_pass{nullptr};

        std::shared_ptr<Window> window{nullptr};

        std::shared_ptr<DescriptorSetLayout> descriptor_set_layout{nullptr};
        std::shared_ptr<DescriptorPool> descritptor_pool{nullptr};

        std::shared_ptr<GraphicsPipeline> graphics_pipeline{nullptr};
    };

  public:
    explicit Renderer(std::shared_ptr<Window> _window);

    void begin(DrawPrimitive mode);
    void draw(Mesh &_mesh);
    void end();

    ~Renderer();

  private:
    void createGraphicsPipeline();

  private:
    RendererInfo renderer_info;
    Mesh mesh;

	std::vector<Buffer> vbos;
};
