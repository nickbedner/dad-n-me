#include "game.h"

static inline void game_hotswap_scenery(struct Game* game, struct GPUAPI* gpu_api) {
  // Delete scenery, position, entity
  //for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
  //  struct RenderScenery* entity = array_list_get(&game->scenery_render_list, entity_num);
  //  render_scenery_delete(entity, gpu_api);
  //  free(entity);
  //}

  //array_list_clear(&game->scenery_render_list);

  //if (game->game_state != NULL) {
  //  game_state_delete(game->game_state);
  //  free(game->game_state);
  //}
  //game->game_state = calloc(1, sizeof(struct GameState));
  //game_state_init(game->game_state);

  struct XmlNode* game_stage_node = xml_parser_load_xml_file("./assets/stages/gamestage.xml");
  struct XmlNode* scenery_node = xml_node_get_child(game_stage_node, "scenery");
  const char* scenery_list_key = NULL;
  struct MapIter scenery_list_iter = map_iter();

  struct SceneryBucket {
    struct Scenery* scenery[64];
    struct XmlNode* node[64];
  } texture_settings_bucket = {0};

  int total_scenery_num = 0;
  while ((scenery_list_key = map_next(scenery_node->child_nodes, &scenery_list_iter)))
    texture_settings_bucket.node[total_scenery_num++] = array_list_get(*((struct ArrayList**)map_get(scenery_node->child_nodes, scenery_list_key)), 0);

  //#pragma omp parallel for schedule(dynamic)
  for (int scenery_num = 0; scenery_num < total_scenery_num; scenery_num++) {
    texture_settings_bucket.scenery[scenery_num] = calloc(1, sizeof(struct Scenery));
    struct XmlNode* position_node = xml_node_get_child(texture_settings_bucket.node[scenery_num], "position");
    float scale = atof(xml_node_get_data(xml_node_get_child(texture_settings_bucket.node[scenery_num], "scale")));
    int repeat_factor = atoi(xml_node_get_data(xml_node_get_child(texture_settings_bucket.node[scenery_num], "repeat")));
    float offset = atof(xml_node_get_data(xml_node_get_child(texture_settings_bucket.node[scenery_num], "offset")));

    for (int repeat_num = 0; repeat_num < repeat_factor; repeat_num++) {
      struct Entity new_scenery_entity = {0};
      entity_init(&new_scenery_entity);
      vector_push_back(&game->entities, &new_scenery_entity);

      struct Scenery new_scenery = (struct Scenery){.texture_path = xml_node_get_attribute(position_node, "path"), .repeat_factor = repeat_factor, .offset = offset};
      component_registry_set(&game->scenery_registry, &new_scenery_entity, &new_scenery);

      struct Render new_scenery_render = {0};
      sprite_init(&new_scenery_render.sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, new_scenery.texture_path));

      struct Position new_scenery_position = {0};
      new_scenery_position.scale = scale;
      new_scenery_position.width = new_scenery_render.sprite.width * scale;
      new_scenery_position.height = new_scenery_render.sprite.height * scale;
      new_scenery_position.position.x = atof(xml_node_get_data(xml_node_get_child(position_node, "x"))) + (new_scenery_position.width * repeat_num) * new_scenery.offset;
      new_scenery_position.position.y = atof(xml_node_get_data(xml_node_get_child(position_node, "y")));
      new_scenery_position.position.z = atof(xml_node_get_data(xml_node_get_child(position_node, "z")));
      component_registry_set(&game->position_registry, &new_scenery_entity, &new_scenery_position);

      new_scenery_render.sprite.position = new_scenery_position.position;
      new_scenery_render.sprite.scale = (vec3){.x = scale, .y = scale, .z = scale};
      component_registry_set(&game->render_registry, &new_scenery_entity, &new_scenery_render);
    }
  }

  xml_parser_delete(game_stage_node);

  // Note: Could probably use a temp arraylist for speed?
  // Sort scenery by z position
  for (int render_num = 0; render_num < vector_size(&game->entities); render_num++) {
    for (int other_render_num = render_num; other_render_num > 0; other_render_num--) {
      struct Entity* entity_one = ((struct Entity*)vector_get(&game->entities, other_render_num));
      struct Entity* entity_two = ((struct Entity*)vector_get(&game->entities, other_render_num - 1));

      struct Position* position_one = component_registry_get(&game->position_registry, entity_one->entity_id);
      struct Position* position_two = component_registry_get(&game->position_registry, entity_two->entity_id);

      if (position_one->position.z > position_two->position.z)
        continue;

      struct Entity swap = *((struct Entity*)vector_get(&game->entities, other_render_num));
      vector_set(&game->entities, other_render_num, vector_get(&game->entities, other_render_num - 1));
      vector_set(&game->entities, other_render_num - 1, &swap);
    }
  }
}

/*static inline void game_hotswap_scenery(struct Game* game, struct GPUAPI* gpu_api) {
  // Delete scenery, position, entity
  //for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
  //  struct RenderScenery* entity = array_list_get(&game->scenery_render_list, entity_num);
  //  render_scenery_delete(entity, gpu_api);
  //  free(entity);
  //}

  //array_list_clear(&game->scenery_render_list);

  //if (game->game_state != NULL) {
  //  game_state_delete(game->game_state);
  //  free(game->game_state);
  //}
  //game->game_state = calloc(1, sizeof(struct GameState));
  //game_state_init(game->game_state);

  struct XmlNode* game_stage_node = xml_parser_load_xml_file("./assets/stages/gamestage.xml");
  struct XmlNode* scenery_node = xml_node_get_child(game_stage_node, "scenery");
  const char* scenery_list_key = NULL;
  struct MapIter scenery_list_iter = map_iter();

  struct SceneryBucket {
    struct Scenery* scenery[64];
    struct XmlNode* node[64];
  } texture_settings_bucket = {0};

  int total_scenery_num = 0;
  while ((scenery_list_key = map_next(scenery_node->child_nodes, &scenery_list_iter)))
    texture_settings_bucket.node[total_scenery_num++] = array_list_get(*((struct ArrayList**)map_get(scenery_node->child_nodes, scenery_list_key)), 0);

  //#pragma omp parallel for schedule(dynamic)
  for (int scenery_num = 0; scenery_num < total_scenery_num; scenery_num++) {
    texture_settings_bucket.scenery[scenery_num] = calloc(1, sizeof(struct Scenery));
    struct XmlNode* position_node = xml_node_get_child(texture_settings_bucket.node[scenery_num], "position");
    float scale = atof(xml_node_get_data(xml_node_get_child(texture_settings_bucket.node[scenery_num], "scale")));
    int repeat_factor = atoi(xml_node_get_data(xml_node_get_child(texture_settings_bucket.node[scenery_num], "repeat")));
    float offset = atof(xml_node_get_data(xml_node_get_child(texture_settings_bucket.node[scenery_num], "offset")));

    struct Entity new_scenery_entity = {0};
    entity_init(&new_scenery_entity);
    vector_push_back(&game->entities, &new_scenery_entity);

    struct Position new_scenery_position = {0};
    new_scenery_position.position.x = atof(xml_node_get_data(xml_node_get_child(position_node, "x")));
    new_scenery_position.position.y = atof(xml_node_get_data(xml_node_get_child(position_node, "y")));
    new_scenery_position.position.z = atof(xml_node_get_data(xml_node_get_child(position_node, "z")));
    new_scenery_position.scale = scale;
    component_registry_set(&game->position_registry, &new_scenery_entity, &new_scenery_position);

    struct Scenery new_scenery = (struct Scenery){.texture_path = xml_node_get_attribute(position_node, "path"), .repeat_factor = repeat_factor, .offset = offset};
    component_registry_set(&game->scenery_registry, &new_scenery_entity, &new_scenery);
  }

  //xml_parser_delete(game_stage_node);

  for (int render_num = 0; render_num < vector_size(&game->entities); render_num++) {
    struct Entity* entity = vector_get(&game->entities, render_num);
    struct Scenery* scenery = component_registry_get(&game->scenery_registry, entity->entity_id);
    if (scenery == NULL)
      break;
    struct Position* position = component_registry_get(&game->position_registry, entity->entity_id);
    for (int repeat_num = 0; repeat_num < scenery->repeat_factor; repeat_num++) {
      struct Entity new_render_entity = {0};
      entity_init(&new_render_entity);
      vector_push_back(&game->entities, &new_render_entity);

      struct Render new_scenery_render = {0};
      sprite_init(&new_scenery_render.sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, scenery->texture_path));
      new_scenery_render.sprite.position = position->position;
      new_scenery_render.sprite.position.x = position->position.x + (position->width * repeat_num) * scenery->offset;
      new_scenery_render.sprite.scale = (vec3){.x = position->scale, .y = position->scale, .z = position->scale};
      component_registry_set(&game->render_registry, &new_render_entity, &new_scenery_render);
    }
  }

  //// Sort scenery by z position
  for (int render_num = 0; render_num < vector_size(&game->entities); render_num++) {
    for (int other_render_num = render_num; other_render_num > 0; other_render_num--) {
      struct Entity* entity_one = ((struct Entity*)vector_get(&game->render_entities, render_num));
      struct Entity* entity_two = ((struct Entity*)vector_get(&game->render_entities, other_render_num - 1));

      struct Position* position_one = component_registry_get(&game->position_registry, entity_one->entity_id);
      struct Position* position_two = component_registry_get(&game->position_registry, entity_two->entity_id);

      if (position_one->position.z > position_two->position.z)
        continue;

      struct Entity swap = *((struct Entity*)vector_get(&game->render_entities, render_num));
      vector_set(&game->render_entities, render_num, vector_get(&game->render_entities, render_num - 1));
      vector_set(&game->render_entities, render_num - 1, &swap);
    }
  }
}*/

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  game->window = window;

  dad_n_me_server_init(&game->dad_n_me_server, DAD_N_ME_SERVER_EMULATE);

  resource_manager_init(&game->resource_manager, gpu_api);
  struct ResourceManager* resource_manager = &game->resource_manager;

  game->job_system = calloc(1, sizeof(struct JobSystem));
  job_system_init(game->job_system, game);

  audio_clip_init(&game->music_clip, resource_manager->music_clip_cache, MUSIC_AUDIO_CLIP, 1, 0.75f, 0.025f);
  audio_manager_play_audio_clip(&resource_manager->audio_manager, &game->music_clip);

  fxaa_shader_init(&game->fxaa_shader, gpu_api);
  game->fxaa_shader.on = 0;
  sprite_shader_init(&game->sprite_shader, gpu_api, 0);
  sprite_animation_shader_init(&game->sprite_animation_shader, gpu_api, 0);

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

  // Get current game state from server
  //array_list_init(&game->scenery_render_list);
  //vector_init(&game->render_entities, sizeof(struct Entity));

  entity_init(&game->entity);

  component_registry_init(&game->position_registry, sizeof(struct Position));
  component_registry_init(&game->scenery_registry, sizeof(struct Scenery));

  struct Position new_position = {0};
  component_registry_set(&game->position_registry, &game->entity, &new_position);
  printf("%f\n", ((struct Position*)component_registry_get(&game->position_registry, game->entity.entity_id))->position.x);
  //struct Position* test = map_get_by_index(&game->position_registry, 0);  //((struct Position*)game->position_registry.registry.buckets);

  struct Scenery new_scenery = (struct Scenery){.texture_path = "./assets/textures/floortiles/streetgrass.png", .repeat_factor = 2, .offset = 0.99f};
  component_registry_set(&game->scenery_registry, &game->entity, &new_scenery);

  component_registry_init(&game->render_registry, sizeof(struct Render));
  struct Render new_render = {0};
  sprite_init(&new_render.sprite, gpu_api, &game->sprite_shader.shader, texture_cache_get(&game->resource_manager.texture_cache, ((struct Scenery*)component_registry_get(&game->scenery_registry, game->entity.entity_id))->texture_path));
  component_registry_set(&game->render_registry, &game->entity, &new_render);

  vector_init(&game->entities, sizeof(struct Entity));

  game_hotswap_scenery(game, gpu_api);

  game->resource_manager.audio_manager.master_volume = 0.0f;
}

void game_delete(struct Game* game, struct Mana* mana) {
  struct GPUAPI* gpu_api = &mana->engine.gpu_api;
  // Note: Wait for command buffers to finish before deleting desciptor sets
  vkWaitForFences(gpu_api->vulkan_state->device, 2, gpu_api->vulkan_state->swap_chain->in_flight_fences, VK_TRUE, UINT64_MAX);

  texture_cache_delete(&game->resource_manager.texture_cache, gpu_api);

  component_registry_delete(&game->position_registry);

  /*for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
    struct RenderScenery* entity = array_list_get(&game->scenery_render_list, entity_num);
    render_scenery_delete(entity, gpu_api);
    free(entity);
  }

  for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
    struct Entity* entity = array_list_get(&game->stage_entity_render_list, entity_num);
    (*entity->delete_func)(entity->entity_data, gpu_api);
  }

  free(game->render_me);
  free(game->render_wilbur);*/

  //array_list_delete(&game->scenery_render_list);
  //array_list_delete(&game->stage_entity_render_list);

  //vector_delete(&game->render_entities);

  //game_state_delete(game->game_state);
  //free(game->game_state);

  sprite_animation_shader_delete(&game->sprite_animation_shader, gpu_api);
  sprite_shader_delete(&game->sprite_shader, gpu_api);

  fxaa_shader_delete(&game->fxaa_shader, gpu_api);

  job_system_delete(game->job_system);
  free(game->job_system);

  resource_manager_delete(&game->resource_manager);
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
    }

    for (int entity_num = 0; entity_num < array_list_size(&game->scenery_render_list); entity_num++) {
      struct Entity* entity = array_list_get(&game->scenery_render_list, entity_num);
      (*entity->recreate_func)(entity->entity_data, gpu_api);
    }*/
  }

  player_camera_update(&game->player_camera, delta_time);
  game_update_input(game, &mana->engine);
  // Draw order sorting
  /*for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
    for (int other_entity_num = entity_num; other_entity_num > 0; other_entity_num--) {
      struct Entity* entity_one = ((struct Entity*)array_list_get(&game->stage_entity_render_list, other_entity_num));
      struct Entity* entity_two = ((struct Entity*)array_list_get(&game->stage_entity_render_list, other_entity_num - 1));
      if (entity_one->position.y - entity_one->height / 2.0f < entity_two->position.y - entity_two->height / 2.0f)
        continue;
      array_list_swap(&game->stage_entity_render_list, other_entity_num, other_entity_num - 1);
    }
  }
  // TODO: Make this less messy with inlined function
  // Update scenery
  struct EntityUpdateData* scenery_update_data_pool = malloc(sizeof(struct EntityUpdateData) * array_list_size(&game->scenery_render_list));
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

  gpu_api->vulkan_state->gbuffer->projection_matrix = camera_get_projection_matrix(&game->player_camera.camera, game->window);
  gpu_api->vulkan_state->gbuffer->view_matrix = camera_get_view_matrix(&game->player_camera.camera);

  gbuffer_start(gpu_api->vulkan_state->gbuffer, gpu_api->vulkan_state);
  // TODO: Implement instanced rendering and texture atlas
  // Render scenery
  for (int entity_num = 0; entity_num < vector_size(&game->entities); entity_num++) {
    struct Entity* entity = vector_get(&game->entities, entity_num);
    struct Render* next_render = map_get(&game->render_registry.registry, entity->entity_id);
    sprite_update_uniforms(&next_render->sprite, gpu_api);
    sprite_render(&next_render->sprite, gpu_api);
    //(*entity->render_func)(entity->entity_data, gpu_api);
  }
  // Render sprites
  /*for (int entity_num = 0; entity_num < array_list_size(&game->stage_entity_render_list); entity_num++) {
    struct Entity* entity = array_list_get(&game->stage_entity_render_list, entity_num);
    (*entity->render_func)(entity->entity_data, gpu_api);
  }*/

  //const char* render_list_key = NULL;
  //struct MapIter render_list_iter = map_iter();
  //while ((render_list_key = map_next(&game->render_registry.registry, &render_list_iter))) {
  //  struct Render* next_render = map_get(&game->render_registry.registry, render_list_key);
  //  sprite_update_uniforms(&next_render->sprite, gpu_api);
  //  sprite_render(&next_render->sprite, gpu_api);
  //}

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

  if (input_manager->keys[GLFW_KEY_1].pushed == 1)
    (game->fxaa_shader.on ^= 1) ? printf("FXAA ON\n") : printf("FXAA OFF\n");

  //if (input_manager->keys[GLFW_KEY_2].pushed == 1)
  //  game_hotswap_scenery(game, &engine->gpu_api);
  //
  //if (input_manager->keys[GLFW_KEY_3].pushed == 1)
  //  game->player_camera.focus_entity = &game->render_me->me.entity;
  //
  //if (input_manager->keys[GLFW_KEY_4].pushed == 1)
  //  game->player_camera.focus_entity = &game->render_wilbur->wilbur.entity;

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

  if (input_manager->keys[GLFW_KEY_Z].state == PRESSED)
    game->player_camera.camera.position.z += 0.01f;
  if (input_manager->keys[GLFW_KEY_X].state == PRESSED)
    game->player_camera.camera.position.z -= 0.01f;

  if (input_manager->keys[GLFW_KEY_E].state == PRESSED)
    game->player_camera.camera.position.y += 0.01f;
  if (input_manager->keys[GLFW_KEY_Q].state == PRESSED)
    game->player_camera.camera.position.y -= 0.01f;
}
