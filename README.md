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
```
---

## Build

Build with CMake or g++ directly:

```bash
mpicxx -std=c++20 -O3 -o ddd_parallel src/*.cpp -lteddy
```

Ensure the TeDDy library and Open MPI are installed and available in your system paths.

---

## Usage

General Command

```bash
mpirun <threads> -n <processes> <main> <conf_file> <divider> <state> <timer>
```

### Example

```bash
export PMIX_MCA_pcompress_base_silence_warning=1
mpirun -n 2 mpi_cloud/ddd_parallel mpi_cloud/modules/module_map.conf 0 0 n
```

### Arguments

- **threads** – To use hardware threads instead of cores 
```bash
--use-hwthread-cpus
``` 
- **processes** – Number of MPI processes to spawn 
- **main** - Path to main.exe 
- **conf_file** – Path to configuration file (default: `module_map.conf`)  
- **divider** – Module division strategy:  
  - `0` = var_count_divider  
  - `1` = node_divider  
- **state** – Availability state to calculate (`0` or `1`)  
- **timer** – Measure execution time (`y` or `n`)  

## Example workflow

1. Prepare your structure function in a configuration file (e.g., `module_map.conf`).  
2. Run DDD with MPI using the configuration file and chosen divider.  
3. The tool will:  
   1. Parse modules  
   2. Assign them to processes  
   3. Perform distributed computation with **TeDDy**  
   4. Output the overall system reliability  


---

### Example Configuration File (`module_map.conf`)

```ini
# This conf file contains mapping of modules containing .pla files in BDD

# First, we tell where our modules are placed in directories
# and which column of the PLA file should be considered as a function.
# M{num} are the names of our module variables, where {num} distinguishes them.

# Root module
M0 ../load_files/modules/Root/A and B or C and D.pla 0

# First level modules
M1 ../load_files/modules/First_Level/A or B or C.pla 0
M2 ../load_files/modules/First_Level/A and B and C.pla 0

# Second level modules
M3 ../load_files/modules/Second_Level/A and B.pla 0
M4 ../load_files/modules/Second_Level/A or B.pla 0
M5 ../load_files/modules/Second_Level/(A or B) and C.pla 0

# Second, we specify which variable in modules is another module and which is a plain variable.
# "V" means the position is a binary variable.
# "M{num}" means the position links to another module.

# Root module depends on M1 and M2
M0 M1VVM2

# M1 depends on M3
M1 VVM3

# M2 depends on M4
M2 VM4V

# M3 depends on M5
M3 M5V

# M4 is composed of two plain variables
M4 VV

# M5 is composed of three plain variables
M5 VVV

# End of configuration file
```
### Example of output

```bash
mpirun -n 2 DDD/main ../load_files/modules/module_map.conf 0 1 y
Density of 1: 0.454834
Time: 0.00101632
----------------
mpirun -n 2 DDD/main ../load_files/modules/module_map.conf 0 0 n
Density of 0: 0.545166
```

## Citation

If you use this tool in research, please cite:

F. Šefčík, M. Kvassay, M. Mrena, Distributed Topological Reliability Analysis via Modular Decomposition and MPI, IEEE IDAACS 2025.