## Instrumentation via hypercall

1. NULL

2. First, an if statement was added to the "kvm_emulate_cpuid" function where %ecx register is read which contains the exit reason. In order to determine if this exit reason is defined in the SDM, the value is compared with the list of exit reasons provided in Volume 3 Appendix C of the SDM. If the exit reason is not in the SDM, 0xFFFFFFFF is returned in %edx and 0 in %eax, %ebx, %ecx. In order to determine which exit controls KVM has NOT enabled, the following variables "_pin_based_exec_control", "_cpu_based_exec_control", and "_cpu_based_2nd_exec_control" found in "setup_vmcs_config" function in "vmx.c" file were printed to the kernel log. The values of these variables were then compared with the reference provided in Chapter 24.6 "VM-EXECUTION CONTROL FIELDS" in Volume 3 of the SDM. Based on this, the exits not enabled by KVM were determined and checked for in the CPUID leaf function. If the exit number was not enabled by KVM, 0 was returned in %eax, %ebx, %ecx, %edx. Otherwise, %eax will contain the total number of exits for the exit number in %ecx. To keep track of the total number of exits, an array of 70 atomic variables is stored and initialized with 0. In the "__vmx_handle_exit" function in "vmx.c" file, the exit reason found in "exit_reason.basic" is captured in the "vcpu" structure. Based on this value, the corresponding atomic variable in the array is incremented by 1 on every exit.

3. The table below shows the number of counts for each exit reason. The first count was taken immediately after booting up and setting up a ssh connection. The second count was taken ~5 seconds later and third count was taken ~5 seconds after that. Based on the second and third counts, it seems like certain exits like #1, #10, #12, #30, #32, #40, #48, #49 increase at a small stable rate. Some exits like #0, #29, #46, $47, #54, #55 didn't change at all since boot. So, VM operations such as booting up end up with lots of exits compared to when the OS is idle with some exits not even increasing when the OS is idle after a boot.

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

4. The exit with the highest absolute count is #48, and there are many exits which are 0 like #2, #3, etc. Exit #2 has a count of 2 which is the fewest nonzero count. Exit #12 increased most rapidly compared to others.
