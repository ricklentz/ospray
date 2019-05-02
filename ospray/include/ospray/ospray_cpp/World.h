// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include <ospray/ospray_cpp/GeometryInstance.h>
#include <ospray/ospray_cpp/ManagedObject.h>
#include <ospray/ospray_cpp/VolumeInstance.h>

namespace ospray {
  namespace cpp {

    class World : public ManagedObject_T<OSPWorld>
    {
     public:
      World();
      World(const World &copy);
      World(OSPWorld existing);

      void addGeometryInstance(GeometryInstance &v) const;
      void addGeometryInstance(OSPGeometryInstance v) const;

      void removeGeometryInstance(GeometryInstance &v) const;
      void removeGeometryInstance(OSPGeometryInstance v) const;

      void addVolumeInstance(VolumeInstance &v) const;
      void addVolumeInstance(OSPVolumeInstance v) const;

      void removeVolumeInstance(VolumeInstance &v) const;
      void removeVolumeInstance(OSPVolumeInstance v) const;
    };

    // Inlined function definitions ///////////////////////////////////////////

    inline World::World()
    {
      OSPWorld c = ospNewWorld();
      if (c) {
        ospObject = c;
      } else {
        throw std::runtime_error("Failed to create OSPWorld!");
      }
    }

    inline World::World(const World &copy)
        : ManagedObject_T<OSPWorld>(copy.handle())
    {
    }

    inline World::World(OSPWorld existing) : ManagedObject_T<OSPWorld>(existing)
    {
    }

    inline void World::addGeometryInstance(GeometryInstance &v) const
    {
      addGeometryInstance(v.handle());
    }

    inline void World::addGeometryInstance(OSPGeometryInstance v) const
    {
      ospAddGeometryInstance(handle(), v);
    }

    inline void World::removeGeometryInstance(GeometryInstance &v) const
    {
      removeGeometryInstance(v.handle());
    }

    inline void World::removeGeometryInstance(OSPGeometryInstance v) const
    {
      ospRemoveGeometryInstance(handle(), v);
    }

    inline void World::addVolumeInstance(VolumeInstance &v) const
    {
      addVolumeInstance(v.handle());
    }

    inline void World::addVolumeInstance(OSPVolumeInstance v) const
    {
      ospAddVolumeInstance(handle(), v);
    }

    inline void World::removeVolumeInstance(VolumeInstance &v) const
    {
      removeVolumeInstance(v.handle());
    }

    inline void World::removeVolumeInstance(OSPVolumeInstance v) const
    {
      ospRemoveVolumeInstance(handle(), v);
    }

  }  // namespace cpp
}  // namespace ospray
