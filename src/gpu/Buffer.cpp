//
// Created by Petr on 6/5/2022.
//

#include "Buffer.h"

namespace pf::gpu {

BufferMapping::~BufferMapping() { unmap(owner); }

}  // namespace pf::gpu