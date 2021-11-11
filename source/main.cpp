#include <memory>
#include "config.hpp"
#include "fmt/color.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "math/Matrix.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/graphics/Renderer.hpp"
#include "renderer/graphics/ressources/Image.hpp"
#include "window.hpp"

int main() {
    try {
		auto mat1 = math::Matrix<float, 4, 4>(2);

        auto window = std::make_shared<Window>(WindowSpec("application", config::window_size));
        auto renderer = Renderer(window);

        auto image = Image(renderer.getInfo().device, "artistic.jpeg");

        renderer.begin();

        auto defaultVertices = GraphicsPipeline::defaultMeshRectangleVertices();
        auto defaultIndices = GraphicsPipeline::defaultMeshRectangleIndices();

        auto mesh1 = Mesh(DrawPrimitive::RECTANGLE, renderer.getInfo().device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});
        auto ubo1 = UniformObject();
        ubo1.model = glm::mat4(1.0f);
        ubo1.view = glm::mat4(1.0f);
        ubo1.proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -100.0f, 100.0f);

        renderer.draw(mesh1, ubo1);

        auto mesh2 = Mesh(DrawPrimitive::RECTANGLE, renderer.getInfo().device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});
        auto ubo2 = ubo1;
        ubo2.model = glm::translate(ubo2.model, glm::vec3(200.f, 200.f, 0.f));

        renderer.draw(mesh2, ubo2);

        auto mesh3 = Mesh(DrawPrimitive::RECTANGLE, renderer.getInfo().device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});
        auto ubo3 = ubo1;
        ubo3.model = glm::translate(ubo2.model, glm::vec3(-100.f, -100.f, 0.f));

        renderer.draw(mesh3, ubo3);

        auto mesh4 = Mesh(DrawPrimitive::RECTANGLE, renderer.getInfo().device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});
        auto ubo4 = ubo3;
        ubo4.model = glm::translate(ubo3.model, glm::vec3(150.f, 50.f, 0.f));

        renderer.draw(mesh4, ubo4);

        renderer.end();
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold, "[exception] : {}\n", e.what());
    }

    return 0;
}
