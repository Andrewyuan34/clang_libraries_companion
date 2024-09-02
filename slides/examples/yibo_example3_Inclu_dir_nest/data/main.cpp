#include "a.hpp"

int main() {
    A a;
    a.print(); // 调用 A 的方法，间接调用 B 和 C 的方法
    return 0;
}

