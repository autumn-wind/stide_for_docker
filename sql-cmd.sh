#!/bin/bash

ip=`/sbin/ifconfig|grep '^en*' -A 3|sed -n '/inet addr/s/^[^:]*:\([0-9.]\{7,15\}\) .*/\1/p'`

while true
do
mysqlslap -h${ip} -P3306 -uroot -p123456  -a -x 4 -y 3 --concurrency=50 --iterations=50 --auto-generate-sql-write-number=1000 --auto-generate-sql-unique-query-number=100 --number-of-queries=1000 --auto-generate-sql-unique-query-number=100 
done
