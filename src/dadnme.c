#include "dadnme.h"

int dad_n_me_init(struct DadNMe* dad_n_me) {
  int mana_init_error = mana_init(&dad_n_me->mana, (struct EngineSettings){GLFW_LIBRARY, VULKAN_API});
  switch (mana_init_error) {
    case (MANA_SUCCESS):
      break;
    case (MANA_ENGINE_ERROR):
      fprintf(stderr, "Failed to setup glfw for engine!\n");
      return DAD_N_ME_MANA_ERROR;
    default:
      fprintf(stderr, "Unknown mana error! Error code: %d\n", mana_init_error);
      return DAD_N_ME_MANA_ERROR;
  }
  // TODO: Load from settings or on first boot do fullscreen and monitor resolution
  window_init(&dad_n_me->window, &dad_n_me->mana.engine, 1920, 1080);

  game_init(&dad_n_me->game, &dad_n_me->mana, &dad_n_me->window);

  return DAD_N_ME_SUCCESS;
}

void dad_n_me_delete(struct DadNMe* dad_n_me) {
  game_delete(&dad_n_me->game, &dad_n_me->mana);
  window_delete(&dad_n_me->window);
  mana_cleanup(&dad_n_me->mana);
}

void dad_n_me_start(struct DadNMe* dad_n_me) {
  struct Engine* engine = &dad_n_me->mana.engine;

  while (window_should_close(&dad_n_me->window) == 0) {
    engine->fps_counter.now_time = engine_get_time();
    engine->fps_counter.delta_time = engine->fps_counter.now_time - engine->fps_counter.last_time;
    engine->fps_counter.last_time = engine->fps_counter.now_time;

    window_prepare_frame(&dad_n_me->window);
    //(*dad_n_me->update_func)(dad_n_me->state.game, &dad_n_me->mana, engine->fps_counter.delta_time);
    game_update(&dad_n_me->game, &dad_n_me->mana, engine->fps_counter.delta_time);
    window_end_frame(&dad_n_me->window);

    engine->fps_counter.frames++;

    if (engine_get_time() - engine->fps_counter.timer > 1.0) {
      engine->fps_counter.timer++;

      engine->fps_counter.second_target_fps = engine->fps_counter.target_fps;
      engine->fps_counter.second_average_fps = engine->fps_counter.average_fps;
      engine->fps_counter.second_frames = engine->fps_counter.frames;

      char title_buffer[1024];
      sprintf(title_buffer, "Dad n' Me %d", (int)engine->fps_counter.second_frames);
      window_set_title(&dad_n_me->window, title_buffer);

      engine->fps_counter.fps = engine->fps_counter.frames;

      float average_calc = 0;
      for (int loopNum = FPS_COUNT - 1; loopNum >= 0; loopNum--) {
        if (loopNum != 0)
          engine->fps_counter.fps_past[loopNum] = engine->fps_counter.fps_past[loopNum - 1];

        average_calc += engine->fps_counter.fps_past[loopNum];
      }
      engine->fps_counter.fps_past[0] = engine->fps_counter.fps;
      engine->fps_counter.average_fps = average_calc / FPS_COUNT;
      engine->fps_counter.frames = 0;
    }
  }
}