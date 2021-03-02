:: one vertex shader
wsl circle -shader -emit-dxil -c shaders.cxx -E vert -o vert.dxil & dxil-signing vert.dxil

:: fragment shaders
wsl circle -shader -emit-dxil -c shaders.cxx -E fractal -o fractal.dxil && dxil-signing fractal.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E clouds -o clouds.dxil && dxil-signing clouds.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E devil -o devil.dxil && dxil-signing devil.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E square -o square.dxil && dxil-signing square.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E modulation -o modulation.dxil && dxil-signing modulation.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E bands -o bands.dxil && dxil-signing bands.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E paint -o paint.dxil && dxil-signing paint.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E triangle_grid -o triangle_grid.dxil && dxil-signing triangle_grid.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E menger -o menger.dxil && dxil-signing menger.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E hypercomplex -o hypercomplex.dxil && dxil-signing hypercomplex.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E band1 -o band1.dxil && dxil-signing band1.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E band2 -o band2.dxil && dxil-signing band2.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E sphere -o sphere.dxil && dxil-signing sphere.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E segment -o segment.dxil && dxil-signing segment.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E comparison -o comparison.dxil && dxil-signing comparison.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E raymarch -o raymarch.dxil && dxil-signing raymarch.dxil
wsl circle -shader -emit-dxil -c shaders.cxx -E thumbnails -o thumbnails.dxil && dxil-signing thumbnails.dxil