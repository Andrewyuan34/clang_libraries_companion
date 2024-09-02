#include <iostream>
#include <stdexcept>

[[noreturn]]
void terminateProgram() {
    throw std::runtime_error("Terminating program");
}

[[deprecated("Use newFunction() instead")]]
void oldFunction() {
    std::cout << "This function is deprecated.\n";
}

// Clang-specific attribute: This attribute forces the compiler to always inline the function
[[clang::always_inline]]
void alwaysInlineFunction() {
    std::cout << "This function will always be inlined.\n";
}

// Clang-specific attribute: This attribute specifies that the function should not be inlined
[[clang::noinline]]
void noInlineFunction() {
    std::cout << "This function will never be inlined.\n";
}

void switchExample(int x) {
    switch (x) {
        case 1:
            std::cout << "Case 1\n";
            [[fallthrough]];
        case 2:
            std::cout << "Case 2 (fallthrough from Case 1)\n";
            break;
        default:
            std::cout << "Default case\n";
    }
}

[[nodiscard]]
int computeValue() {
    return 42;
}

[[maybe_unused]]
void maybeUnusedFunction() {
    int unusedVar [[maybe_unused]] = 10;
}

// 将 likely/unlikely 替换为普通的分支预测代码
void likelyExample(int x) {
    if (x > 0) {  // 移除 [[likely]]
        std::cout << "Likely positive\n";
    } else {
        std::cout << "Unlikely negative\n";
    }
}

void carriesDependencyExample([[carries_dependency]] int* data) {
    // Simulate dependency carrying
    std::cout << "Carries dependency example\n";
}

struct MyStruct {
    [[no_unique_address]] int a;
    int b;
};

// 将 assume 替换为简单的静态断言
void assumeExample() {
    static_assert(1 + 1 == 2, "Math error: 1 + 1 must equal 2");
    std::cout << "Assume example, this will always print\n";
}

int main() {
    try {
        terminateProgram(); // Will throw and terminate
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    oldFunction(); // Will generate a deprecation warning

    alwaysInlineFunction(); // This function will be inlined
    noInlineFunction();     // This function will never be inlined

    switchExample(1);

    int value = computeValue(); // Using the nodiscard function
    (void)value; // Avoid unused variable warning

    maybeUnusedFunction(); // Call the maybe_unused function

    likelyExample(10);

    int data = 42;
    carriesDependencyExample(&data);

    MyStruct s{1, 2};
    assumeExample();

    return 0;
}
