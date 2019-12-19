run:
	clang -g -o meshell -lsqlite3 src/{main,commands,shell,token,db}.c
	./meshell
