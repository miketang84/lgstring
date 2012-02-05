all:
	gcc -O2 -fPIC -shared lgstring.c -o lgstring.so -I /usr/include/lua5.1/ -llua5.1 -lm
