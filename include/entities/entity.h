#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <mana/core/memoryallocator.h>
//
#include <mana/mana.h>

struct Entity {
  void* entity_data;
  void (*delete_func)(void*, struct GPUAPI*);
  void (*update_func)(void*, void*, float);
  void (*render_func)(void*, struct GPUAPI*);
};

#endif  // ENTITY_H
