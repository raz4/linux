# Custom Linux

Team members: Muslim Razi

## Discovering VMX Features

1. NULL
2. To discover VMX features in a machine with an Intel CPU running Ubuntu Server 20.04, a custom kernel module was written and loaded. The module uses the RDMSR instruction to read the contents of the multiple Model Specific Registers (MSR) which reveal the available VMX features based on bit values. The semantic meaning of the bits are defined in the Intel Software Development Manual (SDM) and was used to print detailed VMX features to the kernel log. The 5 registers which are analyzed include the following: IA32_VMX_PINBASED_CTLS, IA32_VMX_PROCBASED_CTLS, IA32_VMX_PROCBASED_CTLS2, IA32_VMX_EXIT_CTLS, and IA32_VMX_ENTRY_CTLS. Each MSR has an index value associated with it which is passed to the RDMSR instruction through %ecx. 

Note: relevant files are under "custom_modules/" directory

## Modifying instruction behavior in KVM

1. NULL
2.
To modify the CPUID emulation, a leaf function was added to the existing CPUID emulation implementation. This function would be executed whenever the value in %eax is 0x4FFFFFFF. Within this function, %eax is filled with the number of total exits KVM has processed across all vCPUs, %ebx is filled with the high 32 bits of the total number of CPU cycles spent during all exits, and %ecx is filled with the low 32 bits of the total number of CPU cycles spent during all exits. To calculate total exits and total exit cycles, two atomic variables were declared and initialized to 0. Total exits is incremented by 1 in the "vcpu_enter_guest" function found in x86.c file. This is also where the number of exits (separate variable) is incremented by 1 per vCPU in the original KVM implementation. To calculate total exit cycles, the rdtsc function is used to store the number of CPU cycles at different stages. The rdtsc function is first called before the guest is first run and the result is stored in "init_cycles" local variable. Then, the rdtsc function is called twice more to calculate the number of CPU cycles the guest had run for before exiting. The value of the counter before the guest is run is subtracted from the value of the counter after the guest exits. The resulting value is added to another local variable "cycles_spent_in_guest" which keeps track of the total CPU cycles the guest ever consumes. Now to calculate the number of cycles spent processing the exit, the following equation is used:
```
exit_cycles = current_cycles - init_cycles - cycles_spent_in_guest
```
This value is added to the 64-bit atomic variable "total_exit_cycles" which is used to fill %ebx and %ecx.

To test out the new CPUID emulation, the modified kernel was compiled and installed on a VM running Ubuntu 20.04 Desktop. Within this VM, another nested VM was created using virt-manager (GUI) and ran Ubuntu 20.04 Server. A test program was executed in the nested VM which sets the %eax register appropriately and calls the CPUID instruction. The contents of registers %eax, %ebx, and %ecx are printed out for validating and debugging purposes.

### Architecture Diagram of Testing Environment
![Testing Architecture](environment.png)

3.
After booting the VM, immediately opening a SSH connection, and executing the [test program](test_cpuid/prog.c), the total number of exits returned was 172784164. After executing the test program multiple times in succession, the returned total number of exits seems to increase at a stable rate. Specifically, the total number of exits seems to be increase by about 200-400 exits each time the test program is executed manually in quick succession. Since CPU usage is really low, the HLT exit could make up a significant portion of the exits. During the boot/initialization process, there was a massive number of exits. After, the total number of exits didn't increase significantly compared to the initial value.
