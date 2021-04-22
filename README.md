# Custom Linux

Team members: Muslim Razi

1. NULL
2. To discover VMX features in a machine with an Intel CPU running Ubuntu Server 20.04, a custom kernel module was written and loaded. The module uses the RDMSR instruction to read the contents of the multiple Model Specific Registers (MSR) which reveal the available VMX features based on bit values. The semantic meaning of the bits are defined in the Intel Software Development Manual (SDM) and was used to print detailed VMX features to the kernel log.

Note: relevant files are under "custom_modules/" directory
