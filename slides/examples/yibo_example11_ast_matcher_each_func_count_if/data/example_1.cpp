#include <iostream>
#include <string>

// 定义一个 Functor（仿函数）类
class MyFunctor {
public:
    void operator()(int x) {
        if (x > 0) {
            std::cout << "x is positive" << std::endl;
        }
        if (x < 0) {
            std::cout << "x is negative" << std::endl;
        }
        if (x == 0) {
            std::cout << "x is zero" << std::endl;
        }
    }
};

// 定义一个包含 method 的类
class MyClass {
public:
    void myMethod(int a, int b) {
        if (a > b) {
            std::cout << "a is greater than b" << std::endl;
        }
        if (a == b) {
            std::cout << "a is equal to b" << std::endl;
        }
        if (a < b) {
            std::cout << "a is less than b" << std::endl;
        }
    }
};

namespace foo {
    // 第一个函数，包含3个if语句
    void func1(int x) {
        if (x > 10) {
            std::cout << "x is greater than 10" << std::endl;
        }
        if (x < 5) {
            std::cout << "x is less than 5" << std::endl;
        }
        if (x == 7) {
            std::cout << "x is equal to 7" << std::endl;
        }
    }

    // 第二个函数，包含2个if语句
    void func2(const std::string &str) {
        if (str == "hello") {
            std::cout << "Hello world!" << std::endl;
        }
        if (str.empty()) {
            std::cout << "Empty string" << std::endl;
        }
    }

    // 第三个函数，包含4个if语句
    void func3(int a, int b) {
        if (a > b) {
            std::cout << "a is greater than b" << std::endl;
        }
        if (a == b) {
            std::cout << "a is equal to b" << std::endl;
        }
        if (b > 0) {
            std::cout << "b is positive" << std::endl;
        }
        if (a < 0) {
            std::cout << "a is negative" << std::endl;
        }
    }
}

int main() {
    // 调用foo命名空间的函数
    foo::func1(12);
    foo::func2("hello");
    foo::func3(3, 5);

    // 顶层函数中的if语句
    int value = 0;
    if (value == 0) {
        std::cout << "Value is zero" << std::endl;
    }

    // 创建一个仿函数对象并调用
    MyFunctor functor;
    functor(5);  // 这里会触发仿函数中的if语句

    // 创建类对象并调用其method
    MyClass myClass;
    myClass.myMethod(3, 3);  // 触发类方法中的if语句

    // lambda表达式中的if语句
    auto lambda = [](int x) {
        if (x > 10) {
            std::cout << "Lambda: x is greater than 10" << std::endl;
        }
        if (x < 10) {
            std::cout << "Lambda: x is less than 10" << std::endl;
        }
    };
    lambda(15);  // 触发lambda中的if语句

    return 0;
}

