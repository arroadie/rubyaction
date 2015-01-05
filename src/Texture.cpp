#include "Texture.hpp"

using namespace RubyAction;

Texture::Texture(mrb_value self, const char *filename)
  : TextureBase(self)
{
  texture.loadFromFile(filename);
  sprite.setTexture(texture);

  sf::Vector2u size = texture.getSize();
  width = size.x;
  height = size.y;
}

void Texture::render(sf::RenderTarget &target, const sf::Transform &transform, const sf::IntRect &rect,
  const sf::Color &color)
{
  sprite.setColor(color);
  sprite.setTextureRect(rect);
  target.draw(sprite, transform);
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
