#include "timer.h"
#include "messageloop.h"
#include <iostream>


int main(int, char **) {
    CoreUtils::MessageLoop loop;
    CoreUtils::Timer timer(loop, 1000, [&loop]{
        std::cout << "Timer handler called" << std::endl;
        loop.postRoutine([]{
            std::cout << "Routine called" << std::endl;
        });
    });
    loop.run();
    return 0;
}