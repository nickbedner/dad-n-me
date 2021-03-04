#include "render/renderwilbur.h"

int render_wilbur_init(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api, struct Game* game) {
  wilbur_init(&render_wilbur->wilbur, game->game_state);

  render_wilbur->wilbur.entity.entity_data = render_wilbur;
  render_wilbur->wilbur.entity.delete_func = (void (*)(void*, void*))render_wilbur_delete;
  render_wilbur->wilbur.entity.update_func = (void (*)(void*, void*, float))render_wilbur_update;
  render_wilbur->wilbur.entity.render_func = (void (*)(void*, void*))render_wilbur_render;
  render_wilbur->wilbur.entity.recreate_func = (void (*)(void*, void*))render_wilbur_recreate;

  float draw_scale = 0.2;

  render_wilbur->wilbur.entity.direction = 1.0f;

  sprite_init(&render_wilbur->shadow, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/characters/shadow.png"));
  render_wilbur->shadow.position = (vec3){.x = -2.3f, .y = 0.45f, .z = -0.002f};
  render_wilbur->shadow.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Standing animation
  sprite_animation_init(&render_wilbur->standing_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/characters/wilbur/wilburstandingspritesheet.png"), 1, 1.0f / 10.0f, 0);
  render_wilbur->standing_animation.position = (vec3){.x = 0.0f * draw_scale, .y = 0.0f, .z = -0.01f};
  render_wilbur->standing_animation.scale = (vec3){.x = draw_scale, .y = draw_scale, .z = draw_scale};

  // Walking animation
  sprite_animation_init(&render_wilbur->walking_animation, gpu_api, &game->sprite_animation_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, "./assets/textures/characters/wilbur/wilburwalkingspritesheet.png"), 8, 1.0f / 30.0f, 0);
  render_wilbur->walking_animation.position = render_wilbur->standing_animation.position;
  render_wilbur->walking_animation.scale = render_wilbur->standing_animation.scale;

  render_wilbur->wilbur.entity.width = render_wilbur->standing_animation.width * draw_scale;
  render_wilbur->wilbur.entity.height = render_wilbur->standing_animation.height * draw_scale;

  return 0;
}
void render_wilbur_delete(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api) {
  wilbur_delete(&render_wilbur->wilbur, NULL);

  sprite_delete(&render_wilbur->shadow, gpu_api);
  sprite_animation_delete(&render_wilbur->walking_animation, gpu_api);
  sprite_animation_delete(&render_wilbur->standing_animation, gpu_api);
}

void render_wilbur_update(struct RenderWilbur* render_wilbur, struct Game* game, float delta_time) {
  wilbur_update(&render_wilbur->wilbur, game->game_state, delta_time);

  struct InputManager* input_manager = game->window->input_manager;
  struct Wilbur* wilbur = &render_wilbur->wilbur;

  wilbur->state = WILBUR_IDLE_STATE;

  if (input_manager->keys[GLFW_KEY_A].state == PRESSED && input_manager->keys[GLFW_KEY_D].state == PRESSED)
    wilbur->state = WILBUR_IDLE_STATE;
  else {
    if (input_manager->keys[GLFW_KEY_A].state == PRESSED) {
      wilbur->state = WILBUR_WALKING_STATE;
      wilbur->entity.direction = -1.0f;
      wilbur->entity.position.x -= 3.5f * delta_time;
    }
    if (input_manager->keys[GLFW_KEY_D].state == PRESSED) {
      wilbur->state = WILBUR_WALKING_STATE;
      wilbur->entity.direction = 1.0f;
      wilbur->entity.position.x += 3.5f * delta_time;
    }
  }

  if (input_manager->keys[GLFW_KEY_W].state == PRESSED && input_manager->keys[GLFW_KEY_S].state == PRESSED &&
      input_manager->keys[GLFW_KEY_A].state != PRESSED && input_manager->keys[GLFW_KEY_D].state != PRESSED)
    wilbur->state = WILBUR_IDLE_STATE;
  else {
    if (input_manager->keys[GLFW_KEY_W].state == PRESSED) {
      wilbur->state = WILBUR_WALKING_STATE;
      wilbur->entity.position.y += 2.0f * delta_time;
    }
    if (input_manager->keys[GLFW_KEY_S].state == PRESSED) {
      wilbur->state = WILBUR_WALKING_STATE;
      wilbur->entity.position.y -= 2.0f * delta_time;
    }
  }

  if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
      float left_x_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
      float left_y_axis = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

      if (fabs(left_x_axis) < 0.25f)
        left_x_axis = 0.0f;
      else {
        wilbur->state = WILBUR_WALKING_STATE;
        wilbur->entity.direction = left_x_axis;
        wilbur->entity.position.x += left_x_axis * 3.5f * delta_time;
      }

      if (fabs(left_y_axis) < 0.25f)
        left_y_axis = 0.0f;
      else {
        wilbur->state = WILBUR_WALKING_STATE;
        wilbur->entity.position.y -= left_y_axis * 2.0f * delta_time;
      }
    }
  }

  // TODO: Yucky hardcoded bounds
  if (wilbur->entity.position.y > 0.5)
    wilbur->entity.position.y = 0.5f;
  else if (wilbur->entity.position.y < -2.7)
    wilbur->entity.position.y = -2.7f;

  if (wilbur->entity.direction > 0.0f)
    render_wilbur->shadow.position = (vec3){.x = wilbur->entity.position.x + 0.05f, .y = wilbur->entity.position.y - 0.65f, render_wilbur->shadow.position.z};
  else
    render_wilbur->shadow.position = (vec3){.x = wilbur->entity.position.x - 0.05f, .y = wilbur->entity.position.y - 0.65f, render_wilbur->shadow.position.z};

  switch (wilbur->state) {
    case (WILBUR_IDLE_STATE):
      render_wilbur->standing_animation.direction = wilbur->entity.direction;
      sprite_animation_update(&render_wilbur->standing_animation, delta_time);
      render_wilbur->standing_animation.position.x = wilbur->entity.position.x;
      render_wilbur->standing_animation.position.y = wilbur->entity.position.y;
      break;
    case (WILBUR_WALKING_STATE):
      render_wilbur->walking_animation.direction = wilbur->entity.direction;
      sprite_animation_update(&render_wilbur->walking_animation, delta_time);
      render_wilbur->walking_animation.position.x = wilbur->entity.position.x;
      render_wilbur->walking_animation.position.y = wilbur->entity.position.y;
      break;
  }
}

void render_wilbur_render(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api) {
  render_wilbur_update_uniforms(render_wilbur, gpu_api);

  sprite_render(&render_wilbur->shadow, gpu_api);

  switch (render_wilbur->wilbur.state) {
    case (WILBUR_IDLE_STATE):
      sprite_animation_render(&render_wilbur->standing_animation, gpu_api);
      break;
    case (WILBUR_WALKING_STATE):
      sprite_animation_render(&render_wilbur->walking_animation, gpu_api);
      break;
  }
}

void render_wilbur_update_uniforms(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api) {
  sprite_update_uniforms(&render_wilbur->shadow, gpu_api);
  sprite_animation_update_uniforms(&render_wilbur->standing_animation, gpu_api);
  sprite_animation_update_uniforms(&render_wilbur->walking_animation, gpu_api);
}

void render_wilbur_recreate(struct RenderWilbur* render_wilbur, struct GPUAPI* gpu_api) {
  sprite_recreate(&render_wilbur->shadow, gpu_api);
  sprite_animation_recreate(&render_wilbur->standing_animation, gpu_api);
  sprite_animation_recreate(&render_wilbur->walking_animation, gpu_api);
}