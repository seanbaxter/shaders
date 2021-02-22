wsl -e circle -shader -emit-dxil -c -E vert shaders.cxx -o vert.dxil
dxil-signing.exe vert.dxil
wsl -e circle -shader -emit-dxil -c -E tesc shaders.cxx -o tesc.dxil
dxil-signing.exe tesc.dxil
wsl -e circle -shader -emit-dxil -c -E tese shaders.cxx -o tese.dxil
dxil-signing.exe tese.dxil
wsl -e circle -shader -emit-dxil -c -E frag shaders.cxx -o frag.dxil
dxil-signing.exe frag.dxil