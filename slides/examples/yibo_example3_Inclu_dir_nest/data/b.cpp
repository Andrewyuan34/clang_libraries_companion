#include <iostream>
#include "b.hpp"

void B::print() {
    std::cout << "Inside class B" << std::endl;
    c.print(); // 调用 C 的方法
}

