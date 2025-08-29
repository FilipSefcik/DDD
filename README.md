Decision Diagram Distributor (DDD)

DDD is a C++20 application for distributed topological reliability analysis of complex systems.
It combines modular decomposition, decision diagrams (via the TeDDy library
), and distributed computing with MPI.

The tool enables evaluation of large Boolean structure functions by splitting them into submodules, distributing the workload across multiple processes, and calculating reliability efficiently.

Features

Representation of Boolean structure functions using Binary Decision Diagrams (BDDs)

Modular decomposition for efficient computation

Distributed computation with Open MPI

Support for configuration files to define modules and dependencies

Built-in timing and performance measurement utilities

Requirements

C++20 compiler (tested with g++ 11.4.1)

Open MPI (≥ 3.1.0, tested with 5.0.1)

TeDDy library (≥ 4.1.0)

Linux system (tested on AlmaLinux 9.3, but portable to other UNIX-like systems)

Installation

Clone the repository:

git clone https://github.com/FilipSefcik/DDD.git
cd DDD


Build with CMake or g++ directly:

mpicxx -std=c++20 -O3 -o ddd_parallel src/*.cpp -lteddy


Ensure the TeDDy library and Open MPI are installed and available in your system paths.

Usage
General Command
mpirun -host <hostlist> -n <processes> ./ddd_parallel <conf_file> <divider> <state> <timer>

Example
export PMIX_MCA_pcompress_base_silence_warning=1
mpirun -host kiscience:1,kiscience2:2 mpi_cloud/ddd_parallel mpi_cloud/modules/module_map.conf 0 0 n

Arguments

<hostlist> – list of hosts with process counts (e.g., kiscience:1,kiscience2:2)

<processes> – number of MPI processes to spawn

<conf_file> – path to configuration file (default: module_map.conf)

<divider> – module division strategy:

0 = var_count_divider

1 = node_divider

<state> – availability state to calculate (0 or 1)

<timer> – measure execution time (y or n)

Notes

To use hardware threads instead of cores, add the MPI flag:

--use-hwthread-cpus


If <timer> = y, the maximum execution time will be displayed at the end.

Example Workflow

Prepare your Boolean structure function in a configuration file (e.g., module_map.conf).

Run DDD with MPI using the configuration file and chosen divider.

The tool will:

Parse modules

Assign them to processes

Perform distributed computation with TeDDy

Output the overall system reliability

Citation

If you use this tool in research, please cite:

F. Šefčík, M. Kvassay, M. Mrena, Distributed Topological Reliability Analysis via Modular Decomposition and MPI, IEEE IDAACS 2025.