#pragma once
#ifndef GAME_H
#define GAME_H

#include <mana/core/memoryallocator.h>
//
#include <chaos/chaos.h>
#include <mana/audio/audiomanager.h>
#include <mana/core/inputmanager.h>
#include <mana/graphics/entities/manifoldplanet.h>
#include <mana/graphics/entities/sprite.h>
#include <mana/graphics/shaders/atmosphericscattering.h>
#include <mana/graphics/shaders/fxaashader.h>
#include <mana/graphics/shaders/manifolddualcontouringshader.h>
#include <mana/graphics/shaders/modelshader.h>
#include <mana/graphics/shaders/modelstaticshader.h>
#include <mana/graphics/shaders/spriteshader.h>
#include <mana/graphics/utilities/camera.h>
#include <mana/graphics/utilities/modelcache.h>
#include <mana/graphics/utilities/texturecache.h>
#include <mana/mana.h>

struct Game {
  struct Window* window;
  struct Camera camera;

  struct AudioManager* audio_manager;
  struct AudioClipCache* music_clip_cache;
  struct AudioClipCache* fart_clip_cache;
  struct AudioClip* music_clip;
  struct AudioClip* fart_clip;

  struct TextureCache texture_cache;
  struct SpriteShader sprite_shader;
  struct FXAAShader fxaa_shader;
  int fxaa_on;

  struct Sprite* hud_sprite;

  struct ArrayList sprites;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
void game_update_camera(struct Game* game, struct Engine* engine);
void game_update_input(struct Game* game, struct Engine* engine);
void game_update_uniform_buffers(struct Game* game, struct Engine* engine);

#endif  //GAME_H
