#include "entities/sandcastle.h"

int sandcastle_init(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api, struct Game* game) {
  sandcastle->entity.entity_data = sandcastle;
  sandcastle->entity.delete_func = (void (*)(void*, struct GPUAPI*))sandcastle_delete;
  sandcastle->entity.update_func = (void (*)(void*, void*, float))sandcastle_update;
  sandcastle->entity.render_func = (void (*)(void*, struct GPUAPI*))sandcastle_render;

  sandcastle->sandcastle_state = SANDCASTLE_FULL_STATE;

  float draw_scale = 0.2;
  float sand_castle_size = draw_scale * 0.85f;
  // Sandcastle
  sandcastle->entity.position = (vec3){.x = -3.333f, .y = -1.4f, .z = -0.001f};
  sandcastle->sandcastle_sprite = calloc(1, sizeof(struct Sprite));
  sprite_init(sandcastle->sandcastle_sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/sandcastle.png"));
  sandcastle->sandcastle_sprite->position = sandcastle->entity.position;
  sandcastle->sandcastle_sprite->scale = (vec3){.x = sand_castle_size, .y = sand_castle_size, .z = sand_castle_size};

  // Sandcastle animation
  sandcastle->sandcastle_animation = calloc(1, sizeof(struct SpriteAnimation));
  sprite_animation_init(sandcastle->sandcastle_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/sandcastlespritesheet.png"), 13, 1.0f / 30.0f, 0);
  sandcastle->sandcastle_animation->position = sandcastle->entity.position;
  sandcastle->sandcastle_animation->scale = (vec3){.x = sand_castle_size, .y = sand_castle_size, .z = sand_castle_size};

  audio_clip_init(&sandcastle->fart_clip, game->resource_manager.fart_clip_cache, SOUND_AUDIO_CLIP, 0, 1.0f, 0.0f);

  return 0;
}

void sandcastle_delete(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api) {
  sprite_delete(sandcastle->sandcastle_sprite, gpu_api);
  sprite_animation_delete(sandcastle->sandcastle_animation, gpu_api);
  free(sandcastle->sandcastle_sprite);
  free(sandcastle->sandcastle_animation);
}

void sandcastle_update(struct Sandcastle* sandcastle, struct Game* game, float delta_time) {
  switch (sandcastle->sandcastle_state) {
    case (SANDCASTLE_FULL_STATE):
      if (fabs((sandcastle->entity.position.x - game->player->entity.position.x + 0.25f) / 2.0f) + fabs(sandcastle->entity.position.y - game->player->entity.position.y + 0.5f) < 0.35f) {
        sandcastle->sandcastle_state = SANDCASTLE_ANIMATING_STATE;
        sandcastle->sandcastle_animation->loop = 0;
        audio_manager_play_audio_clip(&game->resource_manager.audio_manager, &sandcastle->fart_clip);
      }
      break;
    case (SANDCASTLE_ANIMATING_STATE):
      sprite_animation_update(sandcastle->sandcastle_animation, delta_time);
      if (sandcastle->sandcastle_animation->current_frame >= sandcastle->sandcastle_animation->total_frames)
        sandcastle->sandcastle_state = SANDCASTLE_CRUSHED_STATE;
    case (SANDCASTLE_CRUSHED_STATE):
      break;
  }
}

void sandcastle_render(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api) {
  sandcastle_update_uniforms(sandcastle, gpu_api);

  switch (sandcastle->sandcastle_state) {
    case (SANDCASTLE_FULL_STATE):
      sprite_render(sandcastle->sandcastle_sprite, gpu_api);
      break;
    case (SANDCASTLE_ANIMATING_STATE):
      sprite_animation_render(sandcastle->sandcastle_animation, gpu_api);
    case (SANDCASTLE_CRUSHED_STATE):
      break;
  }
}

void sandcastle_update_uniforms(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api) {
  sprite_update_uniforms(sandcastle->sandcastle_sprite, gpu_api);
  sprite_animation_update_uniforms(sandcastle->sandcastle_animation, gpu_api);
}

void sandcastle_recreate(struct Sandcastle* sandcastle, struct GPUAPI* gpu_api) {
  sprite_recreate(sandcastle->sandcastle_sprite, gpu_api);
  sprite_animation_recreate(sandcastle->sandcastle_animation, gpu_api);
}
