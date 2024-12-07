#include "yxx.h"

namespace yxx
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    YAML::Emitter& operator << (YAML::Emitter& out, pugi::xml_attribute const& v)
    {
        return out << YAML::Key << v.name() << YAML::Value << v.as_string();
    }

    template <YAML::EMITTER_MANIP Begin, YAML::EMITTER_MANIP End>
    class ScopedRegion
    {
        YAML::Emitter& Out;

    public:
        ScopedRegion(YAML::Emitter& out) : Out(out) { Out << Begin; }
        ~ScopedRegion() { Out << End; }

        template <typename T>
        ScopedRegion& operator << (T&& rhs)
        {
            Out << std::forward<T>(rhs);
            return *this;
        }
    };

    using Map = ScopedRegion<YAML::BeginMap, YAML::EndMap>;
    using Seq = ScopedRegion<YAML::BeginSeq, YAML::EndSeq>;

    void Convert(YAML::Emitter& o, pugi::xml_node const& node, X2YArguments const& args)
    {
        if (node == node.root())
        {
            auto children = Seq(o);
            for (auto const& child : node.children())
                Convert(o, child, args);
        }
        // XML declaration nodes are conserved with `/?xml` key. In YXX this is formatted as a regular tag.
        // Other processing instruction nodes are ignored as they are uncommon and can have any format. 
        if (node.type() == pugi::node_declaration)
        {
            auto tag = Map(o) << YAML::Key << "/?xml" << YAML::Value;
            {
                auto attrs = Map(o) << YAML::Flow;
                for (auto const& attr : node.attributes()) o << attr;
            }
            return;
        }

        // YAML comments are stripped by most parsers and they don't allow emitting them either, so to preserve XML
        // comment nodes they're emitted as `//: my comment` and vice-versa, "tags" named `//` are parsed as XML
        // comments
        if (node.type() == pugi::node_comment)
        {
            auto tag = Map(o) << YAML::Key << "//" << YAML::Value << node.value();
            return;
        }

        // plain text nodes are just handled as text scalars
        if (node.type() == pugi::node_cdata || node.type() == pugi::node_pcdata)
        {
            o << node.value();
            return;
        }

        int attrCount = std::distance(node.attributes_begin(), node.attributes_end());
        bool hasAttributes = node.attributes().empty();
        bool hasChildren = node.children().empty();

        auto tag = Map(o) << YAML::Key << node.name();

        // In case of a tag has neither children nor attributes, it should be just an empty key
        // <foobar /> -> - foobar:
        if (!hasAttributes && !hasChildren) return;

        o << YAML::Value;
        if (!hasAttributes)
        {
            auto children = Seq(o);
            for (auto const& child : node.children())
                Convert(o, child, args);
            
            return;
        }
        else if (!hasChildren)
        {
            auto attrs = Map(o) << (attrCount > args.MaxAttributesInFlow ? YAML::Block : YAML::Flow);
            for (auto const& attr : node.attributes()) o << attr;
            return;
        }

        if (attrCount > args.MaxAttributesInFlow)
        {
            {
                auto attrs = Map(o) << YAML::Block;
                for (auto const& attr : node.attributes()) o << attr;
            }
            // empty key
            o << YAML::Key << YAML::Value;
            auto children = Seq(o);
            for (auto const& child : node.children())
                Convert(o, child, args);
        }
        else
        {
            auto innerObject = Map(o) << YAML::Block << YAML::Key;
            {
                // Attributes as complex key
                auto attrs = Map(o) << YAML::Flow;
                for (auto const& attr : node.attributes()) o << attr;
            }
            // value will be the sequence of children
            o << YAML::Value;
            auto children = Seq(o);
            for (auto const& child : node.children())
                Convert(o, child, args);
        }
    }
}
