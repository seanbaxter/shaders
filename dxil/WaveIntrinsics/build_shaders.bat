wsl -e circle -shader -emit-dxil -c -E wave_vert shaders.cxx -o wave_vert.dxil
dxil-signing.exe wave_vert.dxil
wsl -e circle -shader -emit-dxil -c -E wave_frag shaders.cxx -o wave_frag.dxil
dxil-signing.exe wave_frag.dxil

wsl -e circle -shader -emit-dxil -c -E mag_vert shaders.cxx -o mag_vert.dxil
dxil-signing.exe mag_vert.dxil
wsl -e circle -shader -emit-dxil -c -E mag_frag shaders.cxx -o mag_frag.dxil
dxil-signing.exe mag_frag.dxil