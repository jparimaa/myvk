#include "Window.h"
#include "Context.h"

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
    Window window;
    Context context;
};

} // namespace fw
