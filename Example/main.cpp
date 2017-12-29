#include "ExampleApp.h"
#include "../Framework/Framework.h"

int main(/*int argc, char** argv*/)
{
    fw::Framework fw;
    if (fw.initialize()) {
        ExampleApp app;
        if (app.initialize()) {
            fw.setApplication(&app);
            fw.execute();
        }
    }

    return 0;
}
