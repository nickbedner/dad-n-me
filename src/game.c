#include "game.h"

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  game->window = window;

  fxaa_shader_init(&game->fxaa_shader, gpu_api);
  sprite_shader_init(&game->sprite_shader, gpu_api);
  sprite_animation_shader_init(&game->sprite_animation_shader, gpu_api);

  audio_manager_init(&game->audio_manager);

  game->music_clip_cache = calloc(1, sizeof(struct AudioClipCache));
  game->fart_clip_cache = calloc(1, sizeof(struct AudioClipCache));
  audio_clip_cache_init(game->music_clip_cache, "assets/audio/music/Dad_n_Me.wav");
  audio_clip_cache_init(game->fart_clip_cache, "assets/audio/sounds/fart_2.wav");

  game->music_clip = calloc(1, sizeof(struct AudioClip));
  audio_clip_init(game->music_clip, game->music_clip_cache, MUSIC_AUDIO_CLIP, 1, 0.75f, 0.0125f);

  audio_manager_play_audio_clip(&game->audio_manager, game->music_clip);

  struct TextureSettings texture1 = {"./assets/textures/fence.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture2 = {"./assets/textures/grass.png", FILTER_NEAREST, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture3 = {"./assets/textures/clouds.png", FILTER_NEAREST, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture4 = {"./assets/textures/back2.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture5 = {"./assets/textures/trash.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture6 = {"./assets/textures/sign.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture7 = {"./assets/textures/woodfence.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture8 = {"./assets/textures/grassshadow.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture9 = {"./assets/textures/grassdiffer.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture10 = {"./assets/textures/trash2.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture11 = {"./assets/textures/sign2.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture12 = {"./assets/textures/hud.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture13 = {"./assets/textures/sandbox.png", FILTER_NEAREST, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture14 = {"./assets/textures/walkingspritesheet.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture15 = {"./assets/textures/standingspritesheet.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture16 = {"./assets/textures/sandcastle.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture17 = {"./assets/textures/sandcastlespritesheet.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture18 = {"./assets/textures/shadow.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture19 = {"./assets/textures/forewoodfence.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture20 = {"./assets/textures/street.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture21 = {"./assets/textures/streetend.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture22 = {"./assets/textures/streetgrass.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};

  texture_cache_init(&game->texture_cache);
  // TODO: Keep this style and add array style
  texture_cache_add(&game->texture_cache, gpu_api, 14, texture1, texture2, texture3, texture4, texture5, texture6, texture7, texture8, texture9, texture10, texture11, texture12, texture13, texture14);
  texture_cache_add(&game->texture_cache, gpu_api, 8, texture15, texture16, texture17, texture18, texture19, texture20, texture21, texture22);

  array_list_init(&game->sprites);
  array_list_init(&game->animated_sprites);

  player_camera_init(&game->player_camera);
  game->player = calloc(1, sizeof(struct Player));
  player_init(game->player, gpu_api, game);

  game->sandcastle = calloc(1, sizeof(struct Sandcastle));
  sandcastle_init(game->sandcastle, gpu_api, game);

  float draw_scale = 1.0f / 5.0f;

  // Load objects, reset unload/reload from scratch
  // Draw sprites from front to back for transparencies
  // Fore wood fence
  float fence_scale = draw_scale * 0.75f;
  for (int loop_num = 0; loop_num < 10; loop_num++) {
    struct Sprite* sprite = calloc(1, sizeof(struct Sprite));
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/forewoodfence.png"));
    sprite->position = (vec3){.x = -3.0f - (loop_num * sprite->width) * 0.92f * fence_scale, .y = -3.125f, .z = -2.0f + loop_num * 0.000001f};
    sprite->scale = (vec3){.x = fence_scale, .y = fence_scale, .z = fence_scale};
    array_list_add(&game->sprites, sprite);
  }

  // HUD
  //game->hud_sprite = calloc(1, sizeof(struct Sprite));
  //sprite_init(game->hud_sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/hud.png"));
  ////sprite->position = (vec3){.x = (loop_num * sprite->width) * 0.999f * draw_scale, .y = 10.0f, .z = 50.0f + (loop_num * 0.000001)};
  //game->hud_sprite->position = (vec3){.x = game->camera.position.x, .y = game->camera.position.y, .z = -0.1f};
  //game->hud_sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
  //array_list_add(&game->sprites, game->hud_sprite);

  // Sandbox
  struct Sprite* sandbox_sprite = calloc(1, sizeof(struct Sprite));
  sprite_init(sandbox_sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/sandbox.png"));
  sandbox_sprite->position = (vec3){.x = -3.8f, .y = -1.5f, .z = 0.0f};
  sandbox_sprite->scale = (vec3){.x = draw_scale * 1.75, .y = draw_scale * 1.75, .z = draw_scale * 1.75};
  array_list_add(&game->sprites, sandbox_sprite);

  // Trash
  struct Sprite* trash_sprite = calloc(1, sizeof(struct Sprite));
  sprite_init(trash_sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/trash.png"));
  trash_sprite->position = (vec3){.x = -1.35f, .y = -0.125f, .z = 0.0f};
  trash_sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
  array_list_add(&game->sprites, trash_sprite);
  struct Sprite* trash_sprite2 = calloc(1, sizeof(struct Sprite));
  sprite_init(trash_sprite2, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/trash2.png"));
  trash_sprite2->position = (vec3){.x = -3.8f, .y = -0.125f, .z = 0.0f};
  trash_sprite2->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
  array_list_add(&game->sprites, trash_sprite2);

  // Sign
  struct Sprite* sign_sprite = calloc(1, sizeof(struct Sprite));
  sprite_init(sign_sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/sign.png"));
  sign_sprite->position = (vec3){.x = -2.3f, .y = 0.45f, .z = 0.0f};
  sign_sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
  array_list_add(&game->sprites, sign_sprite);

  // Fence
  for (int loop_num = 0; loop_num < 10; loop_num++) {
    struct Sprite* sprite = calloc(1, sizeof(struct Sprite));
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/fence.png"));
    sprite->position = (vec3){.x = 10.0f - (loop_num * sprite->width) * 0.92f * draw_scale, .y = 0.525f, .z = loop_num * 0.000001f};
    sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
    array_list_add(&game->sprites, sprite);
  }

  // Grass
  for (int loop_num = 0; loop_num < 25; loop_num++) {
    //if (loop_num <= 5) {
    //  struct Sprite* sprite = calloc(1, sizeof(struct Sprite));
    //  sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/street.png"));
    //  sprite->position = (vec3){.x = 14.035f - (loop_num * sprite->width) * 0.995f * draw_scale, .y = -1.75, .z = 0.01 + (loop_num * 0.000001)};
    //  sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
    //  array_list_add(&game->sprites, sprite);
    //} else if (loop_num <= 7) {
    if (loop_num <= 7) {
      struct Sprite* sprite = calloc(1, sizeof(struct Sprite));
      sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/grass.png"));
      sprite->position = (vec3){.x = 14.0f - (loop_num * sprite->width) * 0.98f * draw_scale, .y = -1.675, .z = 0.01 + (loop_num * 0.000001)};
      sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
      array_list_add(&game->sprites, sprite);
    } else {
      struct Sprite* sprite = calloc(1, sizeof(struct Sprite));
      sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/grassshadow.png"));
      sprite->position = (vec3){.x = 14.0f - (loop_num * sprite->width) * 0.98f * draw_scale, .y = -1.675, .z = 0.01 + (loop_num * 0.000001)};
      sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
      array_list_add(&game->sprites, sprite);
    }
  }

  // Wood fence
  for (int loop_num = 0; loop_num < 20; loop_num++) {
    struct Sprite* sprite = calloc(1, sizeof(struct Sprite));
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/woodfence.png"));
    sprite->position = (vec3){.x = -4.37125f - (loop_num * sprite->width) * 0.92f * draw_scale, .y = 0.825f, .z = 0.011f + loop_num * 0.000001f};
    sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
    array_list_add(&game->sprites, sprite);
  }

  // Sign 2
  struct Sprite* sign_sprite2 = calloc(1, sizeof(struct Sprite));
  sprite_init(sign_sprite2, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/sign2.png"));
  sign_sprite2->position = (vec3){.x = 0.15f, .y = 1.1f, .z = 0.0111f};
  sign_sprite2->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
  array_list_add(&game->sprites, sign_sprite2);

  // Background
  struct Sprite* back_sprite = calloc(1, sizeof(struct Sprite));
  sprite_init(back_sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/back2.png"));
  back_sprite->position = (vec3){.x = 1.0f, .y = 1.1f, .z = 7.0f};
  back_sprite->scale = (vec3){.x = draw_scale * 2, .y = draw_scale * 2, .z = draw_scale * 2};
  array_list_add(&game->sprites, back_sprite);

  // Clouds
  float cloud_scale = 10.0f * draw_scale;
  for (int loop_num = 0; loop_num < 2; loop_num++) {
    struct Sprite* sprite = calloc(1, sizeof(struct Sprite));
    float cloud_scale = 10.0f * draw_scale;
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/clouds.png"));
    sprite->position = (vec3){.x = (loop_num * sprite->width) * 0.999f * draw_scale, .y = 10.0f, .z = 50.0f + (loop_num * 0.000001)};
    sprite->scale = (vec3){.x = cloud_scale, .y = cloud_scale, .z = cloud_scale};
    array_list_add(&game->sprites, sprite);
  }
}

void game_delete(struct Game* game, struct Mana* mana) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  // Wait for command buffers to finish before deleting desciptor sets
  vkWaitForFences(gpu_api->vulkan_state->device, 2, gpu_api->vulkan_state->swap_chain->in_flight_fences, VK_TRUE, UINT64_MAX);

  sandcastle_delete(game->sandcastle, gpu_api);
  free(game->sandcastle);

  texture_cache_delete(&game->texture_cache, gpu_api);

  for (int sprite_num = 0; sprite_num < array_list_size(&game->sprites); sprite_num++) {
    struct Sprite* sprite = array_list_get(&game->sprites, sprite_num);
    sprite_delete(sprite, gpu_api);
    free(sprite);
  }

  for (int sprite_animation_num = 0; sprite_animation_num < array_list_size(&game->animated_sprites); sprite_animation_num++) {
    struct SpriteAnimation* sprite_animation = array_list_get(&game->animated_sprites, sprite_animation_num);
    sprite_animation_delete(sprite_animation, gpu_api);
    free(sprite_animation);
  }

  array_list_delete(&game->sprites);
  array_list_delete(&game->animated_sprites);
  sprite_animation_shader_delete(&game->sprite_animation_shader, gpu_api);
  sprite_shader_delete(&game->sprite_shader, gpu_api);

  fxaa_shader_delete(&game->fxaa_shader, gpu_api);

  audio_clip_cache_delete(game->music_clip_cache);
  audio_clip_cache_delete(game->fart_clip_cache);
  audio_manager_delete(&game->audio_manager);

  free(game->music_clip);

  free(game->music_clip_cache);
  free(game->fart_clip_cache);
}

void game_update(struct Game* game, struct Mana* mana, double delta_time) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  // When the window is resized everything must be recreated in vulkan
  if (mana->engine.gpu_api.vulkan_state->reset_shaders) {
    mana->engine.gpu_api.vulkan_state->reset_shaders = 0;
    vkDeviceWaitIdle(mana->engine.gpu_api.vulkan_state->device);

    fxaa_shader_delete(&game->fxaa_shader, gpu_api);
    fxaa_shader_init(&game->fxaa_shader, gpu_api);

    sprite_shader_delete(&game->sprite_shader, gpu_api);
    sprite_shader_init(&game->sprite_shader, gpu_api);

    sprite_animation_shader_delete(&game->sprite_animation_shader, gpu_api);
    sprite_animation_shader_init(&game->sprite_animation_shader, gpu_api);

    for (int sprite_num = 0; sprite_num < array_list_size(&game->sprites); sprite_num++) {
      struct Sprite* sprite = array_list_get(&game->sprites, sprite_num);
      sprite_recreate(sprite, gpu_api);
    }

    for (int sprite_animation_num = 0; sprite_animation_num < array_list_size(&game->animated_sprites); sprite_animation_num++) {
      struct SpriteAnimation* sprite_animation = array_list_get(&game->animated_sprites, sprite_animation_num);
      sprite_animation_recreate(sprite_animation, gpu_api);
    }

    sandcastle_recreate(game->sandcastle, gpu_api);
    player_recreate(game->player, gpu_api);
  }

  sandcastle_update(game->sandcastle, game, delta_time);
  player_update(game->player, game, delta_time);
  player_camera_update(&game->player_camera, delta_time);
  game_update_input(game, &mana->engine);

  gpu_api->vulkan_state->gbuffer->projection_matrix = camera_get_projection_matrix(&game->player_camera.camera, game->window);
  gpu_api->vulkan_state->gbuffer->view_matrix = camera_get_view_matrix(&game->player_camera.camera);
  //game->hud_sprite->position = (vec3){.x = game->camera.position.x + 5.5f, .y = game->camera.position.y + 2.9f, .z = game->camera.position.z + 8.0f};
  /////////////////////////////////////////////////////////////////
  for (int entity_num = 0; entity_num < array_list_size(&game->sprites); entity_num++) {
    struct Sprite* sprite = (struct Sprite*)array_list_get(&game->sprites, entity_num);
    sprite_update_uniforms(sprite, gpu_api);
  }

  for (int sprite_animation_num = 0; sprite_animation_num < array_list_size(&game->animated_sprites); sprite_animation_num++) {
    struct SpriteAnimation* sprite_animation = array_list_get(&game->animated_sprites, sprite_animation_num);
    sprite_animation_update_uniforms(sprite_animation, gpu_api);
  }
  /////////////////////////////////////////////////////////////////

  gbuffer_start(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  for (int sprite_num = array_list_size(&game->sprites) - 1; sprite_num >= 0; sprite_num--)
    sprite_render(array_list_get(&game->sprites, sprite_num), gpu_api);
  sandcastle_render(game->sandcastle, gpu_api);
  player_render(game->player, gpu_api);
  gbuffer_stop(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  blit_post_process_render(gpu_api->vulkan_state->post_process->blit_post_process, gpu_api);

  fxaa_shader_render(&game->fxaa_shader, gpu_api);

  blit_swap_chain_render(gpu_api->vulkan_state->swap_chain->blit_swap_chain, gpu_api);

  float duration = delta_time;
  if (duration <= 0.0f)
    return;
  else if (duration > 0.05f)
    duration = 0.05f;
}

void game_update_input(struct Game* game, struct Engine* engine) {
  int in_focus = glfwGetWindowAttrib(engine->graphics_library.glfw_library.glfw_window, GLFW_FOCUSED);
  if (in_focus == 0)
    return;

  struct InputManager* input_manager = game->window->input_manager;

  if (input_manager->keys[GLFW_KEY_ESCAPE].state == PRESSED)
    glfwSetWindowShouldClose(engine->graphics_library.glfw_library.glfw_window, 1);

  if (input_manager->keys[GLFW_KEY_1].pushed == 1) {
    game->fxaa_shader.fxaa_on ^= 1;
    game->fxaa_shader.fxaa_on ? printf("FXAA ON\n") : printf("FXAA OFF\n");
  }

  //if (input_manager->keys[GLFW_KEY_2].pushed == 1)
  //  audio_manager_play_audio_clip(game->audio_manager, game->fart_clip);

  if (input_manager->keys[GLFW_KEY_O].pushed == 1) {
    game->audio_manager.master_volume -= 0.1f;
    if (game->audio_manager.master_volume < 0.0f)
      game->audio_manager.master_volume = 0.0f;
  }
  if (input_manager->keys[GLFW_KEY_P].pushed == 1) {
    game->audio_manager.master_volume += 0.1f;
    if (game->audio_manager.master_volume > 1.0f)
      game->audio_manager.master_volume = 1.0f;
  }
}
