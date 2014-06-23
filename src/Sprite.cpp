#include "Sprite.hpp"
#include "util/array.hpp"
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace RubyAction;

Sprite::Sprite(mrb_value self)
  : EventDispatcher(self),
    x(0),
    y(0),
    width(0),
    height(0),
    scaleX(1),
    scaleY(1),
    anchorX(0),
    anchorY(0),
    rotation(0),
    visible(true)
{
  if (!mrb_nil_p(self))
  {
    setProperty("children", mrb_ary_new(mrb));
  }
}

int Sprite::getX()
{
  return x;
}

void Sprite::setX(int x)
{
  this->x = x;
}

int Sprite::getY()
{
  return y;
}

void Sprite::setY(int y)
{
  this->y = y;
}

int Sprite::getWidth()
{
  return width;
}

void Sprite::setWidth(int width)
{
  this->width = width;
}

int Sprite::getHeight()
{
  return height;
}

void Sprite::setHeight(int height)
{
  this->height = height;
}

float Sprite::getScaleX()
{
  return scaleX;
}

void Sprite::setScaleX(float scaleX)
{
  this->scaleX = scaleX;
}

float Sprite::getScaleY()
{
  return scaleY;
}

void Sprite::setScaleY(float scaleY)
{
  this->scaleY = scaleY;
}

float Sprite::getAnchorX()
{
  return anchorX;
}

void Sprite::setAnchorX(float anchorX)
{
  this->anchorX = anchorX;
}

float Sprite::getAnchorY()
{
  return anchorY;
}

void Sprite::setAnchorY(float anchorY)
{
  this->anchorY = anchorY;
}

float Sprite::getRotation()
{
  return rotation;
}

void Sprite::setRotation(float rotation)
{
  this->rotation = rotation;
}

bool Sprite::isVisible()
{
  return visible;
}

void Sprite::setVisible(bool visible)
{
  this->visible = visible;
}

Sprite* Sprite::getParent()
{
  return mrb_nil_p(getProperty("parent")) ? NULL : (Sprite*) getObject("parent");
}

void Sprite::setParent(Sprite *parent)
{
  setProperty("parent", parent ? parent->getSelf() : mrb_nil_value());
}

void Sprite::render(SDL_Renderer *renderer)
{
  if (!isVisible()) return;

  glPushMatrix();
  glm::mat4 matrix = glm::mat4(1.0);

  glm::mat4 anchor = glm::translate(matrix, glm::vec3(-width * scaleX * anchorX, -height * scaleY * anchorY, 0.0f));
  glm::mat4 rotate = glm::rotate(matrix, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 translate = glm::translate(matrix, glm::vec3(x, y, 0));
  glm::mat4 scale = glm::scale(matrix, glm::vec3(scaleX, scaleY, 0));

  glm::mat4 model = translate * (rotate * anchor) * scale;
  glMultMatrixf(glm::value_ptr(model));

  this->renderMe(renderer);

  mrb_value children = getProperty("children");
  for (int i = 0; i < RARRAY_LEN(children); i++)
  {
    mrb_value child = mrb_ary_ref(mrb, children, i);
    GET_INSTANCE(child, sprite, Sprite)
    sprite->render(renderer);
  }

  glPopMatrix();
}

void Sprite::addChild(mrb_value child)
{
  if (!contains(child))
  {
    mrb_ary_push(mrb, getProperty("children"), child);
    GET_INSTANCE(child, childSprite, Sprite)
    childSprite->removeFromParent();
    childSprite->setParent(this);
  }
}

void Sprite::removeChild(mrb_value child)
{
  if (contains(child))
  {
    mrb_value old = getProperty("children");
    mrb_value children = mrb_ary_new(mrb);

    for (int i = 0; i < RARRAY_LEN(old); i++)
    {
      mrb_value current = mrb_ary_ref(mrb, old, i);
      if (!mrb_obj_equal(mrb, child, current)) mrb_ary_push(mrb, children, current);
    }

    setProperty("children", children);
    GET_INSTANCE(child, childSprite, Sprite)
    childSprite->setParent(NULL);
  }
}

void Sprite::removeFromParent()
{
  Sprite *parent = this->getParent();
  if (parent) parent->removeChild(this->getSelf());
}

bool Sprite::contains(mrb_value child)
{
  mrb_value children = getProperty("children");
  for (int i = 0; i < RARRAY_LEN(children); i++)
  {
    mrb_value current = mrb_ary_ref(mrb, children, i);
    if (mrb_obj_equal(mrb, child, current)) return true;
  }
  return false;
}

SDL_Point Sprite::globalToLocal(SDL_Point global)
{
  Sprite *parent = this->getParent();
  if (parent) global = parent->globalToLocal(global);

  glm::mat4 matrix = glm::mat4(1.0);
  glm::mat4 anchor = glm::translate(matrix, glm::vec3(-width * anchorX, -height * anchorY, 0.0f));
  glm::mat4 rotate = glm::rotate(matrix, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 translate = glm::translate(matrix, glm::vec3(x, y, 0));
  glm::mat4 model = translate * (rotate * anchor);

  glm::vec4 point = model * glm::vec4(global.x, global.y, 1, 1);
  glm::vec4 position = model * glm::vec4(x, y, 1, 1);

  return {
    (int) ((point.x - position.x + width * anchorX * scaleX) / scaleX),
    (int) ((point.y - position.y + height * anchorY * scaleY) / scaleY)
  };
}

bool Sprite::collide(SDL_Point point)
{
  point = globalToLocal(point);
  return (point.x >= 0) && (point.x <= width) && (point.y >= 0) && (point.y <= height);
}

void Sprite::dispatch(mrb_sym name, mrb_value* argv, int argc)
{
  mrb_value children = getProperty("children");

  for (int i = 0; i < RARRAY_LEN(children); i++)
  {
    A_GET_TYPE(children, i, child, Sprite)
    child->dispatch(name, argv, argc);
  }

  EventDispatcher::dispatch(name, argv, argc);
}

static mrb_value Sprite_initialize(mrb_state *mrb, mrb_value self)
{
  SET_INSTANCE(self, new Sprite(self));
  return self;
}

static mrb_value Sprite_getX(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_fixnum_value(sprite->getX());
}

static mrb_value Sprite_setX(mrb_state *mrb, mrb_value self)
{
  mrb_int x;
  mrb_get_args(mrb, "i", &x);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setX(x);
  return self;
}

static mrb_value Sprite_getY(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_fixnum_value(sprite->getY());
}

static mrb_value Sprite_setY(mrb_state *mrb, mrb_value self)
{
  mrb_int y;
  mrb_get_args(mrb, "i", &y);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setY(y);
  return self;
}

static mrb_value Sprite_getPosition(mrb_state *mrb, mrb_value self)
{
  mrb_value position[2] = { Sprite_getX(mrb, self), Sprite_getY(mrb, self) };
  return mrb_ary_new_from_values(mrb, 2, position);
}

static mrb_value Sprite_setPosition(mrb_state *mrb, mrb_value self)
{
  mrb_value position;
  mrb_get_args(mrb, "A", &position);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setX(A_GET_INT(position, 0));
  sprite->setY(A_GET_INT(position, 1));

  return self;
}

static mrb_value Sprite_getWidth(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_fixnum_value(sprite->getWidth());
}

static mrb_value Sprite_setWidth(mrb_state *mrb, mrb_value self)
{
  mrb_int width;
  mrb_get_args(mrb, "i", &width);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setWidth(width);
  return self;
}

static mrb_value Sprite_getHeight(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_fixnum_value(sprite->getHeight());
}

static mrb_value Sprite_setHeight(mrb_state *mrb, mrb_value self)
{
  mrb_int height;
  mrb_get_args(mrb, "i", &height);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setHeight(height);
  return self;
}

static mrb_value Sprite_getSize(mrb_state *mrb, mrb_value self)
{
  mrb_value size[2] = { Sprite_getWidth(mrb, self), Sprite_getHeight(mrb, self) };
  return mrb_ary_new_from_values(mrb, 2, size);
}

static mrb_value Sprite_setSize(mrb_state *mrb, mrb_value self)
{
  mrb_value size;
  mrb_get_args(mrb, "A", &size);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setWidth(A_GET_INT(size, 0));
  sprite->setHeight(A_GET_INT(size, 1));

  return self;
}

static mrb_value Sprite_getScaleX(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_float_value(mrb, sprite->getScaleX());
}

static mrb_value Sprite_setScaleX(mrb_state *mrb, mrb_value self)
{
  mrb_float scaleX;
  mrb_get_args(mrb, "f", &scaleX);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setScaleX(scaleX);
  return self;
}

static mrb_value Sprite_getScaleY(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_float_value(mrb, sprite->getScaleY());
}

static mrb_value Sprite_setScaleY(mrb_state *mrb, mrb_value self)
{
  mrb_float scaleY;
  mrb_get_args(mrb, "f", &scaleY);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setScaleY(scaleY);
  return self;
}

static mrb_value Sprite_getScale(mrb_state *mrb, mrb_value self)
{
  mrb_value scale[2] = { Sprite_getScaleX(mrb, self), Sprite_getScaleY(mrb, self) };
  return mrb_ary_new_from_values(mrb, 2, scale);
}

static mrb_value Sprite_setScale(mrb_state *mrb, mrb_value self)
{
  mrb_value scale;
  mrb_get_args(mrb, "A", &scale);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setScaleX(A_GET_FLOAT(scale, 0));
  sprite->setScaleY(A_GET_FLOAT(scale, 1));

  return self;
}

static mrb_value Sprite_getAnchorX(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_float_value(mrb, sprite->getAnchorX());
}

static mrb_value Sprite_setAnchorX(mrb_state *mrb, mrb_value self)
{
  mrb_float anchorX;
  mrb_get_args(mrb, "f", &anchorX);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setAnchorX(anchorX);
  return self;
}

static mrb_value Sprite_getAnchorY(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_float_value(mrb, sprite->getAnchorY());
}

static mrb_value Sprite_setAnchorY(mrb_state *mrb, mrb_value self)
{
  mrb_float anchorY;
  mrb_get_args(mrb, "f", &anchorY);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setAnchorY(anchorY);
  return self;
}

static mrb_value Sprite_getAnchor(mrb_state *mrb, mrb_value self)
{
  mrb_value anchor[2] = { Sprite_getAnchorX(mrb, self), Sprite_getAnchorY(mrb, self) };
  return mrb_ary_new_from_values(mrb, 2, anchor);
}

static mrb_value Sprite_setAnchor(mrb_state *mrb, mrb_value self)
{
  mrb_value anchor;
  mrb_get_args(mrb, "A", &anchor);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setAnchorX(A_GET_FLOAT(anchor, 0));
  sprite->setAnchorY(A_GET_FLOAT(anchor, 1));

  return self;
}

static mrb_value Sprite_getRotation(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_float_value(mrb, sprite->getRotation());
}

static mrb_value Sprite_setRotation(mrb_state *mrb, mrb_value self)
{
  mrb_float rotation;
  mrb_get_args(mrb, "f", &rotation);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setRotation(rotation);
  return self;
}

static mrb_value Sprite_isVisible(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  return mrb_bool_value(sprite->isVisible());
}

static mrb_value Sprite_setVisible(mrb_state *mrb, mrb_value self)
{
  mrb_bool visible;
  mrb_get_args(mrb, "b", &visible);

  GET_INSTANCE(self, sprite, Sprite)
  sprite->setVisible(visible);
  return self;
}

static mrb_value Sprite_getParent(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  Sprite *parent = sprite->getParent();
  return parent ? parent->getSelf() : mrb_nil_value();
}

static mrb_value Sprite_addChild(mrb_state *mrb, mrb_value self)
{
  mrb_value child;
  mrb_get_args(mrb, "o", &child);

  struct RClass *module = mrb_class_get(mrb, "RubyAction");
  struct RClass *clazz = mrb_class_get_under(mrb, module, "Sprite");
  if (!mrb_obj_is_kind_of(mrb, child, clazz))
  {
    mrb_raise(mrb, E_TYPE_ERROR, "expected Sprite");
  }

  GET_INSTANCE(self, sprite, Sprite)
  sprite->addChild(child);
  return self;
}

static mrb_value Sprite_removeChild(mrb_state *mrb, mrb_value self)
{
  mrb_value child;
  mrb_get_args(mrb, "o", &child);

  struct RClass *module = mrb_class_get(mrb, "RubyAction");
  struct RClass *clazz = mrb_class_get_under(mrb, module, "Sprite");
  if (!mrb_obj_is_kind_of(mrb, child, clazz))
  {
    mrb_raise(mrb, E_TYPE_ERROR, "expected Sprite");
  }

  GET_INSTANCE(self, sprite, Sprite)
  sprite->removeChild(child);
  return self;
}

static mrb_value Sprite_removeFromParent(mrb_state *mrb, mrb_value self)
{
  GET_INSTANCE(self, sprite, Sprite)
  sprite->removeFromParent();
  return self;
}

static mrb_value Sprite_contains(mrb_state *mrb, mrb_value self)
{
  mrb_value child;
  mrb_get_args(mrb, "o", &child);

  struct RClass *module = mrb_class_get(mrb, "RubyAction");
  struct RClass *clazz = mrb_class_get_under(mrb, module, "Sprite");
  if (!mrb_obj_is_kind_of(mrb, child, clazz))
  {
    mrb_raise(mrb, E_TYPE_ERROR, "expected Sprite");
  }

  GET_INSTANCE(self, sprite, Sprite)
  return mrb_bool_value(sprite->contains(child));
}

static mrb_value Sprite_globalToLocal(mrb_state *mrb, mrb_value self)
{
  mrb_int x;
  mrb_int y;
  mrb_get_args(mrb, "ii", &x, &y);

  GET_INSTANCE(self, sprite, Sprite)
  SDL_Point p = sprite->globalToLocal({ x, y });

  mrb_value point[2] = { mrb_fixnum_value(p.x), mrb_fixnum_value(p.y) };
  return mrb_ary_new_from_values(mrb, 2, point);
}

static mrb_value Sprite_collide(mrb_state *mrb, mrb_value self)
{
  mrb_int x;
  mrb_int y;
  mrb_get_args(mrb, "ii", &x, &y);

  GET_INSTANCE(self, sprite, Sprite)
  return mrb_bool_value(sprite->collide({ x, y }));
}

void RubyAction::bindSprite(mrb_state *mrb, RClass *module)
{
  struct RClass *super = mrb_class_get_under(mrb, module, "EventDispatcher");
  struct RClass *clazz = mrb_define_class_under(mrb, module, "Sprite", super);
  MRB_SET_INSTANCE_TT(clazz, MRB_TT_DATA);

  mrb_define_method(mrb, clazz, "initialize", Sprite_initialize, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "x", Sprite_getX, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "x=", Sprite_setX, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "y", Sprite_getY, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "y=", Sprite_setY, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "position", Sprite_getPosition, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "position=", Sprite_setPosition, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "width", Sprite_getWidth, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "width=", Sprite_setWidth, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "height", Sprite_getHeight, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "height=", Sprite_setHeight, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "size", Sprite_getSize, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "size=", Sprite_setSize, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "scale_x", Sprite_getScaleX, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "scale_x=", Sprite_setScaleX, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "scale_y", Sprite_getScaleY, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "scale_y=", Sprite_setScaleY, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "scale", Sprite_getScale, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "scale=", Sprite_setScale, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "anchor_x", Sprite_getAnchorX, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "anchor_x=", Sprite_setAnchorX, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "anchor_y", Sprite_getAnchorY, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "anchor_y=", Sprite_setAnchorY, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "anchor", Sprite_getAnchor, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "anchor=", Sprite_setAnchor, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "rotation", Sprite_getRotation, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "rotation=", Sprite_setRotation, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "visible?", Sprite_isVisible, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "visible=", Sprite_setVisible, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "parent", Sprite_getParent, MRB_ARGS_NONE());
  mrb_define_method(mrb, clazz, "add_child", Sprite_addChild, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "remove_child", Sprite_removeChild, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "remove_from_parent", Sprite_removeFromParent, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "contains?", Sprite_contains, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "global_to_local", Sprite_globalToLocal, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, clazz, "collide?", Sprite_collide, MRB_ARGS_REQ(2));

  // alias
  mrb_alias_method(mrb, clazz, mrb_intern(mrb, "<<"), mrb_intern(mrb, "add_child"));
  mrb_alias_method(mrb, clazz, mrb_intern(mrb, ">>"), mrb_intern(mrb, "remove_child"));
}
