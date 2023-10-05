#include "timer.h"
#include "messageloop.h"
#include <iostream>
#include <thread>


int main(int, char **) {
    CoreUtils::MessageLoop loop;
    auto t1 = std::thread([&loop](){loop.run();});
    loop.post([]{
        std::cout << "Routine 0 called" << std::endl;
    });
    loop.post([]{
        std::cout << "Routine 1 called" << std::endl;
    });

    loop.postAndWait([]{
        std::cout << "Routine X called" << std::endl;
    });
    std::cout << "OA called" << std::endl;
    loop.postAndWait([]{
        std::cout << "Routine Y called" << std::endl;
    });
    std::cout << "OU called" << std::endl;
    loop.exit();
    t1.join();
    /*CoreUtils::Callback cb = [&loop]{
        std::cout << "Timer handler called" << std::endl;
        loop.post([]{
            std::cout << "Routine called" << std::endl;
        });
    };*/
   /* CoreUtils::Callback cb1 = [&loop]{
        std::cout << "Timer handler 1 called" << std::endl;
        loop.postAndWait([]{
            std::cout << "Routine 1 called" << std::endl;
        });
        std::cout << "Timer handler 1 returning" << std::endl;
    };*/
    //CoreUtils::Timer timer(loop, 0, std::move(cb));

//    CoreUtils::Timer timer1(loop, 10, std::move(cb1));
//    loop.run();
    return 0;
}
