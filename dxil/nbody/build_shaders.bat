wsl -e circle -shader -emit-dxil -c -E integrate_shader shaders.cxx -o integrate.dxil
dxil-signing.exe integrate.dxil
wsl -e circle -shader -emit-dxil -c -E vert             shaders.cxx -o vert.dxil
dxil-signing.exe vert.dxil
wsl -e circle -shader -emit-dxil -c -E geom             shaders.cxx -o geom.dxil 
dxil-signing.exe geom.dxil
wsl -e circle -shader -emit-dxil -c -E frag             shaders.cxx -o frag.dxil
 dxil-signing.exe frag.dxil