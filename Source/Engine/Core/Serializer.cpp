#include "Serializer.h"

#include "TypeSystem.h"
#include "Log.h"
#include "Scanning.h"
#include "Scene.h"
#include "AssetDatabase.h"

#include <EASTL/vector.h>

void RecurseSerializeComposites(eastl::string& outString, Variant value, eastl::string indent)
{
    eastl::string localIndent = indent;
    for (Member& member : *value.pTypeData)
    {
        outString.append(localIndent);
        if (member.IsType<bool>())
        {   
            bool val = member.GetAs<bool>(value);
            outString.append_sprintf("%s: %s\n", member.name, val ? "true" : "false");
        }
        else if (member.IsType<int>())
        {
            outString.append_sprintf("%s: %i\n", member.name, member.GetAs<int>(value));
        }
        else if (member.IsType<float>())
        {
            outString.append_sprintf("%s: %.9g\n", member.name, member.GetAs<float>(value));
        }
        else if (member.IsType<double>())
        {
            outString.append_sprintf("%s: %.17g\n", member.name, member.GetAs<double>(value));
        }
        else if (member.IsType<eastl::string>())
        {
            outString.append_sprintf("%s: \"%s\"\n", member.name, member.GetAs<eastl::string>(value).c_str());
        }
        else if (member.IsType<EntityID>())
        {
            outString.append_sprintf("%s: EntityID{ %i }\n", member.name, member.GetAs<EntityID>(value).Index());
        }
        else if (member.IsType<AssetHandle>())
        {
            AssetHandle handle = member.GetAs<AssetHandle>(value);
            outString.append_sprintf("%s: AssetHandle(\"%s\")\n", member.name, AssetDB::GetAssetIdentifier(handle).c_str());
        }
        else
        {
            // Composite Type
            outString.append_sprintf("%s:\n", member.name);
            eastl::string indentation = indent;
            indentation.append("  ");
            RecurseSerializeComposites(outString, member.Get(value), indentation);
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
    else if (type == TypeDatabase::Get<AssetHandle>())
    {
        result.append_sprintf("AssetHandle(\"%s\")\n", AssetDB::GetAssetIdentifier(value.GetValue<AssetHandle>()).c_str());
    }
    else
    {
        // Composite Type
        RecurseSerializeComposites(result, value, "");
    }
    return result;
}

eastl::string ParseIdentifier(Scan::ScanningState& scan, char endChar)
{	
	int start = scan.current;
	eastl::string result;
	while (Scan::Peek(scan) != endChar && !Scan::IsAtEnd(scan))
	{
		char c = Scan::Advance(scan);
		
		switch (c)
		{
		case '\0':
		case '\t':
		case '\v':
		case '\b':
		case '\\':
		case '\f':
        case '"':
			Scan::HandleError(scan, "Invalid character in string", scan.current-1); break;
		case '\r':
		case '\n':
			Scan::HandleError(scan, "Unexpected end of line", scan.current-1); break;
		default:
            if (Scan::IsWhitespace(c))
    			Scan::HandleError(scan, "Unexpected whitespace", scan.current-1); break;
			break;
		}
		result += c;
	}
	return result;
}

eastl::string ParseTypeDBString(Scan::ScanningState& scan)
{	
	int start = scan.current;
	Scan::Advance(scan); // advance over initial '"'
	eastl::string result;
	while (Scan::Peek(scan) != '"' && !Scan::IsAtEnd(scan))
	{
		char c = Advance(scan);
		
		switch (c)
		{
		case '\0':
		case '\t':
		case '\b':
		case '\\':
			Scan::HandleError(scan, "Invalid character in string", scan.current-1); break;
		case '\r':
		case '\n':
			Scan::HandleError(scan, "Unexpected end of line, please keep whole strings on one line", scan.current-1); break;
		default:
			break;
		}

		if (c == '\\')
		{
			char next = Advance(scan);
			switch (next)
			{
			case '"': result += '"'; break;
			case '\\':result += '\\'; break;
			case '/': result += '/'; break;
			case 'b': result += '\b'; break;
			case 'f': result += '\f'; break;
			case 'n': result += '\n'; break;
			case 'r': result += '\r'; break;
			case 't': result += '\t'; break;
			case 'u':
				Scan::HandleError(scan, "This parser does not yet support unicode escape codes", scan.current - 1); break;
			default:
				Scan::HandleError(scan, "Disallowed escape character or none provided", scan.current - 1); break;
			}
		}
		else
			result += c;
	}
	Scan::Advance(scan);
	return result;
}

int ParseInt(Scan::ScanningState& scan)
{	
    // @Incomplete, give appropriate errors on unexpected characters
	int start = scan.current;
	while (Scan::IsPartOfNumber(Peek(scan)))
	{
		Scan::Advance(scan);
	}
	return strtol(scan.file.substr(start, (scan.current - start)).c_str(), nullptr, 10) ;
}

double ParseFloat(Scan::ScanningState& scan)
{	
    // @Incomplete, give appropriate errors on unexpected characters
	int start = scan.current;
	while (Scan::IsPartOfNumber(Peek(scan)))
	{
		Scan::Advance(scan);
	}
    if (Scan::Peek(scan) == '.' && Scan::IsPartOfNumber(PeekNext(scan)))
	{
		Scan::Advance(scan);
		while (Scan::IsPartOfNumber(Peek(scan)))
		{
			Scan::Advance(scan);
		}
	}
	return strtod(scan.file.substr(start, (scan.current - start)).c_str(), nullptr);
}

bool ParseBoolean(Scan::ScanningState& scan)
{
	int start = scan.current;

    if (Scan::Peek(scan) == 't')
    {
        for (int i = 0; i < 4; i++)
		    Scan::Advance(scan);
        
        if (scan.file.substr(start, 4) == "true")
            return true;
    }
    if (Scan::Peek(scan) == 'f')
    {
        for (int i = 0; i < 5; i++)
		    Scan::Advance(scan);
        
        if (scan.file.substr(start, 5) == "false")
            return true;
    }
    else
    {
	    Scan::HandleError(scan, "Expected 'true' or 'false'", start);
    }

	return false;
}

AssetHandle ParseAssetHandle(Scan::ScanningState& scan)
{
	int start = scan.current;

    if (Scan::Peek(scan) == 'A')
    {
        for (int i = 0; i < 12; i++)
		    Scan::Advance(scan);
        
        if (scan.file.substr(start, 12) != "AssetHandle(")
        {
            Scan::HandleError(scan, "Expected \"AssetHandle(\" here", start);
            return AssetHandle();
        }

        eastl::string identifier = ParseTypeDBString(scan);

        if (Scan::Advance(scan) != ')')
        {
            Scan::HandleError(scan, "Expected end of asset handle \")\"", scan.current);
        }

        return AssetHandle(identifier);
    }
    else
    {
        return AssetHandle();
    }
}

Variant ParseComposite(Scan::ScanningState& scan, TypeData& type, int indentLevel)
{
    Variant var = type.New();
    if (!type.members.empty())
    {
        while (!Scan::IsAtEnd(scan) && !scan.encounteredError)
        {   
            // Check if this is a blank line, and if so skip it
            int start = scan.current;
            while (Scan::Peek(scan) == ' ')
                Scan::Advance(scan);

            if (Scan::Peek(scan) == '\n')
            {
                Scan::Advance(scan); // Consume that newline character
                continue;
            }

            scan.current = start;


            // Deal with indentation characters (always spaces)
            int expectedIndentChars = 2 * indentLevel;
            int actualIndentCars = 0;
            for (int i = 0; i < expectedIndentChars; i++)
            {
                if (Scan::Advance(scan) == ' ')
                    actualIndentCars++;
            }

            if (actualIndentCars % 2 != 0)
                Scan::HandleError(scan, "Incorrect indentation", scan.current);
            if (actualIndentCars < expectedIndentChars)
            {
                scan.current -= expectedIndentChars; // Walking backward to return the indentation to what it's supposed to be
                break;
            }

            // Parse out the identifier for this member variable
            eastl::string memberName = ParseIdentifier(scan, ':');
            if (!type.MemberExists(memberName.c_str()))
            {
                Scan::HandleError(scan, "Unknown member variable specified", scan.current - (int)memberName.size());
                break;
            }

            Scan::Advance(scan); // consume ':'
            Scan::AdvanceOverWhitespaceNoNewline(scan);
            Member& mem = type.GetMember(memberName.c_str());

            if (Scan::Peek(scan) == '\n' && mem.GetType().members.empty())
                Scan::HandleError(scan, "Newline after member identifier not allowed", scan.current);

            // Actually parse the value
            if (mem.IsType<bool>())
            {
                mem.Set(var, ParseBoolean(scan));
            }
            else if (mem.IsType<int>())
            {
                mem.Set(var, ParseInt(scan));
            }
            else if (mem.IsType<float>())
            {
                mem.Set(var, (float)ParseFloat(scan));
            }
            else if (mem.IsType<double>())
            {
                mem.Set(var, ParseFloat(scan));
            }
            else if (mem.IsType<eastl::string>())
            {
                mem.Set(var, ParseTypeDBString(scan));
            }
            else if (mem.IsType<AssetHandle>())
            {
                mem.Set(var, ParseAssetHandle(scan));
            }
            else
            {
                Scan::Advance(scan); // Advance over newline
                Variant varInner = ParseComposite(scan, mem.GetType(), indentLevel + 1);
                mem.Set(var, varInner);
                continue;
            }

            // Advance over end of line onto our next line
            Scan::AdvanceOverWhitespaceNoNewline(scan);
            if (Scan::Peek(scan) != '\n')
            {
                Scan::HandleError(scan, "Expected new line character before continuing to parse members", scan.current);
                return var;
            }
            Scan::Advance(scan);
        }
    }
    return var;
}

Variant Serializer::DeSerialize(eastl::string value)
{
    Scan::ScanningState scan;
	scan.file = value;
	scan.current = 0;
	scan.line = 1;
    
    // Parse for the opening square bracket and then parse till the end to get the name of the type. Instance it immediately
    if (Scan::Peek(scan) == '[')
    {
        Scan::Advance(scan); // consume '['
        eastl::string typeName = ParseIdentifier(scan, ']');
        Scan::Advance(scan); // consume ']'
        
        if (!TypeDatabase::TypeExists(typeName.c_str()))
            return Variant();

        TypeData& type = TypeDatabase::GetFromString(typeName.c_str());

        Scan::AdvanceOverWhitespaceNoNewline(scan);
        Scan::Advance(scan); // Advance over newline


        Variant output;

        // If we have members, parse for identifiers
        if (!type.members.empty())
        {
            output = ParseComposite(scan, type, 0);
        }
        else
        {
            if (type == TypeDatabase::Get<bool>())
            {
                output = ParseBoolean(scan);
            }
            else if (type == TypeDatabase::Get<int>())
            {
                output = ParseInt(scan);
            }
            else if (type == TypeDatabase::Get<float>())
            {
                output = (float)ParseFloat(scan);
            }
            else if (type == TypeDatabase::Get<double>())
            {
                output = ParseFloat(scan);
            }
            else if (type == TypeDatabase::Get<eastl::string>())
            {
                output = ParseTypeDBString(scan);
            }
            else if (type == TypeDatabase::Get<AssetHandle>())
            {
                output = ParseAssetHandle(scan);
            }
        }
        return output;
    }
    else
    {
        Scan::HandleError(scan, "No top level type deceleration exists", scan.current);
    }

    return Variant();
}