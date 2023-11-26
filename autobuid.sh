#!/bin/bash
set -x  #执行脚本时将每条命令都打印出来
rm -rf $PWD/build/*
cd $PWD/build && cmake .. && make
