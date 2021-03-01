#pragma once
#ifndef RENDER_ME_H
#define RENDER_ME_H

#include <mana/core/memoryallocator.h>
//
#include <entities/me.h>
#include <mana/graphics/shaders/spriteanimationshader.h>
#include <mana/graphics/utilities/spriteanimation.h>
#include <mana/mana.h>

#include "game.h"

struct Game;

struct RenderMe {
  struct Me me;

  // TODO: Store in map?
  struct Sprite shadow;
  struct SpriteAnimation standing_animation;
  struct SpriteAnimation walking_animation;
};

int render_me_init(struct RenderMe* render_me, struct GPUAPI* gpu_api, struct Game* game);
void render_me_delete(struct RenderMe* render_me, struct GPUAPI* gpu_api);
void render_me_update(struct RenderMe* render_me, struct Game* game, float delta_time);
void render_me_render(struct RenderMe* render_me, struct GPUAPI* gpu_api);
void render_me_update_uniforms(struct RenderMe* render_me, struct GPUAPI* gpu_api);
void render_me_recreate(struct RenderMe* render_me, struct GPUAPI* gpu_api);

#endif  // RENDER_ME_H
