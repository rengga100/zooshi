// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ZOOSHI_WORLD_H_
#define ZOOSHI_WORLD_H_

#include <map>
#include <memory>
#include <string>
#include "component_library/common_services.h"
#include "component_library/entity_factory.h"
#include "component_library/meta.h"
#include "component_library/physics.h"
#include "component_library/rendermesh.h"
#include "component_library/transform.h"
#include "components/attributes.h"
#include "components/audio_listener.h"
#include "components/digit.h"
#include "components/patron.h"
#include "components/player.h"
#include "components/player_projectile.h"
#include "components/rail_denizen.h"
#include "components/rail_node.h"
#include "components/river.h"
#include "components/services.h"
#include "components/shadow_controller.h"
#include "components/sound.h"
#include "components/time_limit.h"
#include "components_generated.h"
#include "entity/entity_manager.h"
#include "fplbase/renderer.h"
#include "fplbase/render_target.h"
#include "inputcontrollers/base_player_controller.h"
#include "motive/engine.h"
#include "railmanager.h"
#include "world_editor/edit_options.h"
#include "world_editor/world_editor.h"
#include "world_renderer.h"

#ifdef USING_GOOGLE_PLAY_GAMES
#include "gpg_manager.h"
#include "gpg_multiplayer.h"
#endif

namespace pindrop {

class AudioEngine;

}  // namespace pindrop

namespace fpl {

class InputSystem;
class AssetManager;

namespace fpl_project {

class WorldRenderer;
struct Config;

class World {
 public:
  World() : is_in_cardboard(false), draw_debug_physics(false) {}

  void Initialize(const Config& config, InputSystem* input_system,
                  AssetManager* asset_mgr, WorldRenderer* worldrenderer,
                  FontManager* font_manager, pindrop::AudioEngine* audio_engine,
                  event::EventManager* event_manager, Renderer* renderer);

  motive::MotiveEngine motive_engine;

  // Entity manager
  entity::EntityManager entity_manager;

  // Entity factory, for creating entities from data.
  std::unique_ptr<component_library::EntityFactory> entity_factory;

  // Rail Manager - manages loading and storing of rail definitions
  RailManager rail_manager;

  // Components
  component_library::TransformComponent transform_component;
  RailDenizenComponent rail_denizen_component;
  PlayerComponent player_component;
  PlayerProjectileComponent player_projectile_component;
  component_library::RenderMeshComponent render_mesh_component;
  component_library::PhysicsComponent physics_component;
  PatronComponent patron_component;
  TimeLimitComponent time_limit_component;
  AudioListenerComponent audio_listener_component;
  SoundComponent sound_component;
  AttributesComponent attributes_component;
  DigitComponent digit_component;
  RiverComponent river_component;
  RailNodeComponent rail_node_component;
  ServicesComponent services_component;
  component_library::CommonServicesComponent common_services_component;
  ShadowControllerComponent shadow_controller_component;
  component_library::MetaComponent meta_component_;
  editor::EditOptionsComponent edit_options_component_;

  // Each player has direct control over one entity.
  entity::EntityRef active_player_entity;

  const Config* config;

  AssetManager* asset_manager;
  WorldRenderer* world_renderer;

  // TODO: Refactor all components so they don't require their source
  // data to remain in memory after their initial load. Then get rid of this,
  // which keeps all entity files loaded in memory.
  std::map<std::string, std::string> loaded_entity_files_;

  // Determines if the game is in Cardboard mode (for special rendering)
  bool is_in_cardboard;

  // Determines if the debug drawing of physics should be used
  bool draw_debug_physics;

#ifdef USING_GOOGLE_PLAY_GAMES
  GPGManager* gpg_manager;

  // Network multiplayer library for multi-screen version
  GPGMultiplayer* gpg_multiplayer;
#endif
};

// Removes all entities from the world, then repopulates it based on the entity
// definitions given in the WorldDef. The input controller is required to hook
// up the player's controller to the player entity.
void LoadWorldDef(World* world, const WorldDef* world_def,
                  BasePlayerController* input_controller);

}  // fpl_project
}  // fpl

#endif  // ZOOSHI_WORLD_H_
