
int __display(int);
int __init_display();
int __clear_display();

int main()
{
    __init_display();
    
    for (int i=1;i<=5;i++)
    {
	__clear_display();
	__display(i+11);
    }
    
    return 0;
}
