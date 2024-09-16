#include <iostream>



void functionWithNestedForLoops() {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 3; ++j) {
            std::cout << "Nested for loops in function, i: " << i << ", j: " << j << std::endl;
        }
    }
}

void functionWithWhileLoop() {
    int i = 0;
    while (i < 5) {
        int j = 0;
        while (j < 3) {
            std::cout << "Nested while loops in function, i: " << i << ", j: " << j << std::endl;
            ++j;
            for(int k = 0; k < 3; ++k) {
                std::cout << "Nested for loops in while loop, i: " << i << ", j: " << j << ", k: " << k << std::endl;
            }
        }
        ++i;
    }
}

void functionWithDoWhileLoop() {
    int i = 0;
    do {
        int j = 0;
        do {
            std::cout << "Nested do-while loops in function, i: " << i << ", j: " << j << std::endl;
            ++j;
        } while (j < 3);
        ++i;
    } while (i < 5);
}

void mixedNestedLoops() {
    for (int i = 0; i < 5; ++i) {
        int j = 0;
        while (j < 3) {
            std::cout << "Mixed nested loops in function, i: " << i << ", j: " << j << std::endl;
            ++j;
        }
    }
}



int main() {
    functionWithNestedForLoops();
    functionWithWhileLoop();
    functionWithDoWhileLoop();
    mixedNestedLoops();

    // 全局范围的 for 循环，不在任何函数内部
    for (int i = 0; i < 5; ++i) {
        std::cout << "Global loop: " << i << std::endl;
    }

    // 全局范围的 while 循环，不在任何函数内部
    int k = 0;
    while (k < 3) {
        std::cout << "Global while loop: " << k << std::endl;
        ++k;
    }
    return 0;
}

