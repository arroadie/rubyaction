#include "Stage.hpp"
#include "RubyEngine.hpp"
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/hash.h>

using namespace RubyAction;

Stage *Stage::instance = new Stage();

Stage* Stage::getInstance()
{
  return instance;
}

void RubyAction::bindStage(mrb_state *mrb, RClass *module)
{
  mrb_value stage = RubyEngine::getInstance()->newInstance("Sprite", 0, NULL);
  mrb_define_const(mrb, module, "Stage", stage);

  Stage::getInstance()->setSelf(stage);
  SET_INSTANCE(stage, Stage::getInstance());
}
