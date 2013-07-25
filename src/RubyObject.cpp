#include "RubyObject.hpp"
#include <iostream>

namespace RubyAction
{

  struct mrb_data_type mrb_ruby_object_binding = { "RubyObject", mrb_ruby_object_free };

  RubyObject::RubyObject(mrb_value self)
    : self(self)
  {
    std::cout << "Created: " << this->inspect() << std::endl;
  }

  RubyObject::~RubyObject()
  {
    std::cout << "Destroyed: " << this->inspect() << std::endl;
  }

  mrb_value RubyObject::getProperty(const char *property)
  {
    mrb_state *mrb = RubyEngine::getInstance()->getState();
    return mrb_iv_get(mrb, self, mrb_intern(mrb, property));
  }

  void RubyObject::setProperty(const char *property, mrb_value value)
  {
    mrb_state *mrb = RubyEngine::getInstance()->getState();
    mrb_iv_set(mrb, self, mrb_intern(mrb, property), value);
  }

  RubyObject* RubyObject::getObject(const char *property)
  {
    mrb_state *mrb = RubyEngine::getInstance()->getState();
    GET_INSTANCE(getProperty(property), object, RubyObject)
    return object;
  }

  const char * RubyObject::inspect()
  {
    mrb_state *mrb = RubyEngine::getInstance()->getState();
    return mrb_string_value_ptr(mrb, mrb_inspect(mrb, self));
  }

}
