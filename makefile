PingpongOS: ppos-core-aux.c
	gcc -Wall -o ppos-test ppos-core-aux.c pingpong-contab-prio.c libppos_static.a -g

clean:
	@rm -rf *.o ppos-test
