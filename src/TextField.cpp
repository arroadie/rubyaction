#include "TextField.hpp"
#include "FontBase.hpp"
#include "util/array.hpp"

using namespace RubyAction;

TextField::TextField(mrb_value self, mrb_value font, const char *text)
  : Sprite(self),
    text(text),
    color(new sf::Color())
{
  setProperty("font", font);
}

TextField::~TextField()
{
  delete color;
}

void TextField::renderMe(sf::RenderTarget *renderer)
{
  sf::Transform transform = this->getTransform();
  sf::FloatRect bounds(0, 0, getWidth(), getHeight());
  FontBase *font = (FontBase*) getObject("font");
  font->render(renderer, &transform, color, bounds, text.c_str());
}

void TextField::setColor(sf::Color *color)
{
  delete this->color;
  this->color = color;
}

sf::Color * TextField::getColor()
{
  return color;
}

void TextField::setText(const char *text)
{
  this->text = std::string(text);
}

const char * TextField::getText()
{
  return text.c_str();
}

static mrb_value TextField_initialize(mrb_state *mrb, mrb_value self)
{
  mrb_value font;
  const char *text;
  size_t length;
  mrb_get_args(mrb, "os", &font, &text, &length);

  RubyEngine *engine = RubyEngine::getInstance();

  if (!mrb_obj_is_kind_of(mrb, font, engine->getClass("FontBase")))
    mrb_raise(mrb, E_TYPE_ERROR, "expected FontBase");

  SET_INSTANCE(self, new TextField(self, font, text));
  return self;
}

static mrb_value TextField_setColor(mrb_state *mrb, mrb_value self)
{
  mrb_value c;
  mrb_get_args(mrb, "A", &c);

  sf::Color *color = new sf::Color(
    (sf::Uint8) A_GET_INT(c, 0),
    (sf::Uint8) A_GET_INT(c, 1),
    (sf::Uint8) A_GET_INT(c, 2),
    (sf::Uint8) (A_SIZE(c) == 3) ? 255 : A_GET_INT(c, 3)
  );

  GET_INSTANCE(self, textField, TextField)
  textField->setColor(color);
  return self;
}

static mrb_value TextField_getColor(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, textField, TextField)
  sf::Color *c = textField->getColor();

  mrb_value color[4] = {
    mrb_fixnum_value(c->r),
    mrb_fixnum_value(c->g),
    mrb_fixnum_value(c->b),
    mrb_fixnum_value(c->a)
  };
  return mrb_ary_new_from_values(mrb, 4, color);
}

static mrb_value TextField_setText(mrb_state *mrb, mrb_value self)
{
  const char *text;
  size_t length;
  mrb_get_args(mrb, "s", &text, &length);

  GET_INSTANCE(self, textField, TextField)
  textField->setText(text);
  return self;
}

static mrb_value TextField_getText(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, textField, TextField)
  return mrb_str_new_cstr(mrb, textField->getText());
}

void RubyAction::bindTextField(mrb_state *mrb, RClass *module)
{
  struct RClass *super = mrb_class_get_under(mrb, module, "Sprite");
  struct RClass *clazz = mrb_define_class_under(mrb, module, "TextField", super);
  MRB_SET_INSTANCE_TT(clazz, MRB_TT_DATA);

  mrb_define_method(mrb, clazz, "initialize", TextField_initialize, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, clazz, "color=", TextField_setColor, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "color", TextField_getColor, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "text=", TextField_setText, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "text", TextField_getText, MRB_ARGS_NONE());
}
