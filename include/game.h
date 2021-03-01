#pragma once
#ifndef GAME_H
#define GAME_H

#include <mana/core/memoryallocator.h>
//
#include <chaos/chaos.h>
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

#include "render/renderme.h"
#include "render/renderwilbur.h"
#include "utilities/playercamera.h"
#include "utilities/resourcemanager.h"

struct Game {
  struct Window* window;

  struct SpriteShader sprite_shader;
  struct SpriteAnimationShader sprite_animation_shader;
  struct FXAAShader fxaa_shader;

  struct ResourceManager resource_manager;

  //struct Sprite* hud_sprite;
  //struct SpriteAnimation* walking_animation;

  //struct ArrayList sprites;
  //struct ArrayList animated_sprites;

  struct PlayerCamera player_camera;
  struct RenderMe* render_me;
  struct RenderWilbur* render_wilbur;
  //struct Sandcastle* sandcastle;

  //struct ArrayList background_entity_list;
  //struct ArrayList stage_entity_list;
  //struct ArrayList foreground_entity_list;

  struct AudioClip music_clip;

  // NOTE: If running server locally, share memory of game states
  struct DadNMeServer dad_n_me_server;
  struct GameState* game_state;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
void game_update_camera(struct Game* game, struct Engine* engine);
void game_update_input(struct Game* game, struct Engine* engine);

#endif  //GAME_H
