#ifndef __TEXTURE_REGION__
#define __TEXTURE_REGION__

#include "TextureBase.hpp"

namespace RubyAction
{

  class TextureRegion : public RubyObject
  {
  protected:
    int x;
    int y;
    int width;
    int height;
  public:
    TextureRegion(mrb_value, int, int, int, int);
    int getX();
    int getY();
    int getWidth();
    int getHeight();
    void setRegion(int, int, int, int);
    TextureBase* getTextureBase();
  };

  void bindTextureRegion(mrb_state*, RClass*);

}

#endif // __TEXTURE_REGION__
