#!/bin/bash

while true
do
mysqlslap -h192.168.35.196 -P3306 -uroot -p123456  -a -x 4 -y 3 --concurrency=50 --iterations=50 --auto-generate-sql-write-number=1000 --auto-generate-sql-unique-query-number=100 --number-of-queries=1000 --auto-generate-sql-unique-query-number=100 
done
