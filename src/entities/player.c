#include "entities/player.h"

int player_init(struct Player* player, struct GPUAPI* gpu_api, struct Game* game) {
  player->entity.entity_data = player;
  player->entity.delete_func = (void (*)(void*, struct GPUAPI*))player_delete;
  player->entity.update_func = (void (*)(void*, void*, float))player_update;
  player->entity.render_func = (void (*)(void*, struct GPUAPI*))player_render;

  player->character_state = CHARACTER_IDLE_STATE;
  player->character_position = VEC3_ZERO;
  player->character_direction = 1.0f;

  float draw_scale = 0.2;

  sprite_init(&player->character_shadow, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/shadow.png"));
  player->character_shadow.position = (vec3){.x = -2.3f, .y = 0.45f, .z = -0.002f};
  player->character_shadow.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Standing animation
  sprite_animation_init(&player->standing_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/standingspritesheet.png"), 4, 1.0f / 10.0f, 0);
  player->standing_animation.position = (vec3){.x = 0.0f * draw_scale, .y = 0.0f, .z = -0.01f};
  player->standing_animation.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Walking animation
  sprite_animation_init(&player->walking_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->texture_cache, "./assets/textures/walkingspritesheet.png"), 11, 1.0f / 35.0f, 0);
  player->walking_animation.position = player->standing_animation.position;
  player->walking_animation.scale = player->standing_animation.scale;
}

void player_delete(struct Player* player, struct GPUAPI* gpu_api) {
  sprite_delete(&player->character_shadow, gpu_api);
  sprite_animation_delete(&player->walking_animation, gpu_api);
  sprite_animation_delete(&player->standing_animation, gpu_api);
}

void player_update(struct Player* player, struct Game* game, float delta_time) {
  struct InputManager* input_manager = game->window->input_manager;

  // Left and right
  //float left_right_velocity = game->player_camera.camera_left_right_velocity * delta_time;
  //vec3 added_pos = vec3_scale(game->player_camera.camera.right, left_right_velocity);
  //if (fabs(game->player_camera.camera_left_right_velocity) < game->player_camera.max_camera_velocity) {
  //  game->player_camera.camera_left_right_velocity += (game->player_camera.camera.position.x - player->character_position.x) * delta_time * 10.0f;
  //}
  //game->player_camera.camera.position = vec3_add(game->player_camera.camera.position, added_pos);

  // In and out
  //float in_out_velocity = game->player_camera.camera_in_out_velocity * delta_time;
  //added_pos = vec3_scale(game->player_camera.camera.front, in_out_velocity);
  //if (input_manager->keys[GLFW_KEY_Z].state == PRESSED) {
  //  if (game->player_camera.camera_in_out_velocity < game->player_camera.max_camera_velocity)
  //    game->player_camera.camera_in_out_velocity += in_out_velocity;
  //  else
  //    game->player_camera.camera_in_out_velocity = game->player_camera.max_camera_velocity;
  //}
  //if (input_manager->keys[GLFW_KEY_X].state == PRESSED) {
  //  if (game->player_camera.camera_in_out_velocity < game->player_camera.max_camera_velocity)
  //    game->player_camera.camera_in_out_velocity -= in_out_velocity;
  //  else
  //    game->player_camera.camera_in_out_velocity = game->player_camera.max_camera_velocity;
  //}
  //game->player_camera.camera.position = vec3_add(game->player_camera.camera.position, added_pos);
  //
  //float up_down_velocity = game->player_camera.camera_up_down_velocity * delta_time;
  //added_pos = vec3_scale(game->player_camera.camera.up, up_down_velocity);
  //if (input_manager->keys[GLFW_KEY_E].state == PRESSED) {
  //  if (game->player_camera.camera_up_down_velocity < game->player_camera.max_camera_velocity)
  //    game->player_camera.camera_up_down_velocity += 0.005f;
  //}
  //if (input_manager->keys[GLFW_KEY_Q].state == PRESSED) {
  //  if (game->player_camera.camera_up_down_velocity < game->player_camera.max_camera_velocity)
  //    game->player_camera.camera_up_down_velocity -= 0.005f;
  //}
  //game->player_camera.camera.position = vec3_add(game->player_camera.camera.position, added_pos);
  float camera_mov_diff = (game->player_camera.camera.position.x - game->player->character_position.x) * 4.0f * delta_time;
  // TODO: Calculate camera velocity then use this to clamp speed
  //if (camera_mov_diff > 0.0f && camera_mov_diff > game->player_camera.max_camera_velocity)
  //  camera_mov_diff = game->player_camera.max_camera_velocity;
  //else if (camera_mov_diff < 0.0f && camera_mov_diff < -game->player_camera.max_camera_velocity)
  //  camera_mov_diff = -game->player_camera.max_camera_velocity;

  game->player_camera.camera.position.x -= camera_mov_diff;

  player->character_state = CHARACTER_IDLE_STATE;

  if (input_manager->keys[GLFW_KEY_A].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->character_direction = -1.0f;
    player->character_position.x += 0.025f;
  }
  if (input_manager->keys[GLFW_KEY_D].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->character_direction = 1.0f;
    player->character_position.x -= 0.025f;
  }

  if (input_manager->keys[GLFW_KEY_W].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->character_position.y += 0.015f;
  }
  if (input_manager->keys[GLFW_KEY_S].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->character_position.y -= 0.015f;
  }

  if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
      float left_x_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
      float left_y_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

      if (fabs(left_x_axis) < 0.25f)
        left_x_axis = 0.0f;
      else {
        player->character_state = CHARACTER_WALKING_STATE;
        player->character_direction = left_x_axis;
        player->character_position.x -= left_x_axis * 0.025f;
      }

      if (fabs(left_y_axis) < 0.25f)
        left_y_axis = 0.0f;
      else {
        player->character_state = CHARACTER_WALKING_STATE;
        player->character_position.y -= left_y_axis * 0.015f;
      }
    }
  }

  if (player->character_direction > 0.0f)
    player->character_shadow.position = (vec3){.x = player->character_position.x + 0.05f, .y = player->character_position.y - 0.65f, player->character_shadow.position.z};
  else
    player->character_shadow.position = (vec3){.x = player->character_position.x - 0.05f, .y = player->character_position.y - 0.65f, player->character_shadow.position.z};

  switch (player->character_state) {
    case (CHARACTER_IDLE_STATE):
      player->standing_animation.direction = player->character_direction;
      sprite_animation_update(&player->standing_animation, delta_time);
      player->standing_animation.position.x = player->character_position.x;
      player->standing_animation.position.y = player->character_position.y;
      break;
    case (CHARACTER_WALKING_STATE):
      player->walking_animation.direction = player->character_direction;
      sprite_animation_update(&player->walking_animation, delta_time);
      player->walking_animation.position.x = player->character_position.x;
      player->walking_animation.position.y = player->character_position.y;
      break;
  }
}

void player_render(struct Player* player, struct GPUAPI* gpu_api) {
  player_update_uniforms(player, gpu_api);

  sprite_render(&player->character_shadow, gpu_api);

  switch (player->character_state) {
    case (CHARACTER_IDLE_STATE):
      sprite_animation_render(&player->standing_animation, gpu_api);
      break;
    case (CHARACTER_WALKING_STATE):
      sprite_animation_render(&player->walking_animation, gpu_api);
      break;
  }
}

void player_update_uniforms(struct Player* player, struct GPUAPI* gpu_api) {
  sprite_update_uniforms(&player->character_shadow, gpu_api);
  sprite_animation_update_uniforms(&player->standing_animation, gpu_api);
  sprite_animation_update_uniforms(&player->walking_animation, gpu_api);
}

void player_recreate(struct Player* player, struct GPUAPI* gpu_api) {
  sprite_recreate(&player->character_shadow, gpu_api);
  sprite_animation_recreate(&player->standing_animation, gpu_api);
  sprite_animation_recreate(&player->walking_animation, gpu_api);
}
