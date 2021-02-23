#pragma once
#ifndef DAD_N_ME_H
#define DAD_N_ME_H

#include <mana/core/memoryallocator.h>
//
#include <mana/mana.h>

#include "game.h"

struct DadNMe {
  struct Mana mana;
  struct Window window;
  struct Game game;
};

enum DAD_N_ME_STATUS {
  DAD_N_ME_SUCCESS = 0,
  DAD_N_ME_MANA_ERROR,
  DAD_N_ME_LAST_ERROR
};

int dad_n_me_init(struct DadNMe* dad_n_me);
void dad_n_me_delete(struct DadNMe* dad_n_me);
void dad_n_me_start(struct DadNMe* dad_n_me);

#endif  // DAD_N_ME_H
