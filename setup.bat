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
