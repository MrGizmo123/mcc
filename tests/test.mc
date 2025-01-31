int global_var = 0;

int test_func(int a1, int b1, int c1)
{
    int result = a1 - b1 + c1;
    return result;
}

int test_func(int,int,int);

int main(int x, int y)
{
    int k = x + y;
    int z = test_func(k+y,y,3);

    return 0;
}
