#include<stdio.h>

int main()
{
	unsigned int a[3];

	printf("Testing 0x4FFFFFFF...\n");
	__asm__("mov $0x4FFFFFFF, %eax\n\t");
	__asm__("cpuid\n\t");
	__asm__("mov %%eax, %0\n\t":"=r" (a[0]));
	__asm__("mov %%ebx, %0\n\t":"=r" (a[1]));
	__asm__("mov %%ecx, %0\n\t":"=r" (a[2]));
	printf("Value in eax register is %u\n", a[0]);
	printf("Value in ebx register is %u\n", a[1]);
	printf("Value in ecx register is %u\n", a[2]);

	printf("\nTesting 0x4FFFFFFE...\n\n");
	for (unsigned int i = 0; i < 70; i++){
		__asm__("mov $0x4FFFFFFE, %eax\n\t");
		__asm__("mov %0, %%ecx\n\t":: "m"(i));
		__asm__("cpuid\n\t");
		__asm__("mov %%eax, %0\n\t":"=r" (a[0]));
		printf("Exit Reason: %u, Count: %u\n", i, a[0]);
	}

	return 0;
}
