#pragma once
#ifndef SANDCASTLE_H
#define SANDCASTLE_H

#include <mana/core/memoryallocator.h>
//
#include <mana/audio/audiomanager.h>
#include <mana/graphics/shaders/spriteanimationshader.h>
#include <mana/graphics/utilities/spriteanimation.h>
#include <mana/mana.h>

#include "entities/entity.h"
#include "game.h"

enum SandcastleState {
  SANDCASTLE_FULL_STATE = 0,
  SANDCASTLE_ANIMATING_STATE,
  SANDCASTLE_CRUSHED_STATE
};

struct Sandcastle {
  struct Entity entity;

  enum SandcastleState sandcastle_state;
  struct Sprite* sandcastle_sprite;
  struct SpriteAnimation* sandcastle_animation;

  struct AudioClip fart_clip;
};

struct Game;

int sandcastle_init(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api, struct Game* game);
void sandcastle_delete(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api);
void sandcastle_update(struct Sandcastle* sandcastle, struct Game* game, float delta_time);
void sandcastle_render(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api);
void sandcastle_update_uniforms(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api);
void sandcastle_recreate(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api);

#endif  // SANDCASTLE_H
