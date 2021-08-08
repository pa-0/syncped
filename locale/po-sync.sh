#!/bin/bash

function find_sed()
{
  gsed --h > /dev/null

  if [ $? == 0 ]
  then
    export _sed=gsed
  else
    export _sed=sed
  fi
}

function substitute 
{
  ${_sed} -i "s/SOME.*/syncped localization file $1/" "$1"
  ${_sed} -i "s/YEAR/2017-2021/" "$1"
  ${_sed} -i "s/PACKAGE/syncped/" "$1"
  ${_sed} -i "s/VERSION/22.04/" "$1"
  ${_sed} -i "/FIRST AUTHOR.*/d" "$1" 
  ${_sed} -i "s/charset=CHARSET/charset=UTF-8/" "$1" 
}

find_sed

# file locations
locs="../*.cpp ../*.h"

# create pot file
xgettext -F -k_ -o syncped.pot --no-location --copyright-holder="A.M. van Wezenbeek" $locs

# merge (join) all po files
for f in *.po; do
  xgettext -F -j -k_ -o "$f" --no-location --copyright-holder="A.M. van Wezenbeek" $locs
  substitute "$f"
done  
