#pragma once
#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <mana/core/memoryallocator.h>
//
#include <mana/audio/audiomanager.h>
#include <mana/core/xmlparser.h>
#include <mana/graphics/utilities/texturecache.h>
#include <mana/mana.h>

struct ResourceManager {
  // TODO: Move to someplace more abstract this is way too specfic
  struct AudioManager audio_manager;
  struct AudioClipCache* music_clip_cache;
  struct AudioClipCache* fart_clip_cache;

  struct TextureCache texture_cache;
};

int resource_manager_init(struct ResourceManager* resource_manager, struct GPUAPI* gpu_api);
void resource_manager_delete(struct ResourceManager* resource_manager);

#endif  // RESOURCE_MANAGER_H
