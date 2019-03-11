#!/bin/bash

cd build
ctest -R "_test$"
cd ..
