#ifdef __cplusplus
#pragma once
extern "C" {
#endif

#include "yaml-cpp/yaml.h"
#include "pugixml.hpp"

namespace yxx
{
    void Convert(YAML::Emitter& o, pugi::xml_node const& node);
}

#ifdef __cplusplus
}
#endif
