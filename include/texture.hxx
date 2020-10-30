#include "appglfw.hxx"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#include "../thirdparty/stb/stb_image.h"

GLuint load_texture(const char* path) {
  int width, height, comp;
  stbi_uc* data = stbi_load(path, &width, &height, &comp, STBI_rgb_alpha);

  GLuint texture;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
  glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, 
    GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);

  glGenerateTextureMipmap(texture);

  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 
  return texture;
}

GLuint load_cubemap(const char* (&paths)[6]) {
  GLuint cubemap;
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap);

  int width, height, comp;
  stbi_uc* data = stbi_load(paths[0], &width, &height, &comp, STBI_rgb_alpha);

  glTextureStorage2D(cubemap, 1, GL_RGBA8, width, height);

  for(int face = 0; face < 6; ++face) {
    if(face)
      data = stbi_load(paths[face], &width, &height, &comp, STBI_rgb_alpha);

    glTextureSubImage3D(cubemap, 0, 0, 0, face, width, height, 1, GL_RGBA,
      GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
  }
  glGenerateTextureMipmap(cubemap);

  glTextureParameteri(cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return cubemap;
}
