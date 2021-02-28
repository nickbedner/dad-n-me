#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include <mana/core/memoryallocator.h>
//
#include <mana/graphics/shaders/spriteanimationshader.h>
#include <mana/graphics/utilities/spriteanimation.h>
#include <mana/mana.h>

#include "entities/entity.h"
#include "game.h"

enum CharacterState {
  CHARACTER_IDLE_STATE = 0,
  CHARACTER_WALKING_STATE
};

struct Player {
  struct Entity entity;

  float character_direction;
  enum CharacterState character_state;
  vec3 character_position;
  struct Sprite character_shadow;
  struct SpriteAnimation standing_animation;
  struct SpriteAnimation walking_animation;
};

struct Game;

int player_init(struct Player* player, struct GPUAPI* gpu_api, struct Game* game);
void player_delete(struct Player* player, struct GPUAPI* gpu_api);
void player_update(struct Player* player, struct Game* game, float delta_time);
void player_render(struct Player* player, struct GPUAPI* gpu_api);
void player_update_uniforms(struct Player* player, struct GPUAPI* gpu_api);
void player_recreate(struct Player* player, struct GPUAPI* gpu_api);

#endif  // PLAYER_H
