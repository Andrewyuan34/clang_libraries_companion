#include <iostream>

int foo() {
    std::cout << "In function foo" << std::endl;
    return 42;
    int i = 43;  // unreachable: 在return之后
    std::cout << i << std::endl; // unreachable: 在return之后
}

int bar() {
    if (false) {
        std::cout << "This will never be printed!" << std::endl;  // unreachable: 条件永远为false
    }
    return 0;
}

void unreachable_in_loop() {
    while (true) {
        std::cout << "This loop will run forever" << std::endl;
        break;
        std::cout << "Unreachable inside loop after break" << std::endl;  // unreachable: 在break之后
    }
    std::cout << "Outside loop" << std::endl;
}

int baz(int x) {
    if (x > 10) {
        return x;
    }
    return -1;
    std::cout << "Unreachable code after return" << std::endl;  // unreachable: 在return之后
}

void unreachable_in_switch(int value) {
    switch (value) {
        case 1:
            std::cout << "Case 1" << std::endl;
            break;
        case 2:
            std::cout << "Case 2" << std::endl;
            return;
            std::cout << "Unreachable after return in case 2" << std::endl;  // unreachable: 在return之后
        default:
            std::cout << "Default case" << std::endl;
            break;
    }
    std::cout << "Outside switch" << std::endl;
}

int main() {
    foo();
    bar();
    unreachable_in_loop();
    baz(5);
    unreachable_in_switch(2);
    
    return 0;
}

