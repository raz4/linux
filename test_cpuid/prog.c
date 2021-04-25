#include<stdio.h>

int main()
{
	int a;
	__asm__("mov $0x4FFFFFFF, %eax\n\t");
	__asm__("cpuid\n\t");
	__asm__("mov %%eax, %0\n\t":"=r" (a));
	printf("Value in eax register is %d\n", a);
	return 0;
}
