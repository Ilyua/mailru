CC=gcc
# Remove -g -O2 flags after debug


main: server client

clear:
	find . -maxdepth 1 -type f  -perm /111 -exec rm {} \; && rm -f *.o *.out
