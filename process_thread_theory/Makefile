CC:=gcc
file := execve_invalid_path
$(file): $(file).c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@
