#!/bin/sh

N_JOBS=8

# make -j8 && make -j8 modules && make -j8 modules_install && make -j8 install && make -j8 install
#make -j${N_JOBS} && make -j${N_JOBS} modules && make -j${N_JOBS} modules_install && make -j${N_JOBS} install && make -j${N_JOBS} install
make -j${N_JOBS} && make -j${N_JOBS} modules && make -j${N_JOBS} modules_install && make -j${N_JOBS} install
