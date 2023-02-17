p2a: Wish Shell Program

Author: Iliyas Alabdulaal

Basic overview:
	So essentially in the main function, theres a while loop that runs indefinitely 
	that prints "wish>" if in interactive mode. If a proper file is passed then it
	will run in batch mode.

	The process_command function, processes commmands, hence the name. It's basically
	a bunch of nested if and else statements. One big if for command with args, then
	else if for no args then an error else. And each if contains built-in command handling
	and also contains non-built-in command handling if the proper paths are specified.
