@echo off
set CC=C:\MinGW\mingw64\bin\gcc.exe
set CXX=C:\MinGW\mingw64\bin\g++.exe
cmake -B build -G "Ninja" -DCMAKE_C_COMPILER=C:/MinGW/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/MinGW/mingw64/bin/g++.exe
cmake --build build
