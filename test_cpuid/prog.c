#include<stdio.h>

int main()
{
	unsigned int a[3];
	__asm__("mov $0x4FFFFFFF, %eax\n\t");
	__asm__("cpuid\n\t");
	__asm__("mov %%eax, %0\n\t":"=r" (a[0]));
	__asm__("mov %%ebx, %0\n\t":"=r" (a[1]));
	__asm__("mov %%ecx, %0\n\t":"=r" (a[2]));
	printf("Value in eax register is %u\n", a[0]);
	printf("Value in ebx register is %u\n", a[1]);
	printf("Value in ecx register is %u\n", a[2]);
	return 0;
}
