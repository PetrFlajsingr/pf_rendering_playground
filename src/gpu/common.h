//
// Created by Petr on 6/5/2022.
//

#pragma once

#include <NamedType/named_type.hpp>

namespace pf::gpu {
using Binding = fluent::NamedType<std::int32_t, struct BindingTag>;
}
