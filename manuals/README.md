# Intel® 64 and IA-32 Architectures Software Developer's Manuals

This directory contains the official Intel® Software Developer's Manuals (SDM), which are the definitive reference for building an OS for the x86 architecture.

These manuals cover **both 32-bit (IA-32) and 64-bit (Intel 64) architectures.** The 64-bit architecture ("long mode" or "IA-32e mode") is an extension of the 32-bit one. For OS development, this means:

1.  The 32-bit (IA-32) architecture is the foundation for everything.
2.  When a feature (like paging or interrupt handling) works differently in 64-bit mode, the manual will have a specific subsection, often titled "... in IA-32e Mode," to explain the 64-bit implementation.

## Volume Descriptions

* **`intel-sdm-vol-1.pdf` (Volume 1: Basic Architecture)**
    * Covers the application-level architecture, data types, and general-purpose registers (like `EAX`, `EBX`, etc.). Good for understanding the basic execution environment.

* **`intel-sdm-vol-2.pdf` (Volume 2: Instruction Set Reference, A-Z)**
    * This is the "dictionary" for every single x86 instruction (e.g., `MOV`, `ADD`, `LIDT`, `LGDT`, `IN`, `OUT`). It specifies which instructions are available in 32-bit vs. 64-bit modes.

* **`intel-sdm-vol-3.pdf` (Volume 3: System Programming Guide)**
    * **This is the most important manual for OS development.** It covers all the privileged topics you need, including:
        * Protected Mode (32-bit) and Long Mode (64-bit)
        * Memory Management (Paging and Segmentation)
        * Protection (Privilege Rings 0-3)
        * Interrupts and Exception Handling (IDT)
        * Task Management (TSS)
        * Multi-Processor (MP) Support

* **`intel-sdm-vol-4.pdf` (Volume 4: Model-Specific Registers)**
    * Covers Model-Specific Registers (MSRs) that control advanced, CPU-specific features (like performance monitoring, debugging, etc.). You won't need this at first, but it's useful for advanced features later.

## Which PDF is Most Important?

For the **OSOS** project, your two most important files are:

1.  **`intel-sdm-vol-3.pdf` (Your "Bible"):** You will spend 90% of your time reading this.
2.  **`intel-sdm-vol-2.pdf` (Your "Dictionary"):** You will use this as a reference whenever you see an instruction you don't understand.