#!/bin/bash

mysqlslap -h10.10.30.72 -P3306 -uroot -p123456  -a --concurrency=50,100 --number-of-queries=1000  
