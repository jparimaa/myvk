#include "../Framework/Framework.h"

int main(/*int argc, char** argv*/) {
    fw::Framework fw;
    if (fw.initialize()) { 
        fw.execute();       
    }

    return 0;
}
