#pragma once
#ifndef RENDER_PLAYER_H
#define RENDER_PLAYER_H

#include <mana/core/memoryallocator.h>
//
#include <entities/player.h>
#include <mana/graphics/shaders/spriteanimationshader.h>
#include <mana/graphics/utilities/spriteanimation.h>
#include <mana/mana.h>

#include "game.h"

struct Game;

struct RenderPlayer {
  struct Player player;

  struct Sprite character_shadow;
  struct SpriteAnimation standing_animation;
  struct SpriteAnimation walking_animation;
};

int render_player_init(struct RenderPlayer* render_player, struct GPUAPI* gpu_api, struct Game* game);
void render_player_delete(struct RenderPlayer* render_player, struct GPUAPI* gpu_api);
void render_player_update(struct RenderPlayer* player, struct Game* game, float delta_time);
void render_player_render(struct RenderPlayer* render_player, struct GPUAPI* gpu_api);
void render_player_update_uniforms(struct RenderPlayer* render_player, struct GPUAPI* gpu_api);
void render_player_recreate(struct RenderPlayer* render_player, struct GPUAPI* gpu_api);

#endif  // PLAYER_H
