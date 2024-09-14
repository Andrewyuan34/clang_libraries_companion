#include <iostream>
#include <type_traits>

// 泛型类模板声明 (ClassTemplateDecl)
template <typename T>
class MyClass {
public:
    void display() {
        std::cout << "Primary Template" << std::endl;
    }
};

// 部分特化 (ClassTemplatePartialSpecializationDecl)
template <>
class MyClass<int> {
public:
    void display() {
        std::cout << "Specialized for int" << std::endl;
    }
};

// 变量模板声明 (VarTemplateDecl)
template <typename T>
T myVariable = T(100);

// 变量模板的特化 (VarTemplateSpecializationDecl)
template <>
int myVariable<int> = 42;

// 函数模板声明 (FunctionTemplateDecl)
template <typename T>
T add(T a, T b) {
    return a + b;
}

// 函数模板的显式特化 (FunctionDecl with TemplateSpecialization)
template <>
inline int add(int a, int b) {
    std::cout << "Specialized for int" << std::endl;
    return a + b;
}

// 别名模板 (TypeAliasTemplateDecl)
template <typename T>
using MyAlias = std::add_const<T>;

// 类型别名 (TypeAliasDecl)
//using IntConst = MyAlias<int>;

// 非类型模板参数 (NonTypeTemplateParmDecl)
template <typename T, int size>
class Array {
    T arr[size];
};

// 主函数
int main() {
    MyClass<double> obj1;
    MyClass<int> obj2;

    obj1.display();  // 调用主模板
    obj2.display();  // 调用部分特化

    std::cout << "Variable template (double): " << myVariable<double> << std::endl;
    std::cout << "Variable template (int specialization): " << myVariable<int> << std::endl;

    std::cout << "Function template: " << add(1.1, 2.2) << std::endl;
    std::cout << "Function template (specialized for int): " << add(1, 2) << std::endl;

    //IntConst myInt = 10;
    //std::cout << "Const int alias: " << myInt << std::endl;

    Array<int, 5> intArray;  // 使用非类型模板参数

    return 0;
}
