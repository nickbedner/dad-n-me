#pragma once
#ifndef GAME_H
#define GAME_H

#include <mana/core/memoryallocator.h>
//
#include <chaos/chaos.h>
#include <core/dimensions.h>
#include <core/entity.h>
#include <core/position.h>
#include <core/scenery.h>
#include <dadnmeserver.h>
#include <mana/audio/audiomanager.h>
#include <mana/core/inputmanager.h>
#include <mana/graphics/entities/sprite.h>
#include <mana/graphics/shaders/fxaashader.h>
#include <mana/graphics/shaders/spriteanimationshader.h>
#include <mana/graphics/shaders/spriteshader.h>
#include <mana/graphics/utilities/camera.h>
#include <mana/graphics/utilities/spriteanimation.h>
#include <mana/mana.h>

#include "core/jobsystem.h"
#include "core/render.h"
#include "render/renderme.h"
#include "render/renderscenery.h"
#include "render/renderwilbur.h"
#include "utilities/playercamera.h"
#include "utilities/resourcemanager.h"

struct Game {
  struct Window* window;

  struct JobSystem* job_system;

  struct SpriteShader sprite_shader;
  struct SpriteAnimationShader sprite_animation_shader;
  struct FXAAShader fxaa_shader;

  struct ResourceManager resource_manager;

  struct PlayerCamera player_camera;

  struct AudioClip music_clip;

  // NOTE: If running server locally, share memory of game states
  struct DadNMeServer dad_n_me_server;
  struct GameState* game_state;

  //////////////////////////////////
  // TODO: Move to server
  struct ComponentRegistry position_registry;
  struct ComponentRegistry dimensions_registry;
  struct ComponentRegistry scenery_registry;
  struct Vector entities;
  //////////////////////////////////
  struct ComponentRegistry render_registry;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
void game_update_camera(struct Game* game, struct Engine* engine);
void game_update_input(struct Game* game, struct Engine* engine);

static inline void game_hotswap_scenery(struct Game* game, struct GPUAPI* gpu_api) {
  // Clear any existing scenery
  if (game->scenery_registry.registry.num_nodes > 0) {
    char* scenery_key = NULL;
    struct MapIter scenery_iter = map_iter();
    while ((scenery_key = map_next(&game->scenery_registry.registry, &scenery_iter))) {
      sprite_delete(&((struct Render*)map_get(&game->render_registry.registry, scenery_key))->sprite, gpu_api);

      component_registry_remove(&game->position_registry, scenery_key);
      component_registry_remove(&game->dimensions_registry, scenery_key);
      component_registry_remove(&game->render_registry, scenery_key);

      // TODO: Write better way to remove single entity
      for (int entity_num = 0; entity_num < vector_size(&game->entities); entity_num++) {
        if (strcmp(scenery_key, ((struct Entity*)vector_get(&game->entities, entity_num))->entity_id)) {
          vector_remove(&game->entities, entity_num);
          break;
        }
      }
    }

    component_registry_delete(&game->scenery_registry);
    component_registry_init(&game->scenery_registry, sizeof(struct Scenery));
  }

  // Load scenery from xml file
  struct XmlNode* game_stage_node = xml_parser_load_xml_file("./assets/stages/gamestage.xml");
  struct XmlNode* scenery_node = xml_node_get_child(game_stage_node, "scenery");
  const char* scenery_list_key = NULL;
  struct MapIter scenery_list_iter = map_iter();

  struct SceneryBucket {
    struct Scenery scenery[64];
    struct XmlNode* node[64];
  } texture_settings_bucket = {0};

  // Find each piece of unique scenery
  int total_scenery_num = 0;
  while ((scenery_list_key = map_next(scenery_node->child_nodes, &scenery_list_iter)))
    texture_settings_bucket.node[total_scenery_num++] = array_list_get(*((struct ArrayList**)map_get(scenery_node->child_nodes, scenery_list_key)), 0);

  // Build components from xml data
  //#pragma omp parallel for schedule(dynamic)
  for (int scenery_num = 0; scenery_num < total_scenery_num; scenery_num++) {
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

      struct Dimensions new_scenery_dimensions = {.direction = 1.0f, .scale = scale, .height = new_scenery_render.sprite.height * scale, .width = new_scenery_render.sprite.width * scale};
      component_registry_set(&game->dimensions_registry, &new_scenery_entity, &new_scenery_dimensions);

      struct Position new_scenery_position = (struct Position){.x = atof(xml_node_get_data(xml_node_get_child(position_node, "x"))) + (new_scenery_dimensions.width * repeat_num) * new_scenery.offset, .y = atof(xml_node_get_data(xml_node_get_child(position_node, "y"))), .z = atof(xml_node_get_data(xml_node_get_child(position_node, "z")))};
      component_registry_set(&game->position_registry, &new_scenery_entity, &new_scenery_position);

      new_scenery_render.sprite.position = (vec3){.x = new_scenery_position.x, .y = new_scenery_position.y, .z = new_scenery_position.z};
      new_scenery_render.sprite.scale = (vec3){.x = scale, .y = scale, .z = scale};
      component_registry_set(&game->render_registry, &new_scenery_entity, &new_scenery_render);
    }
  }

  xml_parser_delete(game_stage_node);
}

#endif  //GAME_H
