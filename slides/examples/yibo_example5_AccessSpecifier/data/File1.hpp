#ifndef FILE1_HPP
#define FILE1_HPP

class BaseClass {
public:
    BaseClass();
    virtual ~BaseClass();

public:
    void publicMethod();
protected:
    int protectedMember;
private:
    double privateMember;
};

#endif // FILE1_HPP

