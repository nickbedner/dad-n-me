#include "game.h"

int split_thread(void* aArg) {
  printf("Hello world!\n");
  audio_manager_start((struct AudioManager*)aArg);

  return 0;
}

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  audio_manager_init(&game->audio_manager);

  thrd_t t;
  if (thrd_create(&t, split_thread, &game->audio_manager) == thrd_success)
    ;
  //thrd_detach(t);
  //thrd_join(t, NULL);

  audio_clip_init(&game->music_clip, "assets/audio/music/Dad_n_Me.wav", MUSIC_AUDIO_CLIP, 1);

  audio_manager_play_audio_clip(&game->audio_manager, &game->music_clip);

  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  game->fxaa_on = 1;
  game->window = window;

  camera_init(&game->camera);

  vec3 added_pos = vec3_scale(game->camera.front, 2);
  game->camera.position = vec3_add(game->camera.position, added_pos);
  game->camera.yaw += 180;
  //game->camera.orientation = quaternion_mul(game->camera.orientation, (quat){.x = 0.5f, .y = 0.0f, .z = 0.0f, .w = 1.0f});

  fxaa_shader_init(&game->fxaa_shader, gpu_api);

  struct TextureSettings texture1 = {"./assets/textures/fence.png", FILTER_LINEAR};
  struct TextureSettings texture2 = {"./assets/textures/grass.png", FILTER_LINEAR};
  struct TextureSettings texture3 = {"./assets/textures/clouds.png", FILTER_LINEAR};

  texture_cache_init(&game->texture_cache);
  texture_cache_add(&game->texture_cache, gpu_api, 3, texture1, texture2, texture3);

  sprite_shader_init(&game->sprite_shader, gpu_api);

  array_list_init(&game->sprites);
  for (int loop_num = 0; loop_num < 10; loop_num++) {
    struct Sprite* sprite = malloc(sizeof(struct Sprite));
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/fence.png"));
    sprite->position = (vec3){.x = (loop_num * sprite->width) * 0.92f, .y = 0.0f, .z = loop_num * 0.001f};
    sprite->rotation = (quat){.data[0] = 0, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f};
    //sprite->rotation = (quat){.data[0] = loop_num / 3.0f, .data[1] = loop_num / 3.0f, .data[2] = loop_num / 3.0f, .data[3] = 1.0f};
    array_list_add(&game->sprites, sprite);
  }

  for (int loop_num = 0; loop_num < 10; loop_num++) {
    struct Sprite* sprite = malloc(sizeof(struct Sprite));
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/grass.png"));
    sprite->position = (vec3){.x = (loop_num * sprite->width) * 0.99f, .y = -sprite->height / 2.0, .z = 0.01 + (loop_num * 0.01)};
    sprite->rotation = (quat){.data[0] = 0, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f};
    //sprite->rotation = (quat){.data[0] = loop_num / 3.0f, .data[1] = loop_num / 3.0f, .data[2] = loop_num / 3.0f, .data[3] = 1.0f};
    array_list_add(&game->sprites, sprite);
  }

  for (int loop_num = 0; loop_num < 2; loop_num++) {
    struct Sprite* sprite = malloc(sizeof(struct Sprite));
    sprite_init(sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/clouds.png"));
    sprite->position = (vec3){.x = (loop_num * sprite->width) * 1.0f, .y = 0.0f, .z = 0.1 + (loop_num * 0.01)};
    sprite->rotation = (quat){.data[0] = 0, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f};
    //sprite->rotation = (quat){.data[0] = loop_num / 3.0f, .data[1] = loop_num / 3.0f, .data[2] = loop_num / 3.0f, .data[3] = 1.0f};
    array_list_add(&game->sprites, sprite);
  }
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

  array_list_delete(&game->sprites);
  sprite_shader_delete(&game->sprite_shader, gpu_api);

  fxaa_shader_delete(&game->fxaa_shader, gpu_api);

  audio_clip_delete(&game->music_clip);
  audio_manager_delete(&game->audio_manager);
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
  }

  //game->camera.prev_yaw = game->camera.yaw;
  //game->camera.prev_pitch = game->camera.pitch;
  //game->camera.prev_roll = game->camera.roll;

  game_update_input(game, &mana->engine);
  game_update_camera(game, &mana->engine);
  camera_update_vectors(&game->camera);
  gpu_api->vulkan_state->gbuffer->projection_matrix = camera_get_projection_matrix(&game->camera, game->window);
  gpu_api->vulkan_state->gbuffer->view_matrix = camera_get_view_matrix(&game->camera);
  game_update_uniform_buffers(game, &mana->engine);

  gbuffer_start(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  for (int sprite_num = array_list_size(&game->sprites) - 1; sprite_num >= 0; sprite_num--)
    sprite_render(array_list_get(&game->sprites, sprite_num), gpu_api, delta_time);
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

  //game->camera.yaw -= x_diff * sin(glm_rad(game->camera.roll)) - y_diff * cos(glm_rad(game->camera.roll));
  //game->camera.pitch += y_diff * sin(glm_rad(game->camera.roll)) + x_diff * cos(glm_rad(game->camera.roll));

  //game->camera.orientation = quaternion_mul(game->camera.orientation, (quat){.x = -y_diff, .y = -x_diff, .z = 0.0f, .w = 1.0f});
  //game->camera.orientation = quaternion_mul(game->camera.orientation, (quat){.x = -y_diff, .y = 0.0f, .z = 0.0f, .w = 1.0f});
  //game->camera.orientation = quaternion_mul(game->camera.orientation, (quat){.x = 0.0f, .y = -x_diff, .z = 0.0f, .w = 1.0f});
  game->camera.yaw -= x_diff;
  game->camera.pitch += y_diff;

  if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
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
  }

  if (game->camera.pitch > 89.0f)
    game->camera.pitch = 89.0f;
  if (game->camera.pitch < -89.0f)
    game->camera.pitch = -89.0f;

  game->camera.mouse_locked = mouse_locked;
}

void game_update_input(struct Game* game, struct Engine* engine) {
  int in_focus = glfwGetWindowAttrib(engine->graphics_library.glfw_library.glfw_window, GLFW_FOCUSED);
  if (in_focus == 0)
    return;

  struct InputManager* input_manager = game->window->input_manager;

  if (input_manager->keys[GLFW_KEY_ESCAPE].state == PRESSED)
    glfwSetWindowShouldClose(engine->graphics_library.glfw_library.glfw_window, 1);

  float delta_time = 0.1f;
  float movement_speed = 1.0f;

  if (input_manager->keys[GLFW_KEY_LEFT_SHIFT].state == PRESSED)
    movement_speed *= 5;

  float velocity = movement_speed * delta_time;
  if (input_manager->keys[GLFW_KEY_W].state == PRESSED) {
    vec3 added_pos = vec3_scale(game->camera.front, velocity);
    game->camera.position = vec3_add(game->camera.position, added_pos);
  }
  if (input_manager->keys[GLFW_KEY_S].state == PRESSED) {
    vec3 added_pos = vec3_scale(game->camera.front, velocity);
    game->camera.position = vec3_sub(game->camera.position, added_pos);
  }
  if (input_manager->keys[GLFW_KEY_A].state == PRESSED) {
    vec3 added_pos = vec3_scale(game->camera.right, velocity);
    game->camera.position = vec3_sub(game->camera.position, added_pos);
  }
  if (input_manager->keys[GLFW_KEY_D].state == PRESSED) {
    vec3 added_pos = vec3_scale(game->camera.right, velocity);
    game->camera.position = vec3_add(game->camera.position, added_pos);
  }
  if (input_manager->keys[GLFW_KEY_E].state == PRESSED) {
    vec3 added_pos = vec3_scale(game->camera.up, velocity);
    game->camera.position = vec3_add(game->camera.position, added_pos);
  }
  if (input_manager->keys[GLFW_KEY_Q].state == PRESSED) {
    vec3 added_pos = vec3_scale(game->camera.up, velocity);
    game->camera.position = vec3_sub(game->camera.position, added_pos);
  }
  if (input_manager->keys[GLFW_KEY_Z].state == PRESSED)
    game->camera.roll -= 1.0f;

  //game->camera.orientation = quaternion_mul(game->camera.orientation, (quat){.x = 0.0f, .y = 0.0f, .z = 0.05f, .w = 1.0f});
  if (input_manager->keys[GLFW_KEY_X].state == PRESSED)
    game->camera.roll += 1.0f;

  //game->camera.orientation = quaternion_mul(game->camera.orientation, (quat){.x = 0.0f, .y = 0.0f, .z = -0.05f, .w = 1.0f});

  if (input_manager->keys[GLFW_KEY_1].pushed == 1) {
    game->fxaa_on ^= 1;
    game->fxaa_on ? printf("FXAA ON\n") : printf("FXAA OFF\n");
  }

  if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
      float left_x_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
      float left_y_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

      if (fabs(left_x_axis) < 0.2f)
        left_x_axis = 0.0f;

      if (fabs(left_y_axis) < 0.2f)
        left_y_axis = 0.0f;

      float right_trigger = (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) / 2.0f;

      velocity *= (right_trigger * 10.0f) + 1.0f;

      vec3 added_pos_y = vec3_scale(game->camera.front, velocity * left_y_axis);
      game->camera.position = vec3_sub(game->camera.position, added_pos_y);
      vec3 added_pos_x = vec3_scale(game->camera.right, velocity * left_x_axis);
      game->camera.position = vec3_add(game->camera.position, added_pos_x);

      if (state.buttons[GLFW_GAMEPAD_BUTTON_A]) {
        game->fxaa_on ^= 1;
        game->fxaa_on ? printf("FXAA ON\n") : printf("FXAA OFF\n");
      }
    }
  }
}

void game_update_uniform_buffers(struct Game* game, struct Engine* engine) {
  //double time = fmod(engine_get_time(), UM_PI * 8.0);
  for (int entity_num = 0; entity_num < array_list_size(&game->sprites); entity_num++) {
    struct Sprite* sprite = (struct Sprite*)array_list_get(&game->sprites, entity_num);
    sprite_update_uniforms(sprite, &engine->gpu_api);
  }
}
