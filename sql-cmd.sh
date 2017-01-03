#!/bin/bash

mysqlslap -h192.168.35.195 -P3306 -uroot -p123456  -a --concurrency=50,100 --number-of-queries=1000  
