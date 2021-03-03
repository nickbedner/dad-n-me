#include "game.h"

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  game->window = window;

  dad_n_me_server_init(&game->dad_n_me_server, DAD_N_ME_SERVER_EMULATE);

  resource_manager_init(&game->resource_manager, gpu_api);
  struct ResourceManager* resource_manager = &game->resource_manager;

  audio_clip_init(&game->music_clip, resource_manager->music_clip_cache, MUSIC_AUDIO_CLIP, 1, 0.75f, 0.025f);
  audio_manager_play_audio_clip(&resource_manager->audio_manager, &game->music_clip);

  fxaa_shader_init(&game->fxaa_shader, gpu_api);
  game->fxaa_shader.on = 0;
  sprite_shader_init(&game->sprite_shader, gpu_api, 0);
  sprite_animation_shader_init(&game->sprite_animation_shader, gpu_api, 0);

  player_camera_init(&game->player_camera);

  // Get current game state from server
  game->game_state = calloc(1, sizeof(struct GameState));
  game_state_init(game->game_state);

  // TODO: Look into x flip
  game->render_me = calloc(1, sizeof(struct RenderMe));
  render_me_init(game->render_me, gpu_api, game);
  game->render_me->me.entity.position = (vec3){.x = -1.5f, .y = -0.5, .z = 0.0f};

  game->render_wilbur = calloc(1, sizeof(struct RenderWilbur));
  render_wilbur_init(game->render_wilbur, gpu_api, game);
  game->render_wilbur->wilbur.entity.position = (vec3){.x = 1.0f, .y = -0.75, .z = 0.0f};
  game->player_camera.camera.position.x = game->render_wilbur->wilbur.entity.position.x;
  game->player_camera.camera.position.y = game->render_wilbur->wilbur.entity.position.y;

  array_list_init(&game->stage_entity_render_list);
  array_list_add(&game->stage_entity_render_list, &game->render_me->me.entity);
  array_list_add(&game->stage_entity_render_list, &game->render_wilbur->wilbur.entity);

  array_list_init(&game->scenery_render_list);
  for (int scenery_num = 0; scenery_num < array_list_size(&game->game_state->game_stage->scenery_entities); scenery_num++) {
    struct Scenery* scenery = array_list_get(&game->game_state->game_stage->scenery_entities, scenery_num);
    for (int repeat_num = 0; repeat_num < scenery->repeat_factor; repeat_num++) {
      struct RenderScenery* new_scenery = calloc(1, sizeof(struct RenderScenery));
      render_scenery_init(new_scenery, gpu_api, game, scenery);
      new_scenery->scenery.entity.position.x = new_scenery->scenery.entity.position.x + ((new_scenery->texture.width * new_scenery->texture.scale.x) * repeat_num) * 0.99f;
      array_list_add(&game->scenery_render_list, new_scenery);
    }
  }
}

void game_delete(struct Game* game, struct Mana* mana) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  struct ResourceManager* resource_manager = &game->resource_manager;
  // Wait for command buffers to finish before deleting desciptor sets
  vkWaitForFences(gpu_api->vulkan_state->device, 2, gpu_api->vulkan_state->swap_chain->in_flight_fences, VK_TRUE, UINT64_MAX);

  texture_cache_delete(&resource_manager->texture_cache, gpu_api);

  for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
    struct RenderScenery* entity = array_list_get(&game->scenery_render_list, entity_num);
    render_scenery_delete(entity, gpu_api);
    free(entity);
  }

  for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
    struct Entity* entity = array_list_get(&game->stage_entity_render_list, entity_num);
    (*entity->delete_func)(entity->entity_data, gpu_api);
  }

  free(game->render_me);
  free(game->render_wilbur);

  array_list_delete(&game->scenery_render_list);
  array_list_delete(&game->stage_entity_render_list);

  game_state_delete(game->game_state);
  free(game->game_state);

  sprite_animation_shader_delete(&game->sprite_animation_shader, gpu_api);
  sprite_shader_delete(&game->sprite_shader, gpu_api);

  fxaa_shader_delete(&game->fxaa_shader, gpu_api);

  resource_manager_delete(&game->resource_manager);
}

void game_update(struct Game* game, struct Mana* mana, double delta_time) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  // When the window is resized everything must be recreated in vulkan
  if (mana->engine.gpu_api.vulkan_state->reset_shaders) {
    mana->engine.gpu_api.vulkan_state->reset_shaders = 0;
    vkDeviceWaitIdle(mana->engine.gpu_api.vulkan_state->device);

    fxaa_shader_delete(&game->fxaa_shader, gpu_api);
    fxaa_shader_init(&game->fxaa_shader, gpu_api);
    game->fxaa_shader.on = 0;

    sprite_shader_delete(&game->sprite_shader, gpu_api);
    sprite_shader_init(&game->sprite_shader, gpu_api, 0);

    sprite_animation_shader_delete(&game->sprite_animation_shader, gpu_api);
    sprite_animation_shader_init(&game->sprite_animation_shader, gpu_api, 0);

    for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
      struct Entity* entity = array_list_get(&game->stage_entity_render_list, entity_num);
      (*entity->recreate_func)(entity->entity_data, gpu_api);
    }

    for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
      struct Entity* entity = array_list_get(&game->scenery_render_list, entity_num);
      (*entity->recreate_func)(entity->entity_data, gpu_api);
    }
  }

  // TODO: Switch to job system
  player_camera_update(&game->player_camera, delta_time);
  game_update_input(game, &mana->engine);

  gpu_api->vulkan_state->gbuffer->projection_matrix = camera_get_projection_matrix(&game->player_camera.camera, game->window);
  gpu_api->vulkan_state->gbuffer->view_matrix = camera_get_view_matrix(&game->player_camera.camera);

  gbuffer_start(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  // Draw order sorting
  for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
    for (int other_entity_num = entity_num; other_entity_num > 0; other_entity_num--) {
      struct Entity* entity_one = ((struct Entity*)array_list_get(&game->stage_entity_render_list, other_entity_num));
      struct Entity* entity_two = ((struct Entity*)array_list_get(&game->stage_entity_render_list, other_entity_num - 1));
      if (entity_one->position.y - entity_one->height / 2.0f < entity_two->position.y - entity_two->height / 2.0f)
        continue;
      array_list_swap(&game->stage_entity_render_list, other_entity_num, other_entity_num - 1);
    }
  }
  // TODO: Implement instanced rendering and texture
  printf("OMP threads: %d\n", engine_get_max_omp_threads());
  // Render and update scenery
  for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
    struct Entity* entity = array_list_get(&game->scenery_render_list, entity_num);
    (*entity->update_func)(entity->entity_data, game, delta_time);
    (*entity->render_func)(entity->entity_data, gpu_api);
  }
  // Render and update sprites
  for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
    struct Entity* entity = array_list_get(&game->stage_entity_render_list, entity_num);
    (*entity->update_func)(entity->entity_data, game, delta_time);
    (*entity->render_func)(entity->entity_data, gpu_api);
  }

  gbuffer_stop(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);

  blit_post_process_render(gpu_api->vulkan_state->post_process->blit_post_process, gpu_api);
  fxaa_shader_render(&game->fxaa_shader, gpu_api);
  blit_swap_chain_render(gpu_api->vulkan_state->swap_chain->blit_swap_chain, gpu_api);
}

void game_update_input(struct Game* game, struct Engine* engine) {
  int in_focus = glfwGetWindowAttrib(engine->graphics_library.glfw_library.glfw_window, GLFW_FOCUSED);
  if (in_focus == 0)
    return;

  struct InputManager* input_manager = game->window->input_manager;

  if (input_manager->keys[GLFW_KEY_ESCAPE].state == PRESSED)
    glfwSetWindowShouldClose(engine->graphics_library.glfw_library.glfw_window, 1);

  if (input_manager->keys[GLFW_KEY_1].pushed == 1) {
    game->fxaa_shader.on ^= 1;
    game->fxaa_shader.on ? printf("FXAA ON\n") : printf("FXAA OFF\n");
  }

  if (input_manager->keys[GLFW_KEY_O].pushed == 1) {
    game->resource_manager.audio_manager.master_volume -= 0.1f;
    if (game->resource_manager.audio_manager.master_volume < 0.0f)
      game->resource_manager.audio_manager.master_volume = 0.0f;
  }
  if (input_manager->keys[GLFW_KEY_P].pushed == 1) {
    game->resource_manager.audio_manager.master_volume += 0.1f;
    if (game->resource_manager.audio_manager.master_volume > 1.0f)
      game->resource_manager.audio_manager.master_volume = 1.0f;
  }
}
