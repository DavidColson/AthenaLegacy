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
            outString.append_sprintf("%s: %s\n", member.name, val ? "true" : "false");
        }
        else if (member.IsType<int>())
        {
            outString.append_sprintf("%s: %i\n", member.name, *member.Get<int>(value));
        }
        else if (member.IsType<float>())
        {
            outString.append_sprintf("%s: %.9g\n", member.name, *member.Get<float>(value));
        }
        else if (member.IsType<double>())
        {
            outString.append_sprintf("%s: %.17g\n", member.name, *member.Get<double>(value));
        }
        else if (member.IsType<eastl::string>())
        {
            outString.append_sprintf("%s: \"%s\"\n", member.name, member.Get<eastl::string>(value)->c_str());
        }
        else
        {
            // Composite Type
            outString.append_sprintf("%s:\n", member.name);
            RecurseSerializeComposites(outString, member.Get(value), indent.append("  "));
        }
    }
}

eastl::string Serializer::Serialize(Variant value)
{
    eastl::string result;
    TypeData& type = value.GetType();
    result.append_sprintf("[%s]\n", type.name);

    if (type == TypeDatabase::Get<bool>())
    {   
        bool val = value.GetValue<bool>();
        result.append_sprintf("%s\n", val ? "true" : "false");
    }
    else if (type == TypeDatabase::Get<int>())
    {
        result.append_sprintf("%i\n", value.GetValue<int>());
    }
    else if (type == TypeDatabase::Get<float>())
    {
        result.append_sprintf("%.9g\n", value.GetValue<float>());
    }
    else if (type == TypeDatabase::Get<double>())
    {
        result.append_sprintf("%.17g\n", value.GetValue<double>());
    }
    else if (type == TypeDatabase::Get<eastl::string>())
    {
        result.append_sprintf("\"%s\"\n", value.GetValue<eastl::string>().c_str());
    }
    else
    {
        // Composite Type
        RecurseSerializeComposites(result, value, "");
    }

    RecurseSerializeComposites(result, value, "");
    return result;
}

Variant Serializer::DeSerialize(eastl::string value)
{
    // Similar parsing utilities as in the json parser
    
    // Parse for the opening square bracket and then parse till the end to get the name of the type. Instance it immediately

    // Consume newline character (error if none)
    
    // Parse all chars until a space, take as string and use to search for member

    // Parse until colon, then parse until end of line to get value

    // If searched for member is composite, then consume rest of the line

    // Recurse into the next few lines until the indentation level backtracks

    return Variant();
}