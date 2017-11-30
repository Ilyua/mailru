CC=gcc
# Remove -g -O2 flags after debug


main: clear
	gcc  functions.c server.c  -o server
	gcc  functions.c client.c  -o client  

clear:
	find . -maxdepth 1 -type f  -perm /111 -exec rm {} \; && rm -f *.o *.out *.txt
