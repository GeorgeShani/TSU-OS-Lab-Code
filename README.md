# TSU Operating Systems Lab Code

This repository contains various C++ programs developed for the Operating Systems lab course at **Tbilisi State University**.

## ⚙️ Requirements

- **Operating System:** Linux-based system (Ubuntu, Debian, Fedora, etc.)
- **Compiler:** `g++` (GNU C++ Compiler)
- **Basic Knowledge:** Terminal commands and C++ syntax

## 🚀 How to Compile and Run

1. Clone this repository:
   ```bash
   git clone https://github.com/GeorgeShani/TSU-OS-Lab-Code.git
   cd TSU-OS-Lab-Code
   ```

2. Compile any `.cpp` file using the `g++` compiler:
   ```bash
   g++ filename.cpp -o outputname
   ```
   Example:
   ```bash
   g++ process_creation.cpp -o process
   ```

3. Run the compiled program:
   ```bash
   ./outputname
   ```
   Example:
   ```bash
   ./process
   ```

## 🧵 Threading Programs

For programs that use threading:

```bash
g++ thread_example.cpp -o thread_demo -pthread
./thread_demo
```

## ⚠️ Important Notes

- These programs are designed specifically for **Linux environments** and will not function correctly on Windows or macOS.
- For programs using system calls like `fork()`, `exec()`, or IPC mechanisms, ensure you understand their behavior before execution.
- Some examples may create temporary files or `.fifo` special files in your directory that will remain until manually deleted.
- If you encounter permission errors when running executables, use:
  ```bash
  chmod +x outputname
  ```

## 📁 Content Overview

This repository demonstrates key Operating Systems concepts including:

- **Process Management:** Creation, termination, and synchronization
- **Inter-Process Communication:** Pipes, FIFOs, and shared memory
- **Threading:** Creation and synchronization of threads
- **Memory Management:** Allocation and deallocation techniques
- **File System Operations:** Basic I/O operations and file manipulation

## 🔍 Troubleshooting

- If compiling with newer C++ features, specify the standard:
  ```bash
  g++ -std=c++11 filename.cpp -o outputname
  ```
- For segmentation faults, use GDB to debug:
  ```bash
  g++ -g filename.cpp -o outputname
  gdb ./outputname
  ```

## 📬 Contact

Created by Giorgi Shanidze

If you have questions or suggestions, feel free to open an issue or reach out via GitHub.
