#!/bin/sh
rm -f router
cd src/easygl
make
cd ../router
make
cd ../..
cp src/router/router ./

