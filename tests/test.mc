
int fun1(int a, int b)
{
	return a + b;
}

int fun1(int, int);

int main()
{
    int i = 2;

    return fun1(i, 3);
}
