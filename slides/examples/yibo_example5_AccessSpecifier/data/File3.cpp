#include "File2.hpp"

class FinalClass : public DerivedClass {
public:
    FinalClass();
    ~FinalClass();

public:
    void finalPublicMethod();
protected:
    void finalProtectedMethod();
private:
    float finalPrivateMember;
};

FinalClass::FinalClass() {
    // constructor implementation
}

FinalClass::~FinalClass() {
    // destructor implementation
}

void FinalClass::finalPublicMethod() {
}

void FinalClass::finalProtectedMethod() {
    // protected method implementation
}

int main() {
    FinalClass obj;
    obj.finalPublicMethod();
    return 0;
}


