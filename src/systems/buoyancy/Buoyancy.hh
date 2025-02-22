/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef IGNITION_GAZEBO_SYSTEMS_BUOYANCY_HH_
#define IGNITION_GAZEBO_SYSTEMS_BUOYANCY_HH_

#include <ignition/gazebo/System.hh>
#include <memory>

namespace ignition
{
namespace gazebo
{
// Inline bracket to help doxygen filtering.
inline namespace IGNITION_GAZEBO_VERSION_NAMESPACE {
namespace systems
{
  // Forward declaration
  class BuoyancyPrivate;

  /// \brief A system that simulates buoyancy of objects immersed in fluid.
  /// All SDF parameters are optional. This system must be attached to the
  /// world and this system will apply buoyancy to all links that have inertia
  /// and collision shapes.
  ///
  /// The volume and center of volume will be computed for each link, and
  /// stored as components. During each iteration, Archimedes' principle is
  /// applied to each link with a volume and center of volume component.
  ///
  /// Plane shapes are not handled by this plugin, and will not be affected
  /// by buoyancy.
  ///
  /// ## System Parameters
  ///
  /// * `<uniform_fluid_density>` sets the density of the fluid that surrounds
  /// the buoyant object.
  ///
  /// ## Example
  ///
  /// The `buoyancy.sdf` SDF file contains three submarines. The first
  /// submarine is neutrally buoyant, the second sinks, and the third
  /// floats. To run:
  ///
  /// ```
  /// ign gazebo -v 4 buoyancy.sdf
  /// ```
  class Buoyancy
      : public System,
        public ISystemConfigure,
        public ISystemPreUpdate
  {
    /// \brief Constructor
    public: Buoyancy();

    /// \brief Destructor
    public: ~Buoyancy() override = default;

    // Documentation inherited
    public: void Configure(const Entity &_entity,
                           const std::shared_ptr<const sdf::Element> &_sdf,
                           EntityComponentManager &_ecm,
                           EventManager &_eventMgr) override;

    // Documentation inherited
    public: void PreUpdate(
                const ignition::gazebo::UpdateInfo &_info,
                ignition::gazebo::EntityComponentManager &_ecm) override;

    /// \brief Private data pointer
    private: std::unique_ptr<BuoyancyPrivate> dataPtr;
  };
  }
}
}
}

#endif
