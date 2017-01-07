#!/bin/bash

docker events --filter 'event=start' | sudo ./daemon
docker restart first-mysql
