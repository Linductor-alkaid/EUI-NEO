#include "eui_neo.h"
#include "eui/detail/dsl_app_impl.h"
#include "core/render/render_backend.h"
#include "core/window/window_backend.h"

#include <glad/glad.h>
#if defined(EUI_WINDOW_BACKEND_SDL2)
#define SDL_MAIN_HANDLED
#include <SDL.h>
#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <iostream>
#include <stdexcept>

namespace {
int starts = 0;
int shutdowns = 0;
int composes = 0;
bool startBeforeFirstCompose = false;

void require(bool condition, const char* message) {
    if (!condition)
        throw std::runtime_error(message);
}
}

namespace app {
const DslAppConfig& dslAppConfig() {
    static const auto config = DslAppConfig{}
                                   .title("DSL app lifecycle probe")
                                   .onStart([] { ++starts; })
                                   .onShutdown([] { ++shutdowns; });
    return config;
}

void compose(eui::Ui& ui, const eui::Screen&) {
    ++composes;
    startBeforeFirstCompose = starts == 1;
    ui.text("lifecycle.probe").text("ready").build();
}
} // namespace app

int main() {
    core::render::initializeRenderBackendLoader();
#if defined(EUI_WINDOW_BACKEND_SDL2)
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 1;
#else
    if (!glfwInit())
        return 1;
#endif
    core::window::WindowCreateRequest request;
    request.width = 320;
    request.height = 240;
    request.title = "DSL app lifecycle probe";
    request.renderApi = core::window::RenderApi::OpenGL;
    auto window = core::window::createWindow(request);
    auto backend = core::render::createRenderBackend(window);
    int result = 0;
    try {
        require(window && backend && backend->initialize(), "window backend initialization");
        backend->makeCurrent();
        core::render::ScopedRenderBackend scope(*backend);
        require(app::initialize(window), "app initialization");
        require(starts == 1 && composes == 0, "onStart did not run once before first compose");
        require(app::update(window, 0, 320, 240, 1, 1, false), "initial app update");
        require(composes == 1 && startBeforeFirstCompose, "first compose preceded onStart");
        app::shutdown();
        require(shutdowns == 1, "onShutdown did not pair with onStart");
        std::cout << "dsl_app_lifecycle_probe: passed\n";
    } catch (const std::exception& error) {
        std::cerr << "dsl_app_lifecycle_probe: " << error.what() << '\n';
        result = 1;
        app::shutdown();
    }
    backend.reset();
    core::window::destroyWindow(window);
#if defined(EUI_WINDOW_BACKEND_SDL2)
    SDL_Quit();
#else
    glfwTerminate();
#endif
    return result;
}