#include "engine.h"
#include <memory>

int main() {

    auto graphicsEngine = std::make_unique<Engine>();
    graphicsEngine->Init();
    graphicsEngine->Quit();

    return 0;
}