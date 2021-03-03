#include "render/renderscenery.h"

int render_scenery_init(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api, struct Game* game, struct Scenery* scenery) {
  //scenery_init(&render_scenery->scenery, game->game_state);
  render_scenery->scenery = *scenery;
  render_scenery->scenery.texture_path = strdup(scenery->texture_path);

  render_scenery->scenery.entity.entity_data = render_scenery;
  render_scenery->scenery.entity.delete_func = (void (*)(void*, void*))render_scenery_delete;
  render_scenery->scenery.entity.update_func = (void (*)(void*, void*, float))render_scenery_update;
  render_scenery->scenery.entity.render_func = (void (*)(void*, void*))render_scenery_render;
  render_scenery->scenery.entity.recreate_func = (void (*)(void*, void*))render_scenery_recreate;

  sprite_init(&render_scenery->texture, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, render_scenery->scenery.texture_path));
  render_scenery->texture.scale = (vec3){.x = scenery->entity.scale, .y = scenery->entity.scale, .z = scenery->entity.scale};

  render_scenery->scenery.entity.width = render_scenery->texture.width * render_scenery->scenery.entity.scale;
  render_scenery->scenery.entity.height = render_scenery->texture.height * render_scenery->scenery.entity.scale;

  return 0;
}
void render_scenery_delete(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api) {
  scenery_delete(&render_scenery->scenery, NULL);

  sprite_delete(&render_scenery->texture, gpu_api);
}

void render_scenery_update(struct RenderScenery* render_scenery, struct Game* game, float delta_time) {
  scenery_update(&render_scenery->scenery, game->game_state, delta_time);

  render_scenery->texture.position = render_scenery->scenery.entity.position;
}

void render_scenery_render(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api) {
  render_scenery_update_uniforms(render_scenery, gpu_api);

  sprite_render(&render_scenery->texture, gpu_api);
}

void render_scenery_update_uniforms(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api) {
  sprite_update_uniforms(&render_scenery->texture, gpu_api);
}

void render_scenery_recreate(struct RenderScenery* render_scenery, struct GPUAPI* gpu_api) {
  sprite_recreate(&render_scenery->texture, gpu_api);
}