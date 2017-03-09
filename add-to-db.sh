#!/bin/bash

docker events --filter 'event=start' | sudo ./daemon -a
