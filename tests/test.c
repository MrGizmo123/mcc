int main()
{
    int a,b,c;
    a = 1;
    b = 1;
    
    for (int i=0;i<10;i++)
    {
	c = a + b;
	a = b;
	b = c;
    }
}
