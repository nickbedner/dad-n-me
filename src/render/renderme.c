#include "render/renderme.h"

int render_me_init(struct RenderMe* render_me, struct GPUAPI* gpu_api, struct Game* game) {
  me_init(&render_me->me, game->game_state);

  render_me->me.entity.render_func = (void (*)(void*, void*))render_me_render;

  float draw_scale = 0.2;

  sprite_init(&render_me->shadow, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/shadow.png"));
  render_me->shadow.position = (vec3){.x = -2.3f, .y = 0.45f, .z = -0.002f};
  render_me->shadow.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Standing animation
  sprite_animation_init(&render_me->standing_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/standingspritesheet.png"), 4, 1.0f / 10.0f, 0);
  render_me->standing_animation.position = (vec3){.x = 0.0f * draw_scale, .y = 0.0f, .z = -0.01f};
  render_me->standing_animation.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Walking animation
  sprite_animation_init(&render_me->walking_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/walkingspritesheet.png"), 11, 1.0f / 35.0f, 0);
  render_me->walking_animation.position = render_me->standing_animation.position;
  render_me->walking_animation.scale = render_me->standing_animation.scale;

  return 0;
}
void render_me_delete(struct RenderMe* render_me, struct GPUAPI* gpu_api) {
  me_delete(&render_me->me, NULL);

  sprite_delete(&render_me->shadow, gpu_api);
  sprite_animation_delete(&render_me->walking_animation, gpu_api);
  sprite_animation_delete(&render_me->standing_animation, gpu_api);
}

void render_me_update(struct RenderMe* render_me, struct Game* game, float delta_time) {
  me_update(&render_me->me, game->game_state, delta_time);

  struct InputManager* input_manager = game->window->input_manager;
  struct Me* me = &render_me->me;

  if (input_manager->keys[GLFW_KEY_Z].state == PRESSED)
    game->player_camera.camera.position.z += 0.01f;
  if (input_manager->keys[GLFW_KEY_X].state == PRESSED)
    game->player_camera.camera.position.z -= 0.01f;

  if (input_manager->keys[GLFW_KEY_E].state == PRESSED)
    game->player_camera.camera.position.y += 0.01f;
  if (input_manager->keys[GLFW_KEY_Q].state == PRESSED)
    game->player_camera.camera.position.y -= 0.01f;

  float camera_mov_diff = (game->player_camera.camera.position.x - me->entity.position.x) * 4.0f * delta_time;

  game->player_camera.camera.position.x -= camera_mov_diff;

  me->state = ME_IDLE_STATE;

  if (input_manager->keys[GLFW_KEY_A].state == PRESSED) {
    me->state = ME_WALKING_STATE;
    me->entity.direction = -1.0f;
    me->entity.position.x += 0.025f;
  }
  if (input_manager->keys[GLFW_KEY_D].state == PRESSED) {
    me->state = ME_WALKING_STATE;
    me->entity.direction = 1.0f;
    me->entity.position.x -= 0.025f;
  }

  if (input_manager->keys[GLFW_KEY_W].state == PRESSED) {
    me->state = ME_WALKING_STATE;
    me->entity.position.y += 0.015f;
  }
  if (input_manager->keys[GLFW_KEY_S].state == PRESSED) {
    me->state = ME_WALKING_STATE;
    me->entity.position.y -= 0.015f;
  }

  if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
      float left_x_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
      float left_y_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

      if (fabs(left_x_axis) < 0.25f)
        left_x_axis = 0.0f;
      else {
        me->state = ME_WALKING_STATE;
        me->entity.direction = left_x_axis;
        me->entity.position.x -= left_x_axis * 0.025f;
      }

      if (fabs(left_y_axis) < 0.25f)
        left_y_axis = 0.0f;
      else {
        me->state = ME_WALKING_STATE;
        me->entity.position.y -= left_y_axis * 0.015f;
      }
    }
  }

  if (me->entity.direction > 0.0f)
    render_me->shadow.position = (vec3){.x = me->entity.position.x + 0.05f, .y = me->entity.position.y - 0.65f, render_me->shadow.position.z};
  else
    render_me->shadow.position = (vec3){.x = me->entity.position.x - 0.05f, .y = me->entity.position.y - 0.65f, render_me->shadow.position.z};

  switch (me->state) {
    case (ME_IDLE_STATE):
      render_me->standing_animation.direction = me->entity.direction;
      sprite_animation_update(&render_me->standing_animation, delta_time);
      render_me->standing_animation.position.x = me->entity.position.x;
      render_me->standing_animation.position.y = me->entity.position.y;
      break;
    case (ME_WALKING_STATE):
      render_me->walking_animation.direction = me->entity.direction;
      sprite_animation_update(&render_me->walking_animation, delta_time);
      render_me->walking_animation.position.x = me->entity.position.x;
      render_me->walking_animation.position.y = me->entity.position.y;
      break;
  }
}

void render_me_render(struct RenderMe* render_me, struct GPUAPI* gpu_api) {
  render_me_update_uniforms(render_me, gpu_api);

  sprite_render(&render_me->shadow, gpu_api);

  switch (render_me->me.state) {
    case (ME_IDLE_STATE):
      sprite_animation_render(&render_me->standing_animation, gpu_api);
      break;
    case (ME_WALKING_STATE):
      sprite_animation_render(&render_me->walking_animation, gpu_api);
      break;
  }
}

void render_me_update_uniforms(struct RenderMe* render_me, struct GPUAPI* gpu_api) {
  sprite_update_uniforms(&render_me->shadow, gpu_api);
  sprite_animation_update_uniforms(&render_me->standing_animation, gpu_api);
  sprite_animation_update_uniforms(&render_me->walking_animation, gpu_api);
}

void render_me_recreate(struct RenderMe* render_me, struct GPUAPI* gpu_api) {
  sprite_recreate(&render_me->shadow, gpu_api);
  sprite_animation_recreate(&render_me->standing_animation, gpu_api);
  sprite_animation_recreate(&render_me->walking_animation, gpu_api);
}