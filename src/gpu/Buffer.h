//
// Created by Petr on 6/5/2022.
//

#pragma once

#include "GpuObject.h"

namespace pf::gpu {

class Buffer : public GpuObject {
 public:
  PF_GPU_OBJECT_TYPE(GpuObject::Type::Buffer)
};

}  // namespace pf::gpu
