To run the simulator build script, simply use the following command:

./build.sh

Although unlikely, it is possible that the compiler and OS flags within the build file need to be adjusted to match your system.

The simulator takes the following arguments: <trace_file> <number_lines> <line_size> <buffer_size> <retireAtN> <TSO>

TSO is set to 1 if Total Store Order should be used. Alternatively set to 0 to use SC.

This is visible within the following line of build.sh, and can be modified as desired:

./sim trace1.out 128 4 32 4 1

- TSO (1 for true, 0 to use SC)

WARNING: You MUST use g++ for compilation - gcc breaks horribly!!!

Verbosity:

To print out all hit/miss information and other statistics, ensure 'verbose' is defined with a value of 1 within cwk2.cpp. (Otherwise set to 0).

To print out access-level information, ensure 'verbose' is defined within proc.cpp with a value of 3, and also ensure 'verbose' is defined within proc.cpp with a value of 2, (Only needed for debugging and to produce the artificial examples to verify functionality)