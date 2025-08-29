# Decision Diagram Distributor (DDD)

**DDD** is a C++20 application for **distributed topological reliability analysis** of complex systems.  
It combines **modular decomposition**, **decision diagrams** (via the [TeDDy library](https://github.com/MichalMrena/DecisionDiagrams)), and **distributed computing with MPI**.

The tool enables evaluation of large Boolean structure functions by splitting them into submodules, distributing the workload across multiple processes, and calculating reliability efficiently.

---

## Features

- Representation of Boolean structure functions using **Binary Decision Diagrams (BDDs)**
- **Modular decomposition** for efficient computation
- **Distributed computation** with Open MPI
- Support for **configuration files** to define modules and dependencies
- Built-in **timing and performance measurement utilities**

---

## Requirements

- **C++20 compiler** (tested with `g++ 11.4.1`)
- **Open MPI** (≥ 3.1.0, tested with 5.0.1)
- **TeDDy library** (≥ 4.1.0)
- **Linux system** (tested on AlmaLinux 9.3, but portable to other UNIX-like systems)

---

## Installation

Clone the repository:

```bash
git clone https://github.com/FilipSefcik/DDD.git
cd DDD
