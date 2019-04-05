#
p5:     p5.c
	cc -c -g p5.c
	cc -o p5 p5.o

clean:
	rm -f p5
	rm -f *.o
