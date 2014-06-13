#include "Application.hpp"
#include "RubyEngine.hpp"
#include "EventDispatcher.hpp"
#include "TextureBase.hpp"
#include "Texture.hpp"
#include "Sprite.hpp"
#include "Bitmap.hpp"
#include "TextureRegion.hpp"
#include "Stage.hpp"
#include "FontBase.hpp"
#include "Font.hpp"
#include "TTFont.hpp"
#include "TextField.hpp"
#include "physics/Physics.hpp"

#include <sstream>
#include <mruby.h>
#include <mruby/value.h>
#include <mruby/hash.h>
#include <SDL_ttf.h>

using namespace RubyAction;

void mouseMoveEvent(SDL_Event *event)
{
  mrb_state *mrb = RubyEngine::getInstance()->getState();
  mrb_value data[] = {
    mrb_fixnum_value(event->motion.x),
    mrb_fixnum_value(event->motion.y)
  };
  Stage::getInstance()->dispatch(mrb_intern(mrb, "mouse_move"), data, 2);
}

void mouseButtonEvent(SDL_Event *event, const char *name)
{
  mrb_state *mrb = RubyEngine::getInstance()->getState();
  mrb_value data[] = {
    mrb_fixnum_value(event->button.button),
    mrb_fixnum_value(event->button.x),
    mrb_fixnum_value(event->button.y)
  };
  Stage::getInstance()->dispatch(mrb_intern(mrb, name), data, 3);
}

void keyEvent(SDL_Event *event, const char *name)
{
  mrb_state *mrb = RubyEngine::getInstance()->getState();
  mrb_value data = mrb_str_new_cstr(mrb, SDL_GetKeyName(event->key.keysym.sym));
  Stage::getInstance()->dispatch(mrb_intern(mrb, name), &data, 1);
}

void processInputEvents(SDL_Event *event)
{
  switch (event->type)
  {
    case SDL_MOUSEMOTION:
      mouseMoveEvent(event);
      break;
    case SDL_MOUSEBUTTONDOWN:
      mouseButtonEvent(event, "mouse_down");
      break;
    case SDL_MOUSEBUTTONUP:
      mouseButtonEvent(event, "mouse_up");
      break;
    case SDL_KEYDOWN:
      keyEvent(event, "key_down");
      break;
    case SDL_KEYUP:
      keyEvent(event, "key_up");
      break;
  }
}

Application *Application::instance = new Application();

Application* Application::getInstance()
{
  return instance;
}

int Application::run(const char *filename)
{
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    LOG(SDL_GetError());
    return -1;
  }

  if (TTF_Init() < 0)
  {
    LOG(SDL_GetError());
    return -1;
  }

  int x = SDL_WINDOWPOS_CENTERED, y = SDL_WINDOWPOS_CENTERED, width = config.width, height = config.height;
  window = SDL_CreateWindow(config.title, x, y, width, height, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  RubyAction::RubyEngine *engine = RubyAction::RubyEngine::getInstance();
  engine->bind(RubyAction::bindEventDispatcher);
  engine->bind(RubyAction::bindTextureBase);
  engine->bind(RubyAction::bindTexture);
  engine->bind(RubyAction::bindSprite);
  engine->bind(RubyAction::bindBitmap);
  engine->bind(RubyAction::bindTextureRegion);
  engine->bind(RubyAction::bindStage);
  engine->bind(RubyAction::bindFontBase);
  engine->bind(RubyAction::bindFont);
  engine->bind(RubyAction::bindTTFont);
  engine->bind(RubyAction::bindTextField);
  engine->bind(RubyAction::Physics::bind);

  if (!engine->load(filename)) return -1;

  bool running = true;
  SDL_Event event;

  Uint32 before = SDL_GetTicks();

  while (running)
  {
    int arena = mrb_gc_arena_save(engine->getState());

    while(SDL_PollEvent(&event)) {
      switch (event.type)
      {
        case SDL_QUIT:
          running = false;
          break;
        default:
          processInputEvents(&event);
      }
    }

    Uint32 now = SDL_GetTicks();
    float dt = (now - before) / 1000.0;
    before = now;

    mrb_value delta = mrb_float_value(engine->getState(), dt);
    Stage::getInstance()->dispatch(mrb_intern(engine->getState(), "enter_frame"), &delta, 1);

    SDL_RenderClear(renderer);
    Stage::getInstance()->render(renderer);
    SDL_RenderPresent(renderer);

    mrb_gc_arena_restore(engine->getState(), arena);

    engine->garbageCollect();
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();

  return 0;
}

SDL_Window* Application::getWindow()
{
  return window;
}

SDL_Renderer* Application::getRenderer()
{
  return renderer;
}
