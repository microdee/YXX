#ifdef __cplusplus
#pragma once
extern "C" {
#endif

#include "yaml-cpp/yaml.h"
#include "pugixml.hpp"

namespace yxx
{
    struct X2YArguments
    {
        int MaxAttributesInFlow = 5;
    };

    /**
     * @todo: make this into << operator as well
     */
    void Convert(YAML::Emitter& o, pugi::xml_node const& node, X2YArguments const& args = {});
}

#ifdef __cplusplus
}
#endif
