#include "engine.h"
#include <memory>

int main() {

    auto graphicsEngine = std::make_unique<Engine>();
    graphicsEngine->Init();

    auto vertexpath = "../shader/vert.spv";
    auto fragpath = "../shader/frag.spv";

#if defined(WIN32)
    vertexpath = "../../shader/vert.spv";
    fragpath = "../../shader/frag.spv";
#endif

    auto vertexShader = graphicsEngine->CreateShaderModule(vertexpath);
    auto fragShader = graphicsEngine->CreateShaderModule(fragpath);

    graphicsEngine->CreatePipeline(vertexShader,fragShader);

    graphicsEngine->Run();
    graphicsEngine->Quit();

    return 0;
}