#pragma once
#ifndef RENDER_WILBUR_H
#define RENDER_WILBUR_H

#include <mana/core/memoryallocator.h>
//
#include <entities/wilbur.h>
#include <mana/graphics/shaders/spriteanimationshader.h>
#include <mana/graphics/utilities/spriteanimation.h>
#include <mana/mana.h>

#include "game.h"

struct Game;

struct RenderWilbur {
  struct Wilbur wilbur;

  // TODO: Store in map?
  struct Sprite shadow;
  struct SpriteAnimation standing_animation;
  struct SpriteAnimation walking_animation;
};

int render_wilbur_init(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api, struct Game* game);
void render_wilbur_delete(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api);
void render_wilbur_update(struct RenderWilbur* render_wilbur, struct Game* game, float delta_time);
void render_wilbur_render(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api);
void render_wilbur_update_uniforms(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api);
void render_wilbur_recreate(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api);

#endif  // RENDER_WILBUR_H
