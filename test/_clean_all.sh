#!/bin/sh

for i in 1 2
do
  D="test$i"
  if [ -d "$D" ]
  then
    cd "$D"
    make clean
    cd -
  fi
done

