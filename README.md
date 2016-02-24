Cache Simulator
===============

A simulated memory system for a 4-processor parallel architecture, originally produced as a university coursework exercise.
Use the of the program allows tracking memory accesses and cache coherence within the system given a suitable input trace.

For more information and analysis of the functionality of the program, refer to `Report 2.pdf` within the repository.

Usage
=====

To run the simulator build script, simply use the following command:

`./build.sh`

Although unlikely, it is possible that the compiler and OS flags within the build file need to be adjusted to match your system.

The simulator takes the following arguments:
`<trace_file> <number_lines> <line_size> <buffer_size> <retireAtN> <TSO>`

### Modes

- `TSO` is set to `1` if Total Store Order is to be used
- `TSO` is set to `0` if instead you wish to use SC

This is visible within the following line of `build.sh`, and can be modified as desired (last argument):

`./sim trace1.out 128 4 32 4 1`

WARNING: You MUST use g++ for compilation - gcc breaks horribly!!!

### Verbosity:

##### To print out all hit/miss information and other statistics
- Ensure `verbose` is defined with a value of `1` within `cwk2.cpp`. (Otherwise set to `0`).

##### To print out access-level information
- Ensure `verbose` is defined within `proc.cpp` with a value of `3`
- Also ensure `verbose` is defined within `proc.cpp` with a value of `2`
 - (Only needed for debugging and to produce the artificial examples to verify functionality)
