Parboil is a simple MPI program that will take commands from a file and distribute them to the CPUs in the MPI group to be executed. Each command is given on a separate line in the file and is typically a shell command.

The code evolved to replace a third party routine that did not deal properly with jobs that had different resource requirements. That is, given N jobs and M processors it assumed all jobs were equal and distributed the first N/M jobs to the first processor, followed by the second group of N/M jobs to the seconds processor... This code distributes one command at a time, responding on a first-com first-served basis so that longer running jobs are distributed more appropriately.

To compile, edit the makefile to link to the appropriate libraries and run `make`. To run the code generate a file with one command per file and run `mpirun ./parboil.exe command_file`. Each command should be able to accomplish the given task from anywhere on the client server so the command name should be given in full. The easiest way to run the program is to use a wrapper script to change into the correct directory, create input files as necessary, run the code, and cleanup as needed.

The code has some error checking and sanity checking (but don't rely on them too much). It *will* fail ungracefully in the face of MPI errors.

CL