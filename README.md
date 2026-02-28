# 🧠 Cortex-0: AI-Native Operating System
### *A Ring-0 Bare-Metal Environment for High-Performance AI Compute*

**Hackathon Track:** Open Innovation (AI & Systems)  
**Status:** Pre-Alpha (Qualifier MVP)  

---

## 🚀 The Problem: The "OS Tax"
Currently, to run a local AI model, the compute must travel through massive layers of bloat: a heavy Operating System (Windows/Linux) -> Python Interpreters -> High-level Libraries (PyTorch) -> Hardware APIs -> and *finally* the Silicon. 

This creates massive latency, context-switching overhead, and wastes RAM on background OS tasks like drawing desktop animations and managing Bluetooth.

## 💡 The Solution: Cortex-0
Cortex-0 is a custom 32-bit Operating System built from scratch in C and x86 Assembly. It eliminates the "OS Tax" by running AI logic directly in **Kernel Space (Ring 0)**. 
- **Zero Overhead:** No background processes, no bloated standard libraries.
- **Microsecond Latency:** The AI *is* the operating system.
- **Bare-Metal Telemetry:** Direct communication with the CPU architecture.

## ⚡ Current Features (Qualifier MVP)
The current MVP establishes the critical "nervous system" required to host a local AI on bare metal:
1. **Custom Memory & Interrupts:** Hand-coded Global Descriptor Table (GDT) and Interrupt Descriptor Table (IDT) to stabilize the hardware.
2. **Interactive Kernel Shell:** A custom circular input buffer and string comparison logic (built without `<string.h>`).
3. **Hardware Telemetry:** Real-time CPU Cycle counting using the `rdtsc` inline assembly instruction to read the processor's raw hardware heartbeat.
4. **Simulated AI Scheduler:** Task allocation logic embedded directly in the main kernel loop.

## 🛠️ Technical Stack
- **Bootloader:** Custom Assembly (Multiboot Compliant)
- **Kernel:** C (Freestanding, bare-metal)
- **Drivers:** VGA Text Mode, PS/2 Keyboard, Programmable Interrupt Controller (PIC)
- **Testing Environment:** QEMU emulator

## 💿 How to Run the Demo
You can boot Cortex-0 directly using the provided ISO file:
1. Download `myos.iso` from this repository.
2. Run it in an emulator like QEMU:
   ```bash
   qemu-system-i386 -cdrom myos.iso
