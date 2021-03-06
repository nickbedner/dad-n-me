/*#pragma once
#ifndef RENDER_SCENERY_H
#define RENDER_SCENERY_H

#include <mana/core/memoryallocator.h>
//
#include <entities/scenery.h>
#include <mana/graphics/shaders/spriteanimationshader.h>
#include <mana/graphics/utilities/spriteanimation.h>
#include <mana/mana.h>

#include "game.h"

struct Game;

struct RenderScenery {
  struct Scenery scenery;

  struct Sprite texture;
};

int render_scenery_init(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api, struct Game* game, struct Scenery* scenery);
void render_scenery_delete(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api);
void render_scenery_update(struct RenderScenery* render_scenery, struct Game* game, float delta_time);
void render_scenery_render(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api);
void render_scenery_update_uniforms(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api);
void render_scenery_recreate(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api);

#endif  // RENDER_SCENERY_H
*/