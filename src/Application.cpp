#include "Application.hpp"
#include "RubyEngine.hpp"
#include "EventDispatcher.hpp"
#include "TextureBase.hpp"
#include "Texture.hpp"
#include "Sprite.hpp"
#include "Bitmap.hpp"
#include "TextureRegion.hpp"
#include <iostream>

namespace RubyAction
{

  static const int RUN_GAME_LOOP = 1;

  Uint32 timerUpdate(Uint32 interval, void *param)
  {
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = RUN_GAME_LOOP;
    SDL_PushEvent(&event);
    return interval;
  }

  Application *Application::instance = new Application();

  Application* Application::getInstance()
  {
    return instance;
  }

  int Application::run(const char *filename)
  {
    RubyAction::RubyEngine *engine = RubyAction::RubyEngine::getInstance();
    engine->bind(RubyAction::bindEventDispatcher);
    engine->bind(RubyAction::bindTextureBase);
    engine->bind(RubyAction::bindTexture);
    engine->bind(RubyAction::bindSprite);
    engine->bind(RubyAction::bindBitmap);
    engine->bind(RubyAction::bindTextureRegion);
    engine->load(filename);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
      std::cout << SDL_GetError() << std::endl;
      return -1;
    }

    int x = SDL_WINDOWPOS_CENTERED, y = SDL_WINDOWPOS_CENTERED, width = config.width, height = config.height;

    // window = SDL_CreateWindow(config.title, x, y, width, height, SDL_WINDOW_SHOWN);
    // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_AddTimer(1000 / 60, timerUpdate, NULL);

    bool running = true;
    SDL_Event event;

    while (running && SDL_WaitEvent(&event))
    {
      switch (event.type)
      {
        case SDL_USEREVENT:
          if (event.user.code == RUN_GAME_LOOP)
          {
            // SDL_RenderClear(renderer);
            // texture->draw(renderer, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
            // SDL_RenderPresent(renderer);
            engine->garbageCollect();
          }
          break;
        case SDL_QUIT:
          running = false;
          break;
      }
    }

    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);

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

}
