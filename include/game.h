#pragma once
#ifndef GAME_H
#define GAME_H

#include <mana/core/memoryallocator.h>
//
#include <chaos/chaos.h>
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
  //struct RenderMe* render_me;
  //struct RenderWilbur* render_wilbur;

  //struct ArrayList scenery_render_list;
  //struct ArrayList stage_entity_render_list;
  struct AudioClip music_clip;

  // NOTE: If running server locally, share memory of game states
  struct DadNMeServer dad_n_me_server;
  struct GameState* game_state;

  //////////////////////////////////
  // TODO: Move to server
  struct ComponentRegistry position_registry;
  struct ComponentRegistry scenery_registry;
  struct Entity entity;
  struct Vector entities;
  //////////////////////////////////
  struct ComponentRegistry render_registry;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
void game_update_camera(struct Game* game, struct Engine* engine);
void game_update_input(struct Game* game, struct Engine* engine);

#endif  //GAME_H
