#!/bin/sh

CPPFLAGS+=" -Og -g -Wall -Wextra -pedantic -pedantic-errors -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wsuggest-override -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel	\
	-I../../debug/mingw64/include"
CXXFLAGS+=" -Og -g -std=c++17 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt"
LDFLAGS+=" -Og -nostdlib -static -L../../debug/mingw64/lib -lmcf -lstdc++ -lmcfcrt -lmingwex -lgcc -lgcc_eh -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_ExeStartup"

cp -fp ../../debug/mingw64/bin/*.dll ./

x86_64-w64-mingw32-g++ ${CPPFLAGS} ${CXXFLAGS} main.cpp ${LDFLAGS}
