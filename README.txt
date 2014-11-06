READ_ME.txt

###############

Authors:
Alex King
Manuel Perez

###############

-------------------------
***SISH (SIMPLE SHELL)***
-------------------------

-WHITESPACE:
	The following characters are interpreted as whitespace: ' ' ; '\t' ; ',' ; ';' ; ':'
	As well as the any control signal. It is important to note that the Control signal
	characters still show in the the shell input but are not processed as real characters
	by the shell.

-INPUT:
	Input is read in with getChar(). It stops if an EOF file is read.
	
-SIGNAL HANDLING:
	The ctrl-C interrupt signal is disabled in our shell. ctrl-D (EOF)can be used to
	terminate it instead.

-SPECIAL CHARACTERS:
	The special characters (<,>,|,&) can be used without putting spaces between them
	and the arguments surrounding them. For input redirection, only the first instance
	of input redirection is used. The converse is true of output redirection.
	