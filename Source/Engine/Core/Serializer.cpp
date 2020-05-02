#include "Serializer.h"

#include "TypeSystem.h"

void RecurseSerializeComposites(eastl::string& outString, Variant value, eastl::string indent)
{
    eastl::string localIndent = indent;
    for (Member& member : *value.pTypeData)
    {
        outString.append(localIndent);
        if (member.IsType<bool>())
        {   
            bool val = *member.Get<bool>(value);
            outString.append_sprintf("%s %s\n", member.name, val ? "true" : "false");
        }
        else if (member.IsType<int>())
        {
            outString.append_sprintf("%s %i\n", member.name, *member.Get<int>(value));
        }
        else if (member.IsType<float>())
        {
            outString.append_sprintf("%s %.9g\n", member.name, *member.Get<float>(value));
        }
        else if (member.IsType<double>())
        {
            outString.append_sprintf("%s %.17g\n", member.name, *member.Get<double>(value));
        }
        else if (member.IsType<eastl::string>())
        {
            outString.append_sprintf("%s \"%s\"\n", member.name, member.Get<eastl::string>(value)->c_str());
        }
        else
        {
            // Composite Type
            outString.append_sprintf("%s\n", member.name);
            RecurseSerializeComposites(outString, member.Get(value), indent.append("  "));
        }
    }
}

eastl::string Serializer::Serialize(Variant value)
{
    eastl::string result;
    TypeData& type = value.GetType();
    result.append_sprintf("[%s]\n", type.name);
    RecurseSerializeComposites(result, value, "");
    return result;
}

Variant Serializer::DeSerialize(eastl::string value)
{
    return Variant();
}