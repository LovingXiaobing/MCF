#!/bin/sh

CPPFLAGS+=" -O3 -DNDEBUG -Wall -Wextra -pedantic -pedantic-errors -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=core2 -mtune=intel -mno-stack-arg-probe -masm=intel	\
	-I../../release/mingw32/include"
CXXFLAGS+=" -O3 -std=c++17 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -Wsuggest-override -fnothrow-opt"
LDFLAGS+=" -O3 -nostdlib -L../../release/mingw32/lib -lmcf -lstdc++ -lmcfcrt -lmingwex -lgcc -lgcc_s -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_ExeStartup"

cp -fp ../../release/mingw32/bin/*.dll ./

i686-w64-mingw32-g++ ${CPPFLAGS} ${CXXFLAGS} main.cpp ${LDFLAGS}
