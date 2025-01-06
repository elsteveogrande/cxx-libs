#include <cassert>
#include <cxx/StackTrace.h>
#include <iostream>

int main(int, char** argv) {
    std::cout << std::endl << argv[0] << std::endl;
    cxx::StackResolver sr;
    cxx::StackTrace s;
    s.resolve(sr);
    s.dump(std::cout);
    return 0;
}
