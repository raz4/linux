# Custom Linux

Team members: Muslim Razi

## Discovering VMX Features

1. NULL
2. To discover VMX features in a machine with an Intel CPU running Ubuntu Server 20.04, a custom kernel module was written and loaded. The module uses the RDMSR instruction to read the contents of the multiple Model Specific Registers (MSR) which reveal the available VMX features based on bit values. The semantic meaning of the bits are defined in the Intel Software Development Manual (SDM) and was used to print detailed VMX features to the kernel log.

Note: relevant files are under "custom_modules/" directory

## Modifying instruction behavior in KVM

1. NULL
2. To modify the CPUID instruction, a leaf function was added to the existing CPUID emulation implementation. This function would be executed whenever the value in %eax is 0x4FFFFFFF. Within this function, %eax is filled with the number of total exits KVM has processed across all vCPUs, %ebx is filled with the high 32 bits of the total number of CPU cycles spent during all exits, and %ecx is filled with the low 32 bits of the total number of CPU cycles spent during all exits. To calculate total exits and total exit cycles, two atomic variables were declared and initialized to 0. Total exits is incremented by 1 in the "vcpu_enter_guest" function found in x86.c file. This is also where the number of exits (separate variable) is incremented by 1 per vCPU in the original KVM implementation. To calculate total exit cycles...TODO

3. After booting the VM, immediately opening a SSH connection, and executing the [test program](test_cpuid/prog.c), the total number of exits returned was 172784164. After executing the test program multiple times in succession, the returned total number of exits seems to increase at a stable rate. Specifically, the total number of exits seems to be increase by about 200-400 exits each time the test program is executed manually in quick succession. Since CPU usage is really low, the HLT exit could make up a significant portion of the exits. During the boot/initialization process, there was a massive number of exits. After, the total number of exits didn't increase significantly compared to the initial value.
