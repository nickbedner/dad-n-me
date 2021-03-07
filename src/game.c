#include "game.h"

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  game->window = window;

  // Connect to server or start local server
  dad_n_me_server_init(&game->dad_n_me_server, DAD_N_ME_SERVER_EMULATE);

  // Load game assets
  resource_manager_init(&game->resource_manager, gpu_api);
  struct ResourceManager* resource_manager = &game->resource_manager;

  // Start job system for multithreading, on standby
  game->job_system = calloc(1, sizeof(struct JobSystem));
  job_system_init(game->job_system, game);

  // Play game music
  audio_clip_init(&game->music_clip, resource_manager->music_clip_cache, MUSIC_AUDIO_CLIP, 1, 0.75f, 0.025f);
  audio_manager_play_audio_clip(&resource_manager->audio_manager, &game->music_clip);

  // Creat FXAA shader
  fxaa_shader_init(&game->fxaa_shader, gpu_api);
  game->fxaa_shader.on = 0;

  // Create static and dynamic sprite shaders
  sprite_shader_init(&game->sprite_shader, gpu_api, 0);
  sprite_animation_shader_init(&game->sprite_animation_shader, gpu_api, 0);

  // Create camera to follow entities
  player_camera_init(&game->player_camera);

  //game->render_me = calloc(1, sizeof(struct RenderMe));
  //render_me_init(game->render_me, gpu_api, game);
  //game->render_me->me.entity.position = (vec3){.x = -3.5f, .y = -1.125, .z = 0.0f};

  //game->render_wilbur = calloc(1, sizeof(struct RenderWilbur));
  //render_wilbur_init(game->render_wilbur, gpu_api, game);
  //game->render_wilbur->wilbur.entity.position = (vec3){.x = 0.0f, .y = -0.5, .z = 0.0f};
  //game->player_camera.focus_entity = &game->render_wilbur->wilbur.entity;
  //game->player_camera.camera.position.x = game->render_wilbur->wilbur.entity.position.x;
  //game->player_camera.camera.position.y = game->render_wilbur->wilbur.entity.position.y;

  //array_list_init(&game->stage_entity_render_list);
  //array_list_add(&game->stage_entity_render_list, &game->render_me->me.entity);
  //array_list_add(&game->stage_entity_render_list, &game->render_wilbur->wilbur.entity);

  // Init ecs
  vector_init(&game->entities, sizeof(struct Entity));
  component_registry_init(&game->position_registry, sizeof(struct Position));
  component_registry_init(&game->dimensions_registry, sizeof(struct Dimensions));
  component_registry_init(&game->scenery_registry, sizeof(struct Scenery));
  component_registry_init(&game->render_registry, sizeof(struct Render));

  // Load stage scenery
  game_hotswap_scenery(game, gpu_api);

  game->resource_manager.audio_manager.master_volume = 0.0f;
}

void game_delete(struct Game* game, struct Mana* mana) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  // Wait for command buffers to finish before deleting desciptor sets
  vkWaitForFences(gpu_api->vulkan_state->device, 2, gpu_api->vulkan_state->swap_chain->in_flight_fences, VK_TRUE, UINT64_MAX);

  // Free sprite data allocated on gpu
  char* render_list_key = NULL;
  struct MapIter render_list_iter = map_iter();
  while ((render_list_key = map_next(&game->render_registry.registry, &render_list_iter)))
    sprite_delete(&((struct Render*)map_get(&game->render_registry.registry, render_list_key))->sprite, gpu_api);

  // Delete ecs
  vector_delete(&game->entities);
  component_registry_delete(&game->position_registry);
  component_registry_delete(&game->dimensions_registry);
  component_registry_delete(&game->render_registry);
  component_registry_delete(&game->scenery_registry);

  // Delete shaders
  sprite_animation_shader_delete(&game->sprite_animation_shader, gpu_api);
  sprite_shader_delete(&game->sprite_shader, gpu_api);
  fxaa_shader_delete(&game->fxaa_shader, gpu_api);

  // Delete and free job system
  job_system_delete(game->job_system);
  free(game->job_system);

  // Delete resources
  resource_manager_delete(&game->resource_manager, gpu_api);
}

void game_update(struct Game* game, struct Mana* mana, double delta_time) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  // Note: When the window is resized everything must be recreated in vulkan
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

    /*for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
      struct Entity* entity = array_list_get(&game->stage_entity_render_list, entity_num);
      (*entity->recreate_func)(entity->entity_data, gpu_api);
    }*/
  }

  // Update camera and user input
  player_camera_update(&game->player_camera, delta_time);
  game_update_input(game, &mana->engine);

  // TODO: Make this less messy with inlined function
  // Update scenery
  /*struct EntityUpdateData* scenery_update_data_pool = malloc(sizeof(struct EntityUpdateData) * array_list_size(&game->scenery_render_list));
  struct Job* scenery_update_job_pool = malloc(sizeof(struct Job) * array_list_size(&game->scenery_render_list));
  for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
    struct Entity* entity = array_list_get(&game->scenery_render_list, entity_num);

    scenery_update_data_pool[entity_num] = (struct EntityUpdateData){.game_handle = game, .entity_handle = entity, .delta_time = delta_time};
    scenery_update_job_pool[entity_num] = (struct Job){.job_func = entity_update_job, .job_data = &scenery_update_data_pool[entity_num]};
    job_system_enqueue(game->job_system, &scenery_update_job_pool[entity_num]);
  }
  // Update sprites
  struct EntityUpdateData* sprites_update_data_pool = malloc(sizeof(struct EntityUpdateData) * array_list_size(&game->stage_entity_render_list));
  struct Job* sprites_update_job_pool = malloc(sizeof(struct Job) * array_list_size(&game->stage_entity_render_list));
  for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
    struct Entity* entity = array_list_get(&game->stage_entity_render_list, entity_num);

    sprites_update_data_pool[entity_num] = (struct EntityUpdateData){.game_handle = game, .entity_handle = entity, .delta_time = delta_time};
    sprites_update_job_pool[entity_num] = (struct Job){.job_func = entity_update_job, .job_data = &sprites_update_data_pool[entity_num]};
    job_system_enqueue(game->job_system, &sprites_update_job_pool[entity_num]);
  }
  job_system_start_threads(game->job_system);
  job_system_wait(game->job_system);

  free(scenery_update_data_pool);
  free(scenery_update_job_pool);
  free(sprites_update_data_pool);
  free(sprites_update_job_pool);*/

  // Build and sort list of entities to render
  struct ArrayList sorted_render_list = {0};
  array_list_init(&sorted_render_list);

  char* sorted_render_list_key = NULL;
  struct MapIter sorted_render_list_iter = map_iter();
  // Build list of entities that need to be rendered
  while ((sorted_render_list_key = map_next(&game->render_registry.registry, &sorted_render_list_iter)))
    array_list_add(&sorted_render_list, sorted_render_list_key);

  // TODO: Look into multithreaded merge sort
  // Sort render list for draw order
  for (int render_num = 0; render_num < array_list_size(&sorted_render_list); render_num++) {
    for (int other_render_num = render_num; other_render_num > 0; other_render_num--) {
      struct Position* position_one = component_registry_get(&game->position_registry, array_list_get(&sorted_render_list, other_render_num));
      struct Position* position_two = component_registry_get(&game->position_registry, array_list_get(&sorted_render_list, other_render_num - 1));

      // TODO: Check within float range
      if (position_one->z > position_two->z)
        continue;

      // TODO: Add if within z range then check y range/implicit function
      //if (entity_one->position.y - entity_one->height / 2.0f < entity_two->position.y - entity_two->height / 2.0f)
      //  continue;

      array_list_swap(&sorted_render_list, other_render_num, other_render_num - 1);
    }
  }

  // Update projection and view matrices
  gpu_api->vulkan_state->gbuffer->projection_matrix = camera_get_projection_matrix(&game->player_camera.camera, game->window);
  gpu_api->vulkan_state->gbuffer->view_matrix = camera_get_view_matrix(&game->player_camera.camera);

  // TODO: Implement instanced rendering and texture atlas
  // Start blitting to the gbuffer
  gbuffer_start(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  for (int render_num = 0; render_num < array_list_size(&sorted_render_list); render_num++) {
    char* entity_id = array_list_get(&sorted_render_list, render_num);
    struct Render* next_render = component_registry_get(&game->render_registry, entity_id);
    sprite_update_uniforms(&next_render->sprite, gpu_api);
    sprite_render(&next_render->sprite, gpu_api);
  }
  gbuffer_stop(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);

  array_list_delete(&sorted_render_list);

  // Blit gbuffer to post process ping pong
  blit_post_process_render(gpu_api->vulkan_state->post_process->blit_post_process, gpu_api);
  fxaa_shader_render(&game->fxaa_shader, gpu_api);
  // Blit post process to swap chain
  blit_swap_chain_render(gpu_api->vulkan_state->swap_chain->blit_swap_chain, gpu_api);
}

void game_update_input(struct Game* game, struct Engine* engine) {
  // If the windows is not in focues, ignore user input
  if (glfwGetWindowAttrib(engine->graphics_library.glfw_library.glfw_window, GLFW_FOCUSED) == 0)
    return;

  struct InputManager* input_manager = game->window->input_manager;

  // Exit game
  if (input_manager->keys[GLFW_KEY_ESCAPE].state == PRESSED)
    glfwSetWindowShouldClose(engine->graphics_library.glfw_library.glfw_window, 1);

  // Toggle FXAA
  if (input_manager->keys[GLFW_KEY_1].pushed == 1)
    (game->fxaa_shader.on ^= 1) ? printf("FXAA ON\n") : printf("FXAA OFF\n");

  // Reload scenery from xml
  if (input_manager->keys[GLFW_KEY_2].pushed == 1)
    game_hotswap_scenery(game, &engine->gpu_api);

  //if (input_manager->keys[GLFW_KEY_3].pushed == 1)
  //  game->player_camera.focus_entity = &game->render_me->me.entity;
  //
  //if (input_manager->keys[GLFW_KEY_4].pushed == 1)
  //  game->player_camera.focus_entity = &game->render_wilbur->wilbur.entity;

  // Change master volume levels
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

  // Zoom camera in/out
  if (input_manager->keys[GLFW_KEY_Z].state == PRESSED)
    game->player_camera.camera.position.z += 0.01f;
  if (input_manager->keys[GLFW_KEY_X].state == PRESSED)
    game->player_camera.camera.position.z -= 0.01f;

  // Move camera up/down
  if (input_manager->keys[GLFW_KEY_E].state == PRESSED)
    game->player_camera.camera.position.y += 0.01f;
  if (input_manager->keys[GLFW_KEY_Q].state == PRESSED)
    game->player_camera.camera.position.y -= 0.01f;
}
