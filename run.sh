#!/bin/bash

gcc -Wall -o ppos-test ppos-core-aux.c pingpong-scheduler.c libppos_static.a -g
./ppos-test