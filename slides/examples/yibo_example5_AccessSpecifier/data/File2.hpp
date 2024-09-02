#ifndef FILE2_HPP
#define FILE2_HPP

#include "File1.hpp"

class DerivedClass : public BaseClass {
public:
    DerivedClass();
    ~DerivedClass();

public:
    void anotherPublicMethod();
protected:
    void protectedMethod();
private:
    char privateCharMember;
private:
    int privateIntMember;
protected:
    void protectedMethod2(){
        // protected method implementation
    }
};

#endif // FILE2_HPP

