#include <iostream>
#include "a.hpp"

void A::print() {
    std::cout << "Inside class A" << std::endl;
    b.print(); // 调用 B 的方法
}

