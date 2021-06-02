# Custom Linux

## Discovering VMX Features

To discover VMX features in a machine with an Intel CPU running Ubuntu Server 20.04, a custom kernel module was written and loaded. The module uses the RDMSR instruction to read the contents of the multiple Model Specific Registers (MSR) which reveal the available VMX features based on bit values. The semantic meaning of the bits are defined in the Intel Software Development Manual (SDM) and was used to print detailed VMX features to the kernel log. The 5 registers which are analyzed include the following: IA32_VMX_PINBASED_CTLS, IA32_VMX_PROCBASED_CTLS, IA32_VMX_PROCBASED_CTLS2, IA32_VMX_EXIT_CTLS, and IA32_VMX_ENTRY_CTLS. Each MSR has an index value associated with it which is passed to the RDMSR instruction through %ecx. 

Note: relevant files are under "custom_modules/" directory

## Modifying instruction behavior in KVM

To modify the CPUID emulation, a leaf function was added to the existing CPUID emulation implementation. This function would be executed whenever the value in %eax is 0x4FFFFFFF. Within this function, %eax is filled with the number of total exits KVM has processed across all vCPUs, %ebx is filled with the high 32 bits of the total number of CPU cycles spent during all exits, and %ecx is filled with the low 32 bits of the total number of CPU cycles spent during all exits. To calculate total exits and total exit cycles, two atomic variables were declared and initialized to 0. Total exits is incremented by 1 in the "vcpu_enter_guest" function found in x86.c file. This is also where the number of exits (separate variable) is incremented by 1 per vCPU in the original KVM implementation. To calculate total exit cycles, the rdtsc function is used to store the number of CPU cycles at different stages. The rdtsc function is first called before the guest is first run and the result is stored in "init_cycles" local variable. Then, the rdtsc function is called twice more to calculate the number of CPU cycles the guest had run for before exiting. The value of the counter before the guest is run is subtracted from the value of the counter after the guest exits. The resulting value is added to another local variable "cycles_spent_in_guest" which keeps track of the total CPU cycles the guest ever consumes. Now to calculate the number of cycles spent processing the exit, the following equation is used:
```
exit_cycles = current_cycles - init_cycles - cycles_spent_in_guest
```
This value is added to the 64-bit atomic variable "total_exit_cycles" which is used to fill %ebx and %ecx.

To test out the new CPUID emulation, the modified kernel was compiled and installed on a VM running Ubuntu 20.04 Desktop. Within this VM, another nested VM was created using virt-manager (GUI) and ran Ubuntu 20.04 Server. Instructions from the Ubuntu website were used to [install KVM](https://help.ubuntu.com/community/KVM/Installation) and [virt-manager](https://help.ubuntu.com/community/KVM/VirtManager). A [test program](test_cpuid/prog.c) was executed in the nested VM which sets the %eax register appropriately and calls the CPUID instruction. The contents of registers %eax, %ebx, and %ecx are printed out for validating and debugging purposes.

### Architecture Diagram of Testing Environment
![Testing Architecture](environment.png)

After booting the VM, immediately opening a SSH connection, and executing the [test program](test_cpuid/prog.c), the total number of exits returned was 172784164. After executing the test program multiple times in succession, the returned total number of exits seems to increase at a stable rate. Specifically, the total number of exits seems to be increase by about 200-400 exits each time the test program is executed manually in quick succession. Since CPU usage is really low, the HLT exit could make up a significant portion of the exits. During the boot/initialization process, there was a massive number of exits. After, the total number of exits didn't increase significantly compared to the initial value.

## Instrumentation via hypercall

First, an if statement was added to the "kvm_emulate_cpuid" function where %ecx register is read which contains the exit reason. In order to determine if this exit reason is defined in the SDM, the value is compared with the list of exit reasons provided in Volume 3 Appendix C of the SDM. If the exit reason is not in the SDM, 0xFFFFFFFF is returned in %edx and 0 in %eax, %ebx, %ecx. In order to determine which exit controls KVM has NOT enabled, the following variables "_pin_based_exec_control", "_cpu_based_exec_control", and "_cpu_based_2nd_exec_control" found in "setup_vmcs_config" function in "vmx.c" file were printed to the kernel log. The values of these variables were then compared with the reference provided in Chapter 24.6 "VM-EXECUTION CONTROL FIELDS" in Volume 3 of the SDM. Based on this, the exits not enabled by KVM were determined and checked for in the CPUID leaf function. If the exit number was not enabled by KVM, 0 was returned in %eax, %ebx, %ecx, %edx. Otherwise, %eax will contain the total number of exits for the exit number in %ecx. To keep track of the total number of exits, an array of 70 atomic variables is stored and initialized with 0. In the "__vmx_handle_exit" function in "vmx.c" file, the exit reason found in "exit_reason.basic" is captured in the "vcpu" structure. Based on this value, the corresponding atomic variable in the array is incremented by 1 on every exit.

The table below shows the number of counts for each exit reason. The first count was taken immediately after booting up and setting up a ssh connection. The second count was taken ~5 seconds later and third count was taken ~5 seconds after that. Based on the second and third counts, it seems like certain exits like #1, #10, #12, #30, #32, #40, #48, #49 increase at a small stable rate. Some exits like #0, #29, #46, $47, #54, #55 didn't change at all since boot. So, VM operations such as booting up end up with lots of exits compared to when the OS is idle with some exits not even increasing when the OS is idle after a boot.

| Exit Reason  | Count #1 | Count #2  | Count #3 |
| ------------- | ------------- | ------------- | ------------- |
| 0 | 11836 | 11836 | 11836 |
| 1 | 24653 | 28309 | 28875 |
| 2 | 0 | 0 | 0 |
| 3 | 0 | 0 | 0 |
| 4 | 0 | 0 | 0 |
| 5 | 0 | 0 | 0 |
| 6 | 0 | 0 | 0 |
| 7 | 0 | 0 | 0 |
| 8 | 0 | 0 | 0 |
| 9 | 0 | 0 | 0 |
| 10 | 135047 | 141416 | 143085 |
| 11 | 0 | 0 | 0 |
| 12 | 33124 | 38797 | 41116 |
| 13 | 0 | 0 | 0 |
| 14 | 0 | 0 | 0 |
| 15 | 0 | 0 | 0 |
| 16 | 0 | 0 | 0 |
| 17 | 0 | 0 | 0 |
| 18 | 0 | 0 | 0 |
| 19 | 0 | 0 | 0 |
| 20 | 0 | 0 | 0 |
| 21 | 0 | 0 | 0 |
| 22 | 0 | 0 | 0 |
| 23 | 0 | 0 | 0 |
| 24 | 0 | 0 | 0 |
| 25 | 0 | 0 | 0 |
| 26 | 0 | 0 | 0 |
| 27 | 0 | 0 | 0 |
| 28 | 26387 | 26387 | 26387 |
| 29 | 3 | 3 | 3 |
| 30 | 190884 | 190915 | 190945 |
| 31 | 647 | 660 | 672 |
| 32 | 85297 | 99762 | 105440 |
| 33 | 0 | 0 | 0 |
| 34 | 0 | 0 | 0 |
| 36 | 0 | 0 | 0 |
| 37 | 0 | 0 | 0 |
| 39 | 0 | 0 | 0 |
| 40 | 3465 | 3951 | 4143 |
| 41 | 0 | 0 | 0 |
| 43 | 0 | 0 | 0 |
| 44 | 0 | 0 | 0 |
| 45 | 0 | 0 | 0 |
| 46 | 6 | 6 | 6 |
| 47 | 2 | 2 | 2 |
| 48 | 13760741 | 13763192 | 13763857 |
| 49 | 28854 | 30642 | 31029 |
| 50 | 0 | 0 | 0 |
| 51 | 0 | 0 | 0 |
| 52 | 0 | 0 | 0 |
| 53 | 0 | 0 | 0 |
| 54 | 3 | 3 | 3 |
| 55 | 3 | 3 | 3 |
| 56 | 0 | 0 | 0 |
| 57 | 0 | 0 | 0 |
| 58 | 0 | 0 | 0 |
| 59 | 0 | 0 | 0 |
| 60 | 0 | 0 | 0 |
| 61 | 0 | 0 | 0 |
| 62 | 0 | 0 | 0 |
| 63 | 0 | 0 | 0 |
| 64 | 0 | 0 | 0 |
| 66 | 0 | 0 | 0 |
| 67 | 0 | 0 | 0 |
| 68 | 0 | 0 | 0 |
| 69 | 0 | 0 | 0 |

The exit with the highest absolute count is #48, and there are many exits which are 0 like #2, #3, etc. Exit #2 has a count of 2 which is the fewest nonzero count. Exit #12 increased most rapidly compared to others.

# Nested Paging vs. Shadow Paging

| Exit Reason  | With EPT | Without EPT  |
| ------------- | ------------- | ------------- |
| 0 | 11836 | 898976 |
| 1 | 24653 | 75616 |
| 2 | 0 | 0 |
| 3 | 0 | 0 |
| 4 | 0 | 0 |
| 5 | 0 | 0 |
| 6 | 0 | 0 |
| 7 | 0 | 0 |
| 8 | 0 | 0 |
| 9 | 0 | 0 |
| 10 | 135047 | 135198 |
| 11 | 0 | 0 | 0 |
| 12 | 33124 | 8169 |
| 13 | 0 | 0 |
| 14 | 0 | 0 |
| 15 | 0 | 0 |
| 16 | 0 | 0 |
| 17 | 0 | 0 |
| 18 | 0 | 0 |
| 19 | 0 | 0 |
| 20 | 0 | 0 |
| 21 | 0 | 0 |
| 22 | 0 | 0 |
| 23 | 0 | 0 |
| 24 | 0 | 0 |
| 25 | 0 | 0 |
| 26 | 0 | 0 |
| 27 | 0 | 0 |
| 28 | 26387 | 56085 |
| 29 | 3 | 2 |
| 30 | 190884 | 243147 |
| 31 | 647 | 610 |
| 32 | 85297 | 28935 |
| 33 | 0 | 0 | 0 |
| 34 | 0 | 0 | 0 |
| 36 | 0 | 0 | 0 |
| 37 | 0 | 0 | 0 |
| 39 | 0 | 0 | 0 |
| 40 | 3465 | 2550 |
| 41 | 0 | 0 |
| 43 | 0 | 0 |
| 44 | 0 | 0 |
| 45 | 0 | 0 |
| 46 | 6 | 6 |
| 47 | 2 | 2 |
| 48 | 13760741 | 0 |
| 49 | 28854 | 0 |
| 50 | 0 | 0 |
| 51 | 0 | 0 |
| 52 | 0 | 0 |
| 53 | 0 | 0 |
| 54 | 3 | 2 |
| 55 | 3 | 3 |
| 56 | 0 | 0 |
| 57 | 0 | 0 |
| 58 | 0 | 3201 |
| 59 | 0 | 0 | 0 |
| 60 | 0 | 0 | 0 |
| 61 | 0 | 0 | 0 |
| 62 | 0 | 0 | 0 |
| 63 | 0 | 0 | 0 |
| 64 | 0 | 0 | 0 |
| 66 | 0 | 0 | 0 |
| 67 | 0 | 0 | 0 |
| 68 | 0 | 0 | 0 |
| 69 | 0 | 0 | 0 |

There are some significant changes in the exit count between the two modes. The counts are mostly as expected because the different implementations of shadow paging and nested paging would lead to different exit count trends.

The exit counts between the two runs either didn't change, didn't change significantly, or changed significantly. Because the exit count was taken by executing a test program manually after boot, variation is expected in some exit counts which in theory should be the same. Exits #48, #49 are related to EPT so those are 0 with EPT disabled as expected. Exit #58 is 0 with EPT and non-zero without EPT which makes sense since this exit related to invalidating TLB. The most significant change is Exit #0 which is most probably due to page fault exiting which is a major part of the shadow paging implementation. There was a 75x increase in exit counts for Exit #0 with ept=0.
