#include "render/renderplayer.h"

int render_player_init(struct RenderPlayer* render_player, struct GPUAPI* gpu_api, struct Game* game) {
  player_init(&render_player->player, game->game_state);

  render_player->player.entity.render_func = (void (*)(void*, void*))render_player_render;

  float draw_scale = 0.2;

  sprite_init(&render_player->character_shadow, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/shadow.png"));
  render_player->character_shadow.position = (vec3){.x = -2.3f, .y = 0.45f, .z = -0.002f};
  render_player->character_shadow.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Standing animation
  sprite_animation_init(&render_player->standing_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/standingspritesheet.png"), 4, 1.0f / 10.0f, 0);
  render_player->standing_animation.position = (vec3){.x = 0.0f * draw_scale, .y = 0.0f, .z = -0.01f};
  render_player->standing_animation.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Walking animation
  sprite_animation_init(&render_player->walking_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/walkingspritesheet.png"), 11, 1.0f / 35.0f, 0);
  render_player->walking_animation.position = render_player->standing_animation.position;
  render_player->walking_animation.scale = render_player->standing_animation.scale;

  return 0;
}
void render_player_delete(struct RenderPlayer* render_player, struct GPUAPI* gpu_api) {
  player_delete(&render_player->player, NULL);

  sprite_delete(&render_player->character_shadow, gpu_api);
  sprite_animation_delete(&render_player->walking_animation, gpu_api);
  sprite_animation_delete(&render_player->standing_animation, gpu_api);
}

void render_player_update(struct RenderPlayer* render_player, struct Game* game, float delta_time) {
  player_update(&render_player->player, game->game_state, delta_time);

  struct InputManager* input_manager = game->window->input_manager;
  struct Player* player = &render_player->player;

  if (input_manager->keys[GLFW_KEY_Z].state == PRESSED)
    game->player_camera.camera.position.z += 0.01f;
  if (input_manager->keys[GLFW_KEY_X].state == PRESSED)
    game->player_camera.camera.position.z -= 0.01f;

  if (input_manager->keys[GLFW_KEY_E].state == PRESSED)
    game->player_camera.camera.position.y += 0.01f;
  if (input_manager->keys[GLFW_KEY_Q].state == PRESSED)
    game->player_camera.camera.position.y -= 0.01f;

  float camera_mov_diff = (game->player_camera.camera.position.x - render_player->player.entity.position.x) * 4.0f * delta_time;

  game->player_camera.camera.position.x -= camera_mov_diff;

  player->character_state = CHARACTER_IDLE_STATE;

  if (input_manager->keys[GLFW_KEY_A].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->character_direction = -1.0f;
    player->entity.position.x += 0.025f;
  }
  if (input_manager->keys[GLFW_KEY_D].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->character_direction = 1.0f;
    player->entity.position.x -= 0.025f;
  }

  if (input_manager->keys[GLFW_KEY_W].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->entity.position.y += 0.015f;
  }
  if (input_manager->keys[GLFW_KEY_S].state == PRESSED) {
    player->character_state = CHARACTER_WALKING_STATE;
    player->entity.position.y -= 0.015f;
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
        player->entity.position.x -= left_x_axis * 0.025f;
      }

      if (fabs(left_y_axis) < 0.25f)
        left_y_axis = 0.0f;
      else {
        player->character_state = CHARACTER_WALKING_STATE;
        player->entity.position.y -= left_y_axis * 0.015f;
      }
    }
  }

  if (player->character_direction > 0.0f)
    render_player->character_shadow.position = (vec3){.x = player->entity.position.x + 0.05f, .y = player->entity.position.y - 0.65f, render_player->character_shadow.position.z};
  else
    render_player->character_shadow.position = (vec3){.x = player->entity.position.x - 0.05f, .y = player->entity.position.y - 0.65f, render_player->character_shadow.position.z};

  switch (player->character_state) {
    case (CHARACTER_IDLE_STATE):
      render_player->standing_animation.direction = player->character_direction;
      sprite_animation_update(&render_player->standing_animation, delta_time);
      render_player->standing_animation.position.x = player->entity.position.x;
      render_player->standing_animation.position.y = player->entity.position.y;
      break;
    case (CHARACTER_WALKING_STATE):
      render_player->walking_animation.direction = player->character_direction;
      sprite_animation_update(&render_player->walking_animation, delta_time);
      render_player->walking_animation.position.x = player->entity.position.x;
      render_player->walking_animation.position.y = player->entity.position.y;
      break;
  }
}

void render_player_render(struct RenderPlayer* render_player, struct GPUAPI* gpu_api) {
  render_player_update_uniforms(render_player, gpu_api);

  sprite_render(&render_player->character_shadow, gpu_api);

  switch (render_player->player.character_state) {
    case (CHARACTER_IDLE_STATE):
      sprite_animation_render(&render_player->standing_animation, gpu_api);
      break;
    case (CHARACTER_WALKING_STATE):
      sprite_animation_render(&render_player->walking_animation, gpu_api);
      break;
  }
}

void render_player_update_uniforms(struct RenderPlayer* render_player, struct GPUAPI* gpu_api) {
  sprite_update_uniforms(&render_player->character_shadow, gpu_api);
  sprite_animation_update_uniforms(&render_player->standing_animation, gpu_api);
  sprite_animation_update_uniforms(&render_player->walking_animation, gpu_api);
}

void render_player_recreate(struct RenderPlayer* render_player, struct GPUAPI* gpu_api) {
  sprite_recreate(&render_player->character_shadow, gpu_api);
  sprite_animation_recreate(&render_player->standing_animation, gpu_api);
  sprite_animation_recreate(&render_player->walking_animation, gpu_api);
}