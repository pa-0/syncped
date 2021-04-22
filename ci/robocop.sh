#!/usr/bin/env sh
# script to run robocop, excluding some rules

export root=`git rev-parse --show-toplevel`

/usr/local/bin/robocop -e 0201 -e 0202 -e 0704 -e 1003 -e 1004 ${root}
