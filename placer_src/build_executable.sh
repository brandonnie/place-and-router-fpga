#!/bin/sh
rm -f placer
cd SuiteSparse-5.10.1/SuiteSparse_config/
make clean
make library
echo "finish building SuiteSparse_config"
cd ../AMD/
make clean
make library
echo "finish building AMD"
cd ../UMFPACK/
make clean
make library
echo "finish building UMFPACK"
cd Demo/
make purge
make placer
cd ../../..
cp SuiteSparse-5.10.1/UMFPACK/Demo/placer ./

