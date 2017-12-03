#include "Window.h"
#include "Instance.h"
#include "Device.h"

namespace fw {

class Framework {
public:
    Framework();
    Framework(const Framework&) = delete;
    Framework(Framework&&) = delete;
    Framework& operator=(const Framework&) = delete;
    Framework& operator=(Framework&&) = delete;

    bool initialize();
    void execute();

private:
    Instance instance;
    Window window;
    Device device;
};

} // namespace fw
