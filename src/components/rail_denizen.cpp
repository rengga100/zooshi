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

#include "components/rail_denizen.h"

#include <algorithm>

#include "components/transform.h"
#include "entity/component.h"
#include "fplbase/flatbuffer_utils.h"
#include "mathfu/constants.h"
#include "motive/init.h"
#include "rail_def_generated.h"
#include "components_generated.h"

using mathfu::vec3;

namespace fpl {
namespace fpl_project {

void Rail::Initialize(const RailDef* rail_def, float spline_granularity) {
  // Get position extremes.
  vec3 position_min(std::numeric_limits<float>::infinity());
  vec3 position_max(-std::numeric_limits<float>::infinity());
  for (auto iter = rail_def->nodes()->begin(); iter != rail_def->nodes()->end();
       ++iter) {
    const vec3 position = LoadVec3(iter->position());
    position_min = vec3::Min(position_min, position);
    position_max = vec3::Max(position_max, position);
  }

  // Initialize the compact-splines to have the best precision possible,
  // given the range limits.
  for (motive::MotiveDimension i = 0; i < kDimensions; ++i) {
    splines[i].Init(Range(position_min[i], position_max[i]), spline_granularity);
  }

  // Initialize the splines. For now, the splines all have key points at the
  // same time values, but this is a limitation that we can (and should) lift
  // to maximize compression.
  for (auto iter = rail_def->nodes()->begin(); iter != rail_def->nodes()->end();
       ++iter) {
    const float t = iter->time();
    const vec3 position = LoadVec3(iter->position());
    const vec3 tangent = LoadVec3(iter->tangent());
    for (motive::MotiveDimension i = 0; i < kDimensions; ++i) {
      splines[i].AddNode(t, position[i], tangent[i]);
    }
  }
}

void RailDenizenData::Initialize(const Rail& rail, float start_time,
                                 motive::MotiveEngine* engine) {
  motivator.Initialize(motive::SmoothInit(), engine);
  motivator.SetSpline(SplinePlayback3f(rail.splines, start_time, true));
}

void RailDenizenComponent::UpdateAllEntities(entity::WorldTime /*delta_time*/) {
  for (auto iter = component_data_.begin(); iter != component_data_.end();
       ++iter) {
    RailDenizenData* rail_denizen_data = GetComponentData(iter->entity);
    TransformData* transform_data = Data<TransformData>(iter->entity);
    transform_data->position = rail_denizen_data->Position();
    transform_data->orientation = mathfu::quat::RotateFromTo(
        rail_denizen_data->Velocity(), mathfu::kAxisY3f);
  }
}

void RailDenizenComponent::AddFromRawData(entity::EntityRef& entity,
                                          const void* raw_data) {
  auto component_data = static_cast<const ComponentDefInstance*>(raw_data);
  assert(component_data->data_type() == ComponentDataUnion_RailDenizenDef);
  auto rail_denizen_def =
      static_cast<const RailDenizenDef*>(component_data->data());
  RailDenizenData* data = AddEntity(entity);
  data->Initialize(rail_, rail_denizen_def->start_time(), engine_);
}

void RailDenizenComponent::InitEntity(entity::EntityRef& entity) {
  entity_manager_->AddEntityToComponent(entity,
                                        ComponentDataUnion_TransformDef);
}

void RailDenizenComponent::Initialize(const RailDef* rail_def) {
  static const float kSplineGranularity = 10.0f;
  rail_.Initialize(rail_def, kSplineGranularity);
}

}  // fpl_project
}  // fpl
