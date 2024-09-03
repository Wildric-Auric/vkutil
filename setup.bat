mkdir vendor
git   clone https://github.com/Wildric-Auric/NWin.git
move .\NWin .\vendor\nwin
del  .\vendor\nwin\src\gl_context*
del  .\vendor\nwin\src\main.cpp
del  .\vendor\nwin\premake5.lua
move .\vendor\nwin\src\*.* .\vendor\nwin\

git clone https://github.com/nothings/stb.git
move stb .\vendor\stb

git clone https://github.com/Wildric-Auric/LWmath.git
move LWmath .\vendor\lwmath

premake5 vs2022

mkdir build
mkdir .\build\bin

glslc src\shdrs\tri.vert  -o build/bin/trivert.spv
glslc src\shdrs\tri.frag  -o build/bin/trifrag.spv
glslc src\shdrs\test.comp -o build/bin/testcomp.spv
glslc src\shdrs\test.tesc -o build/bin/testtesc.spv
glslc src\shdrs\test.tese -o build/bin/testtese.spv
glslc src\shdrs\test.geom -o build/bin/testgeom.spv
