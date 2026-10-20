#pragma once

#include <nlohmann/json.hpp>

namespace iris {
#ifndef IRIS_DATA_TYPE
using json = nlohmann::json;
#else
using json = IRIS_DATA_TYPE;
#endif
} // namespace iris

