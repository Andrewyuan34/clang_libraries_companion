struct base{
	base() = default;
	~base() = default;
private:
	int a;
public:
	int c;
};

class derived : public base{
public:
	derived() = default;
	~derived() = default;
private:
	int b;
};

void func(){
	derived d;
}

