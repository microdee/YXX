#include "yxx.h"

namespace yxx
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    YAML::Emitter& operator << (YAML::Emitter& out, pugi::xml_attribute const& v)
    {
        return out << YAML::Key << v.name() << YAML::Value << v.as_string();
    }

    class Map
    {
        YAML::Emitter& Out;

    public:
        Map(YAML::Emitter& out) : Out(out)
        {
            out << YAML::BeginMap;
        }

        ~Map()
        {
            Out << YAML::EndMap;
        }

        template <typename T>
        Map& operator << (T&& rhs)
        {
            Out << std::forward<T>(rhs);
            return *this;
        }
    };

    class Seq
    {
        YAML::Emitter& Out;

    public:
        Seq(YAML::Emitter& out) : Out(out)
        {
            out << YAML::BeginSeq;
        }

        ~Seq()
        {
            Out << YAML::EndSeq;
        }
    };

    void Convert(YAML::Emitter& o, pugi::xml_node const& node)
    {
        // XML declaration nodes are conserved with `/?xml` key. In YXX this is formatted as a regular tag.
        // Other processing instruction nodes are ignored as they are uncommon and don't have XML compliant format. 
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

        if (node.type() == pugi::node_cdata || node.type() == pugi::node_pcdata)
        {
            o << node.value();
            return;
        }

        // TODO:
    }
}
