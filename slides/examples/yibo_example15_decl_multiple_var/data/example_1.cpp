struct base{
	base() = default;
	~base() = default;
private:
	int a;
public:
	int c, d, e;
	double v, b, n;
};

class derived : public base{
public:
	derived() = default;
	~derived() = default;
	float q, w, e, r;
	int change(){
		return 0;
	}
private:
	int b, p, o;
	int a, s;
};

int d, e, f; auto g = []() -> int { return 0; };

void func(){
	derived d;
}

int main(){
	func();
	int a, b, c;
	return 0;
}


