#ifdef __cplusplus
#pragma once
extern "C" {
#endif

#include "yaml-cpp/yaml.h"
#include "pugixml.hpp"

namespace yxx
{
    YAML::Node Convert(pugi::xml_document const& xmlDoc);
}

#ifdef __cplusplus
}
#endif
