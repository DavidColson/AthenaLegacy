#include "TypeSystem.h"
#include "Log.h"
#include "Scanning.h"
#include "Scene.h"
#include "AssetDatabase.h"

#include <EASTL/stack.h>
#include <EASTL/vector.h>

eastl::string TypeData::Serialize(Variant value)
{
    eastl::string outString;
    for (Member& member : *value.pTypeData)
    {
        eastl::string serialized = member.GetType().Serialize(member.Get(value));
        
        // If we're a composite type, we need to indent our value
        if (member.GetType().members.size() > 0)
        {
            // Add indentation for this member
            serialized.insert(0, "\n");
            for (int i = 0; i < serialized.length(); i++)
            {
                if (serialized[i] == '\n' && i < serialized.length() - 2)
                    serialized.insert(i + 1, "  ");
            }
        }
        outString.append_sprintf("%s: %s", member.name, serialized.c_str());

        // New line if you're not a composite type
        if (member.GetType().members.size() == 0)
            outString += '\n';
    }
    return outString;
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

Variant TypeData::Parse(eastl::string_view value)
{
    Scan::ScanningState scan;
	scan.file = value;
	scan.current = 0;
	scan.line = 1;

    Scan::AdvanceOverWhitespaceNoNewline(scan);
    Variant var = New();

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

        // @Improvement: Consider giving errors for wrong indendation
        Scan::AdvanceOverWhitespaceNoNewline(scan); // Ignore all leading whitespcae (including indentation)

        // first, parse the identifier and the colon
        eastl::string memberName = ParseIdentifier(scan, ':');
        Scan::Advance(scan);
        Scan::AdvanceOverWhitespaceNoNewline(scan);
        Member& mem = GetMember(memberName.c_str());

        start = scan.current;
        if (mem.GetType().members.size() == 0)
        {
            while (Scan::Peek(scan) != '\n')
                Scan::Advance(scan);
        }
        else
        {
            // For composite types, it's awkward. Effectively need to count the number of lines until it matches the number of members of that composite type.
            // Then we can just give it to the inner parse function. Leading whitespace will be ignored

            // This will iterate through all members, and all child members to work out how many lines this type is taking up
            int targetLines = 0;
            eastl::stack<Member*> path;
            path.push(&mem);
            while(!path.empty())
            {
                Member& top = *path.top();
                path.pop();
                if (top.GetType().members.size() == 0)
                {
                    targetLines += 1;
                }
                else
                {   
                    targetLines += 1;
                    for (Member& innerMem : top.GetType())
                    {
                        path.push(&innerMem);
                    }
                }
            }

            // composite members start with a newline, so skip it
            start = scan.current + 1;
            int startLine = scan.line + 1;
            
            while (!Scan::IsAtEnd(scan) && (scan.line - startLine) < targetLines)
                Scan::Advance(scan);
        }
        eastl::string_view fileView = scan.file;
        eastl::string_view valueString = fileView.substr(start, scan.current - start);

        Variant parsed = mem.GetType().Parse(valueString);
        mem.Set(var, parsed);

        // Advance over end of line onto our next line
        Scan::AdvanceOverWhitespaceNoNewline(scan);
        Scan::Advance(scan);
    }
    return var;
}