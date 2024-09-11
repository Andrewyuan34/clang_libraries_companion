int f(int x) {
	return x;
}
int main() {
	int a;
	int b;
	int c;
	int d; //deadstore
	b = 3;
	if (b) {f(42);}
	c = 5;
	int e = 0; //deadstore
	d = 7;
	//a = e + d;
	if (c) {f(42);}
	return a = f(b * c);

}

