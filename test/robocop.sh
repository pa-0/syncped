#!/usr/bin/env sh
# script to run robocop, excluding some rules

/usr/local/bin/robocop -e 0201 -e 0202 -e 0309 -e 0704 -e 1003 -e 1004 ./
