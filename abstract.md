## Abstract

The Circle C++ Shaders extension aims to bring the entire capability of the OpenGL Shading Language (GLSL) into C++ as a first-class embedded language. Write graphics code using Standard C++ and mark interface variables and shader functions with C++ attributes to indicate their role in the graphics pipeline. When the program is compiled, all shader declarations are lowered to the SPIR-V intermediate representation, which is the portable shader storage format for Vulkan and OpenGL programs. 

The guiding vision behind this extension was to make shader programming feel idiomatic to both C++ and GLSL users. Features imported from GLSL include vector component swizzling, layout modifiers, and the complete set of GLSL texture, derivative and utility functions. By embedding these features into a C++ compiler frontend, most C++ language features are now available for shader programming, including classes, templates and concepts. Circle's features go beyond Standard C++, providing shader developers with metaprogramming tools like reflection and user-attributes.

## Bio

Sean Baxter is an independent programmer working on the next-gen C++ compiler, Circle. He previously worked in scientific computing in the Earth Science directorate at JPL, developed molecular dynamics simulations at DE Shaw Research, and hacked on CUDA algorithms at NVIDIA Research.