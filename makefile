PingpongOS: ppos-core-aux.c
	gcc -Wall -o ppos-test ppos-core-aux.c pingpong-scheduler.c libppos_static.a -g

clean:
	@rm -rf *.o ppos-test
