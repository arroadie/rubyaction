#include "Texture.hpp"
#include "Application.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace RubyAction;

Texture::Texture(mrb_value self, const char *filename)
  : TextureBase(self)
{
  // load image
  unsigned char *pixels = stbi_load(filename, &this->width, &this->height, NULL, STBI_rgb_alpha);
  if (!pixels) mrb_raise(RubyEngine::getInstance()->getState(), E_RUNTIME_ERROR, "Error on load the texture.");

  // texture params
  SDL_Renderer *renderer = Application::getInstance()->getRenderer();
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 format = SDL_PIXELFORMAT_RGBA8888;
  #else
    Uint32 format = SDL_PIXELFORMAT_ABGR8888;
  #endif
  Uint32 access = SDL_TEXTUREACCESS_STATIC;

  // create texture
  this->texture = SDL_CreateTexture(renderer, format, access, this->width, this->height);

  // load texture
  SDL_Rect rect = { 0, 0, this->width, this->height };
  SDL_UpdateTexture(this->texture, &rect, pixels, STBI_rgb_alpha * this->width);

  // clean memory
  stbi_image_free(pixels);
}

Texture::~Texture()
{
  SDL_DestroyTexture(texture);
}

void Texture::render(SDL_Renderer *renderer, const SDL_Rect *srcrect, const SDL_Rect *dstrect)
{
  SDL_RenderCopy(renderer, texture, srcrect, dstrect);
}

static mrb_value Texture_initialize(mrb_state *mrb, mrb_value self)
{
  const char *filename;
  size_t length;
  mrb_get_args(mrb, "s", &filename, &length);

  SET_INSTANCE(self, new Texture(self, filename))
  return self;
}

void RubyAction::bindTexture(mrb_state *mrb, RClass *module)
{
  struct RClass *super = mrb_class_get_under(mrb, module, "TextureBase");
  struct RClass *clazz = mrb_define_class_under(mrb, module, "Texture", super);
  MRB_SET_INSTANCE_TT(clazz, MRB_TT_DATA);

  mrb_define_method(mrb, clazz, "initialize", Texture_initialize, MRB_ARGS_REQ(1));
}
