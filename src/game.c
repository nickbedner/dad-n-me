#include "game.h"

int split_thread(void* aArg) {
  printf("Starting audio thread!\n");
  audio_manager_start((struct AudioManager*)aArg);

  return 0;
}

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  game->audio_manager = calloc(1, sizeof(struct AudioManager));
  audio_manager_init(game->audio_manager);

  thrd_t t;
  if (thrd_create(&t, split_thread, game->audio_manager) == thrd_success)
    ;
  //thrd_detach(t);
  //thrd_join(t, NULL);

  game->music_clip_cache = calloc(1, sizeof(struct AudioClipCache));
  game->fart_clip_cache = calloc(1, sizeof(struct AudioClipCache));
  audio_clip_cache_init(game->music_clip_cache, "assets/audio/music/Dad_n_Me.wav");
  audio_clip_cache_init(game->fart_clip_cache, "assets/audio/sounds/fart_2.wav");

  game->music_clip = calloc(1, sizeof(struct AudioClip));
  game->fart_clip = calloc(1, sizeof(struct AudioClip));
  audio_clip_init(game->music_clip, game->music_clip_cache, MUSIC_AUDIO_CLIP, 1, 0.5f, 0.0125f);
  audio_clip_init(game->fart_clip, game->fart_clip_cache, MUSIC_AUDIO_CLIP, 0, 0.75f, 0.0f);

  audio_manager_play_audio_clip(game->audio_manager, game->music_clip);

  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  game->fxaa_on = 1;
  game->window = window;

  camera_init(&game->camera);

  vec3 added_pos = vec3_scale(game->camera.front, 8);
  game->camera.position = vec3_add(game->camera.position, added_pos);
  game->camera.yaw += 180;

  fxaa_shader_init(&game->fxaa_shader, gpu_api);
  sprite_shader_init(&game->sprite_shader, gpu_api);
  sprite_animation_shader_init(&game->sprite_animation_shader, gpu_api);

  struct TextureSettings texture1 = {"./assets/textures/fence.psd", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture2 = {"./assets/textures/grass.png", FILTER_NEAREST, MODE_CLAMP_TO_BORDER, 0};   // 0
  struct TextureSettings texture3 = {"./assets/textures/clouds.png", FILTER_NEAREST, MODE_CLAMP_TO_BORDER, 0};  // 0
  struct TextureSettings texture4 = {"./assets/textures/back2.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture5 = {"./assets/textures/trash.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture6 = {"./assets/textures/sign.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture7 = {"./assets/textures/woodfence.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 0};    // 0
  struct TextureSettings texture8 = {"./assets/textures/grassshadow.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 0};  // 0
  struct TextureSettings texture9 = {"./assets/textures/grassdiffer.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 0};  // 0
  struct TextureSettings texture10 = {"./assets/textures/trash2.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture11 = {"./assets/textures/sign2.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture12 = {"./assets/textures/hud.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture13 = {"./assets/textures/sandbox.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture14 = {"./assets/textures/walkingspritesheet.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture15 = {"./assets/textures/standingspritesheet.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 1};
  struct TextureSettings texture16 = {"./assets/textures/sandcastle.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 0};
  struct TextureSettings texture17 = {"./assets/textures/sandcastlespritesheet.png", FILTER_LINEAR, MODE_CLAMP_TO_BORDER, 0};

  texture_cache_init(&game->texture_cache);
  texture_cache_add(&game->texture_cache, gpu_api, 14, texture1, texture2, texture3, texture4, texture5, texture6, texture7, texture8, texture9, texture10, texture11, texture12, texture13, texture14);
  texture_cache_add(&game->texture_cache, gpu_api, 3, texture15, texture16, texture17);

  array_list_init(&game->sprites);
  array_list_init(&game->animated_sprites);

  float draw_scale = 1.0f / 5.0f;

  // Draw sprites from front to back for transparencies
  // HUD
  game->hud_sprite = calloc(1, sizeof(struct Sprite));
  sprite_init(game->hud_sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/hud.png"));
  //sprite->position = (vec3){.x = (loop_num * sprite->width) * 0.999f * draw_scale, .y = 10.0f, .z = 50.0f + (loop_num * 0.000001)};
  game->hud_sprite->position = (vec3){.x = game->camera.position.x, .y = game->camera.position.y, .z = -0.1f};
  game->hud_sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
  array_list_add(&game->sprites, game->hud_sprite);

  // Sandcastle
  float sand_castle_size = draw_scale * 0.85f;
  game->sandcastle_position = (vec3){.x = -3.333f, .y = -1.4f, .z = -0.001f};
  game->sandcastle = calloc(1, sizeof(struct Sprite));
  sprite_init(game->sandcastle, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/sandcastle.png"));
  game->sandcastle->position = game->sandcastle_position;
  game->sandcastle->scale = (vec3){.x = sand_castle_size, .y = sand_castle_size, .z = sand_castle_size};

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
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/fence.psd"));
    sprite->position = (vec3){.x = 10.0f - (loop_num * sprite->width) * 0.92f * draw_scale, .y = 0.525f, .z = loop_num * 0.000001f};
    sprite->scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};
    array_list_add(&game->sprites, sprite);
  }

  // Grass
  for (int loop_num = 0; loop_num < 25; loop_num++) {
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

  // Wood Fence
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

  // Animations
  // Standing animation
  struct SpriteAnimation* standing_animation = calloc(1, sizeof(struct SpriteAnimation));
  sprite_animation_init(standing_animation, gpu_api, &game->sprite_animation_shader, texture_cache_get(&game->texture_cache, "./assets/textures/standingspritesheet.png"), 4, 1.0f / 10.0f, 0);
  standing_animation->position = (vec3){.x = 0.0f * draw_scale, .y = 0.0f, .z = -0.01f};
  standing_animation->scale = (vec3){.x = 1.0f * draw_scale, .y = 1.0f * draw_scale, .z = 1.0f * draw_scale};
  array_list_add(&game->animated_sprites, standing_animation);

  // Walking animation
  struct SpriteAnimation* walking_animation = calloc(1, sizeof(struct SpriteAnimation));
  sprite_animation_init(walking_animation, gpu_api, &game->sprite_animation_shader, texture_cache_get(&game->texture_cache, "./assets/textures/walkingspritesheet.png"), 11, 1.0f / 30.0f, 0);
  walking_animation->position = (vec3){.x = 0.0f * draw_scale, .y = 0.0f, .z = -0.01f};
  walking_animation->scale = (vec3){.x = 1.0f * draw_scale, .y = 1.0f * draw_scale, .z = 1.0f * draw_scale};
  array_list_add(&game->animated_sprites, walking_animation);

  // Sandcastle animation
  game->sandcastle_animation = calloc(1, sizeof(struct SpriteAnimation));
  sprite_animation_init(game->sandcastle_animation, gpu_api, &game->sprite_animation_shader, texture_cache_get(&game->texture_cache, "./assets/textures/sandcastlespritesheet.png"), 13, 1.0f / 30.0f, 0);
  game->sandcastle_animation->position = game->sandcastle_position;
  game->sandcastle_animation->scale = (vec3){.x = sand_castle_size, .y = sand_castle_size, .z = sand_castle_size};

  game->character_state = CHARACTER_IDLE_STATE;
  game->character_position = VEC2_ZERO;
  game->character_direction = 1;

  game->sandcastle_state = SANDCASTLE_FULL_STATE;
}

void game_delete(struct Game* game, struct Mana* mana) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  // Wait for command buffers to finish before deleting desciptor sets
  vkWaitForFences(gpu_api->vulkan_state->device, 2, gpu_api->vulkan_state->swap_chain->in_flight_fences, VK_TRUE, UINT64_MAX);

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
  audio_manager_delete(game->audio_manager);

  free(game->music_clip);
  free(game->fart_clip);

  free(game->music_clip_cache);
  free(game->fart_clip_cache);
  free(game->audio_manager);
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

    for (int sprite_num = 0; sprite_num < array_list_size(&game->sprites); sprite_num++) {
      struct Sprite* sprite = array_list_get(&game->sprites, sprite_num);
      sprite_recreate(sprite, gpu_api);
    }

    for (int sprite_animation_num = 0; sprite_animation_num < array_list_size(&game->animated_sprites); sprite_animation_num++) {
      struct SpriteAnimation* sprite_animation = array_list_get(&game->animated_sprites, sprite_animation_num);
      sprite_animation_recreate(sprite_animation, gpu_api);
    }
  }

  game_update_input(game, &mana->engine);
  game_update_camera(game, &mana->engine);
  camera_update_vectors(&game->camera);
  gpu_api->vulkan_state->gbuffer->projection_matrix = camera_get_projection_matrix(&game->camera, game->window);
  gpu_api->vulkan_state->gbuffer->view_matrix = camera_get_view_matrix(&game->camera);
  game->hud_sprite->position = (vec3){.x = game->camera.position.x + 5.5f, .y = game->camera.position.y + 2.9f, .z = game->camera.position.z + 8.0f};
  game_update_uniform_buffers(game, &mana->engine);

  gbuffer_start(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  for (int sprite_num = array_list_size(&game->sprites) - 1; sprite_num >= 0; sprite_num--)
    sprite_render(array_list_get(&game->sprites, sprite_num), gpu_api, delta_time);

  switch (game->sandcastle_state) {
    case (SANDCASTLE_FULL_STATE):
      if (fabs((game->sandcastle_position.x - game->character_position.x + 0.25f) / 2.0f) + fabs(game->sandcastle_position.y - game->character_position.y + 0.5f) < 0.35f) {
        game->sandcastle_state = SANDCASTLE_ANIMATING_STATE;
        game->sandcastle_animation->loop = 0;
        audio_manager_play_audio_clip(game->audio_manager, game->fart_clip);
      }
      sprite_render(game->sandcastle, gpu_api, delta_time);
      break;
    case (SANDCASTLE_ANIMATING_STATE):
      sprite_animation_update(game->sandcastle_animation, delta_time);
      if (game->sandcastle_animation->current_frame == game->sandcastle_animation->total_frames)
        game->sandcastle_state = SANDCASTLE_CRUSHED_STATE;
    case (SANDCASTLE_CRUSHED_STATE):
      sprite_animation_render(game->sandcastle_animation, gpu_api, delta_time);
      break;
  }

  for (int sprite_animation_num = array_list_size(&game->animated_sprites) - 1; sprite_animation_num >= 0; sprite_animation_num--) {
    struct SpriteAnimation* sprite_animation = array_list_get(&game->animated_sprites, sprite_animation_num);
    sprite_animation->direction = game->character_direction;
    sprite_animation_update(sprite_animation, delta_time);
    sprite_animation->position.x = game->character_position.x;
    sprite_animation->position.y = game->character_position.y;
    if (game->character_state == sprite_animation_num)
      sprite_animation_render(sprite_animation, gpu_api, delta_time);
  }

  //switch (game->character_state) {
  //  case (IDLE_STATE):
  //    sprite_animation_render(array_list_get(&game->animated_sprites, 0), gpu_api, delta_time);
  //    break;
  //}
  gbuffer_stop(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  blit_post_process_render(gpu_api->vulkan_state->post_process->blit_post_process, gpu_api);

  if (game->fxaa_on)
    fxaa_shader_render(&game->fxaa_shader, gpu_api);

  blit_swap_chain_render(gpu_api->vulkan_state->swap_chain->blit_swap_chain, gpu_api);

  float duration = delta_time;
  if (duration <= 0.0f)
    return;
  else if (duration > 0.05f)
    duration = 0.05f;
}

// TODO: Add mouse let go no skip
void game_update_camera(struct Game* game, struct Engine* engine) {
  int in_focus = glfwGetWindowAttrib(engine->graphics_library.glfw_library.glfw_window, GLFW_FOCUSED);
  if (in_focus == 0) {
    //game->camera.prev_yaw = game->camera.prev_pitch = game->camera.pitch = game->camera.prev_roll = game->camera.roll = 0.0f;
    return;
  }

  struct InputManager* input_manager = game->window->input_manager;

  int mouse_locked = (glfwGetMouseButton(engine->graphics_library.glfw_library.glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) ? 1 : 0;
  mouse_locked ? glfwSetInputMode(engine->graphics_library.glfw_library.glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED) : glfwSetInputMode(engine->graphics_library.glfw_library.glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  GLdouble x_pos, y_pos;
  glfwGetCursorPos(engine->graphics_library.glfw_library.glfw_window, &x_pos, &y_pos);

  double x_diff = input_manager->last_mouse_x_pos - x_pos;
  double y_diff = input_manager->last_mouse_y_pos - y_pos;

  input_manager->last_mouse_x_pos = x_pos;
  input_manager->last_mouse_y_pos = y_pos;

  // Handles mouse held and big jump edge cases
  if ((mouse_locked == 0 && game->camera.mouse_locked == 1) || (mouse_locked == 0 && (x_pos <= 0 || y_pos <= 0 || x_pos >= game->window->width || y_pos >= game->window->height)) || (fabs(x_diff) > (game->window->width / 2.0f) || fabs(y_diff) > (game->window->height / 2.0f)))
    x_diff = y_diff = 0;

  x_diff *= game->camera.sensitivity;
  y_diff *= game->camera.sensitivity;

  //game->camera.yaw -= x_diff;
  //game->camera.pitch += y_diff;

  /*if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
      float right_x_axis = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] * game->camera.sensitivity * 15;
      float right_y_axis = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] * game->camera.sensitivity * 15;

      if (fabs(right_x_axis) < 0.2f)
        right_x_axis = 0.0f;

      if (fabs(right_y_axis) < 0.2f)
        right_y_axis = 0.0f;

      input_manager->last_mouse_x_pos += right_x_axis;
      input_manager->last_mouse_y_pos -= right_y_axis;

      game->camera.yaw += right_x_axis;
      game->camera.pitch -= right_y_axis;
    }
  }*/

  if (game->camera.pitch > 89.0f)
    game->camera.pitch = 89.0f;
  if (game->camera.pitch < -89.0f)
    game->camera.pitch = -89.0f;

  game->camera.mouse_locked = mouse_locked;
}

static inline float camera_lerp(float start, float end, float t) {
  //return start * (1.0f - t) + end * t;
  return start + (end - start) * t;
}
float max_camera_velocity = 0.25f;
float camera_left_right_velocity = 0.0f;
float camera_in_out_velocity = 0.0f;
float camera_up_down_velocity = 0.0f;
void game_update_input(struct Game* game, struct Engine* engine) {
  int in_focus = glfwGetWindowAttrib(engine->graphics_library.glfw_library.glfw_window, GLFW_FOCUSED);
  if (in_focus == 0)
    return;

  struct InputManager* input_manager = game->window->input_manager;

  if (input_manager->keys[GLFW_KEY_ESCAPE].state == PRESSED)
    glfwSetWindowShouldClose(engine->graphics_library.glfw_library.glfw_window, 1);

  int side_button_pressed = 0;
  int up_button_pressed = 0;

  game->character_state = CHARACTER_IDLE_STATE;

  float delta_time = 0.1f;
  //float movement_speed = 1.0f;
  //
  //if (input_manager->keys[GLFW_KEY_LEFT_SHIFT].state == PRESSED)
  //  movement_speed *= 5;

  //float velocity = camera_velocity + movement_speed * delta_time;
  // Left and right
  float left_right_velocity = camera_left_right_velocity * delta_time;
  vec3 added_pos = vec3_scale(game->camera.right, left_right_velocity);
  if (input_manager->keys[GLFW_KEY_A].state == PRESSED) {
    game->character_state = CHARACTER_WALKING_STATE;
    game->character_direction = -1;
    game->character_position.x += 0.025f;
    //if (camera_left_right_velocity < max_camera_velocity)
    //  camera_left_right_velocity -= 0.005f;
  }
  if (input_manager->keys[GLFW_KEY_D].state == PRESSED) {
    game->character_state = CHARACTER_WALKING_STATE;
    side_button_pressed++;
    game->character_direction = 1;
    game->character_position.x -= 0.025f;
    //if (camera_left_right_velocity > -max_camera_velocity)
    //  camera_left_right_velocity += 0.005f;
  }
  int direction = (game->camera.position.x > game->character_position.x) ? 0 : 1;
  if (fabs(camera_left_right_velocity) < max_camera_velocity) {
    if (fabs(game->camera.position.x - game->character_position.x) > 0.25f) {
      camera_left_right_velocity += (game->camera.position.x - game->character_position.x) * 0.01f;
      //if (direction)
      //  camera_left_right_velocity -= 0.005f;
      //else
      //  camera_left_right_velocity += 0.005f;
    }
  }
  game->camera.position = vec3_add(game->camera.position, added_pos);

  // In and out
  float in_out_velocity = camera_in_out_velocity * delta_time;
  added_pos = vec3_scale(game->camera.front, in_out_velocity);
  if (input_manager->keys[GLFW_KEY_W].state == PRESSED) {
    game->character_state = CHARACTER_WALKING_STATE;
    up_button_pressed++;
    game->character_position.y += 0.015f;
    //if (camera_in_out_velocity < max_camera_velocity)
    //  camera_in_out_velocity += 0.005f;
  }
  if (input_manager->keys[GLFW_KEY_S].state == PRESSED) {
    game->character_state = CHARACTER_WALKING_STATE;
    up_button_pressed++;
    game->character_position.y -= 0.015f;
    //if (camera_in_out_velocity < max_camera_velocity)
    //  camera_in_out_velocity -= 0.005f;
  }
  game->camera.position = vec3_add(game->camera.position, added_pos);

  float up_down_velocity = camera_up_down_velocity * delta_time;
  added_pos = vec3_scale(game->camera.up, up_down_velocity);
  if (input_manager->keys[GLFW_KEY_E].state == PRESSED) {
    if (camera_up_down_velocity < max_camera_velocity)
      camera_up_down_velocity += 0.005f;
  }
  if (input_manager->keys[GLFW_KEY_Q].state == PRESSED) {
    if (camera_up_down_velocity < max_camera_velocity)
      camera_up_down_velocity -= 0.005f;
  }
  game->camera.position = vec3_add(game->camera.position, added_pos);

  if (input_manager->keys[GLFW_KEY_1].pushed == 1) {
    game->fxaa_on ^= 1;
    game->fxaa_on ? printf("FXAA ON\n") : printf("FXAA OFF\n");
  }

  if (input_manager->keys[GLFW_KEY_2].pushed == 1)
    audio_manager_play_audio_clip(game->audio_manager, game->fart_clip);

  if (input_manager->keys[GLFW_KEY_O].pushed == 1) {
    game->audio_manager->master_volume -= 0.1f;
    if (game->audio_manager->master_volume < 0.0f)
      game->audio_manager->master_volume = 0.0f;
  }
  if (input_manager->keys[GLFW_KEY_P].pushed == 1) {
    game->audio_manager->master_volume += 0.1f;
    if (game->audio_manager->master_volume > 1.0f)
      game->audio_manager->master_volume = 1.0f;
  }
  //if (side_button_pressed == 0)
  camera_left_right_velocity *= 0.95f;
  //if (up_button_pressed == 0)
  camera_in_out_velocity *= 0.95f;
  camera_up_down_velocity *= 0.95f;

  /*if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
      float left_x_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
      float left_y_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

      if (fabs(left_x_axis) < 0.2f)
        left_x_axis = 0.0f;

      if (fabs(left_y_axis) < 0.2f)
        left_y_axis = 0.0f;

      float right_trigger = (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) / 2.0f;

      //velocity *= (right_trigger * 10.0f) + 1.0f;

      vec3 added_pos_y = vec3_scale(game->camera.front, velocity * left_y_axis);
      game->camera.position = vec3_sub(game->camera.position, added_pos_y);
      vec3 added_pos_x = vec3_scale(game->camera.right, velocity * left_x_axis);
      game->camera.position = vec3_add(game->camera.position, added_pos_x);

      if (state.buttons[GLFW_GAMEPAD_BUTTON_A]) {
        game->fxaa_on ^= 1;
        game->fxaa_on ? printf("FXAA ON\n") : printf("FXAA OFF\n");
      }
    }
  }*/
}

void game_update_uniform_buffers(struct Game* game, struct Engine* engine) {
  //double time = fmod(engine_get_time(), UM_PI * 8.0);
  for (int entity_num = 0; entity_num < array_list_size(&game->sprites); entity_num++) {
    struct Sprite* sprite = (struct Sprite*)array_list_get(&game->sprites, entity_num);
    sprite_update_uniforms(sprite, &engine->gpu_api);
  }

  sprite_update_uniforms(game->sandcastle, &engine->gpu_api);

  for (int sprite_animation_num = 0; sprite_animation_num < array_list_size(&game->animated_sprites); sprite_animation_num++) {
    struct SpriteAnimation* sprite_animation = array_list_get(&game->animated_sprites, sprite_animation_num);
    sprite_animation_update_uniforms(sprite_animation, &engine->gpu_api);
  }

  sprite_animation_update_uniforms(game->sandcastle_animation, &engine->gpu_api);
}
