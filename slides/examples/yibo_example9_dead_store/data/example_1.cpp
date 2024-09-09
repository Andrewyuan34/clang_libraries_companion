int f(int x) {
	return x;
}
int main() {
	int a;
	int b;
	int c;
	int d = 0; //deadstore
	b = 3;
	if (b) {f(42);}
	c = 5;
	int e = 0; //deadstore
	//a = e + d;
	if (c) {f(42);}
	return a = f(b * c);

}

