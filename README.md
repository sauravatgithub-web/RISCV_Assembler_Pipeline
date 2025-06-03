# RISC-V Assembler and Pipeline

## Overview
This project features two main scripts:
- `assembler.hpp` – Converts RISC-V assembly to machine code.
- `pipeline.cpp` – Simulates the execution pipeline using machine code.

Also included:
- `RISCV_CARD` – A reference sheet covering instruction types and formats.
- `Live Examples` – Sample assembly files and their binary equivalents for real-time learning and testing.

## Table of Contents
- [Overview](#overview)
- [Supported Instructions](#supported-instructions)
- [Additional Assembly Support](#additional-assembly-support)
- [Getting Started](#getting-started)
- [Other Features](#other-features)
- [Contributing](#contributing)

## Supported Instructions

| Type   | Instructions |
|--------|--------------|
| **R**  | `add`, `sub`, `and`, `or`, `xor`, `sll`, `srl`, `sra`, `slt`, `sltu` |
| **I**  | `addi`, `xori`, `ori`, `andi`, `slli`, `srli`, `srai`, `slti`, `sltiu`, `jalr` |
| **L**  | `lb`, `lh`, `lw`, `lbu`, `lhu` |
| **S**  | `sb`, `sh`, `sw` |
| **B**  | `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu` |
| **J**  | `jal` |
| **U**  | `lui`, `auipc` |

## Additional Assembly Support

| Feature | Description |
|--------|-------------|
| **Register Naming** | Supports both numeric (`x1`, `x2`, ...) and ABI names (`a0`, `sp`, ...) |
| **Comments** | Supported using `#`, e.g., `addi x1, zero, 7  # x1 := 7` |
| **Negative Immediates** | `addi x1, zero, -4` assigns `-4` to `x1` |
| **Hexadecimal Immediates** | `addi x1, zero, 0x1a` assigns `26` to `x1` |
| **Labels** | Custom string labels like `for_loop:` are supported |

### Examples
```asm
xor a0, zero, x3
addi sp, x25, 12
# this is a comment
addi x1, zero, 7        # x1 := 7
addi x1, zero, -4       # x1 := -4
addi x1, zero, 0x1a     # x1 := 26

for_loop:
    addi x2, x2, -1
    bne x2, zero, for_loop
```

## Getting Started

### Prerequisites
To use this project, you’ll need:
- A C++ compiler (`g++`, `clang++`, etc.)
- A basic understanding of RISC-V assembly (helpful, not required)

### Setup Instructions

1. **Prepare the Assembly File:**
   Create an `assembly.txt` file in the same directory containing your RISC-V assembly code.

2. **Compile the Pipeline:**
   Open your terminal and run:
   ```bash
   g++ -o pipeline pipeline.cpp
   ```
3. **Run the Program**
   ```bash
   ./pipeline
   ```
   This will:
    - Read and parse `assembly.txt`
    - Generate `binary.txt` containing the machine code
    - Simulate instruction execution and display:
      - Register values
      - Memory state
      - Step-by-step instruction execution
## Other Features

- ✅ **User-Friendly Syntax:** Accepts both ABI and numeric register names (e.g., `a0`, `x10`)
- 🧠 **Robust Parsing:** Handles labels, comments (`#`), and various immediate formats (decimal, negative, hex)
- ⚙️ **Offset Calculation:** Accurately computes jump/branch offsets beyond textbook examples
- ✨ **Formatted Output:** Clear terminal output of registers, memory, and instruction cycles
- 🔡 **Case-Insensitive:** Write `ADD`, `Add`, or `add`—it all works
- 📁 **Live Examples Folder:** Run included `.s` files and explore the output
- 📝 **Instruction Reference Card (RISCV_CARD):** Handy cheat sheet for quick lookup

---

## Contributing

Contributions are welcome and appreciated!

**Ways to contribute:**
- 🐛 Report bugs by opening an issue
- 🧪 Add test cases or live example files
- 📚 Improve documentation or the `RISCV_CARD`
- 💡 Suggest new features or instruction support

**To contribute code:**
1. Fork the repository
2. Create a new branch:
   ```bash
   git checkout -b feature-name
   ```
