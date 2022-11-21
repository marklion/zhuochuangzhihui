#!/bin/bash

docker run -i --init --cap-add=SYS_ADMIN  --rm pull_pic:v.10 $1