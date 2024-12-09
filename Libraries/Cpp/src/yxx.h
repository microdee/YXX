#ifdef __cplusplus
#pragma once
extern "C" {
#endif

#include "yaml-cpp/yaml.h"
#include "pugixml.hpp"

namespace yxx
{
    /** Arguments for converting an XML node to YAML following YXX convention */
    struct X2YArguments
    {
        /**
         * Maximum number of attributes which can be organized to one line. Above this number attributes will be placed
         * in block mapping notation accross multiple lines
         */
        int MaxAttributesInFlow = 5;
    };

    /** Convert an XML node into YAML following YXX convention */
    void Convert(YAML::Emitter& o, pugi::xml_node const& node, X2YArguments const& args = {});

    /** Convert an XML node into YAML following YXX convention with factory default arguments */
    YAML::Emitter& operator << (YAML::Emitter& o, pugi::xml_node const& node);

    struct EmitNode
    {
        EmitNode(pugi::xml_node const& node, X2YArguments&& args = {})
            : Node(node)
            , Args(std::move(args))
            {}

        pugi::xml_node const& Node;
        X2YArguments Args;
    };

    /** Convert an XML node into YAML following YXX convention with provided arguments default arguments */
    YAML::Emitter& operator << (YAML::Emitter& o, EmitNode&& node);
}

#ifdef __cplusplus
}
#endif
