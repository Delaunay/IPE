#include "ipe.h"
#include "logger.h"
#include "version.h"

#include <iostream>

int main() {
    info("Testing throwing function");
    info("version hash  : {}", _HASH);
    info("version date  : {}", _DATE);
    info("version branch: {}", _BRANCH);

    ipe::init_ipe("status", 5);
    ipe::Worker w(4, "status", 5);

    std::cout << w.rendezvous(1) << "\n";

    w.set_name("new_worker");
    w.set_time(10);
    w.write();

    for(int i = 0; i < 4; i++) {
        ipe::Worker w(i, "status", 5);
    }

    std::cout << w.rendezvous(-1) << "\n";

    w.status(std::cout);
    return 0;
}
