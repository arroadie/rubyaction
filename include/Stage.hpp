#ifndef __STAGE__
#define __STAGE__

#include "Sprite.hpp"

namespace RubyAction
{

  class Stage : public Sprite
  {
  private:
    static Stage *instance;
    Stage() : Sprite(mrb_nil_value()) {}
  public:
  	static Stage* getInstance();
  };

  void bindStage(mrb_state*, RClass*);

}

#endif // __STAGE__
