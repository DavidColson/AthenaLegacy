#include "Json.h"

#include "Log.h"

// JsonValue implementation
///////////////////////////

JsonValue::JsonValue()
{
    type = Type::Null;
    internalData.pArray = nullptr;
}

JsonValue::JsonValue(eastl::vector<JsonValue>& array)
{
    internalData.pArray = nullptr;
    internalData.pArray = new eastl::vector<JsonValue>(array.begin(), array.end());
    type = Type::Array;
}

JsonValue::JsonValue(eastl::map<eastl::string, JsonValue> object)
{
    internalData.pArray = nullptr;
    internalData.pObject = new eastl::map<eastl::string, JsonValue>(object.begin(), object.end());
    type = Type::Object;
}

JsonValue::JsonValue(eastl::string string)
{
    internalData.pArray = nullptr;
    internalData.pString = new eastl::string(string);
    type = Type::String;
}

JsonValue::JsonValue(double number)
{
    internalData.pArray = nullptr;
    internalData.floatingNumber = number;
    type = Type::Number;
}

JsonValue::JsonValue(bool boolean)
{
    internalData.pArray = nullptr;
    internalData.boolean = boolean;
    type = Type::Boolean;
}

// Scanning Utilities
/////////////////////

struct ScanningState
{
	eastl::string file;
	int current{ 0 };
	int line{ 1 };
};

char Advance(ScanningState& scan)
{
	scan.current++;
	return scan.file[scan.current - 1];
}

char Peek(ScanningState& scan)
{
	return scan.file[scan.current];
}

char PeekNext(ScanningState& scan)
{
	return scan.file[scan.current + 1];
}

bool IsWhitespace(char c)
{
	if(c == ' ' || c == '\r' || c == '\t' || c == '\n')
		return true;
	return false;
}

void AdvanceOverWhitespace(ScanningState& scan)
{
	char c = scan.file[scan.current];
	while (IsWhitespace(c))
	{
		Advance(scan);
		c = Peek(scan);
		if (c == '\n')
			scan.line++;
	}
}

bool IsAtEnd(ScanningState& scan)
{
	return scan.current >= scan.file.size();
}

bool IsPartOfNumber(char c)
{
	return (c >= '0' && c <= '9') || c == '-';
}

// Error reporting
//////////////////

eastl::string ExtractErrorPointer(ScanningState& scan, int errorAt, const char* message)
{
	int lineStart = errorAt;
	while (lineStart >= 0)
	{
		if (scan.file[lineStart] == '\n')
		{
			lineStart += 1;
			break;
		}
		lineStart--;
	}
	int lineEnd = errorAt;
	while (lineEnd < scan.file.size())
	{
		if (scan.file[lineEnd] == '\n')
		{
			break;
		}
		lineEnd++;
	}
	eastl::string error;
	
	error.append_sprintf("%i |", scan.line);
	int startChars = (int)error.size();

	error.append(scan.file.substr(lineStart, (lineEnd - lineStart)-1));
	error += "\n";

	for(int i = 0; i < (errorAt - lineStart) + startChars; i++)
		error += ' ';

	error.append_sprintf("^----- %s", message);
	return error;
}

// Value Parsing
////////////////

eastl::vector<JsonValue> ParseArray(ScanningState& scan);
eastl::map<eastl::string, JsonValue> ParseObject(ScanningState& scan);

double ParseNumber(ScanningState& scan)
{	
	int start = scan.current;

	while (IsPartOfNumber(Peek(scan)))
	{
		Advance(scan);
	}

	if (Peek(scan) == '.' && IsPartOfNumber(PeekNext(scan)))
	{
		Advance(scan);
		while (IsPartOfNumber(Peek(scan)))
		{
			Advance(scan);
		}
	}
	return strtod(scan.file.substr(start, (scan.current - start)).c_str(), nullptr);
}

eastl::string ParseString(ScanningState& scan)
{
	int start = scan.current;
	Advance(scan); // advance over initial '"'
	while (Peek(scan) != '"' && !IsAtEnd(scan))
	{
		Advance(scan);
	}
	Advance(scan);
	return scan.file.substr(start + 1, (scan.current - start) - 2);
}

bool ParseNull(ScanningState& scan)
{
	int start = scan.current;
	for (int i = 0; i < 4; i++)
		Advance(scan);

	if (scan.file.substr(start, 4) == "null")
	{
		return true;
	}
	return false;
}

bool ParseTrue(ScanningState& scan)
{
	int start = scan.current;
	for (int i = 0; i < 4; i++)
		Advance(scan);

	if (scan.file.substr(start, 4) == "true")
	{
		return true;
	}

	return false;
}

bool ParseFalse(ScanningState& scan)
{
	int start = scan.current;
	for (int i = 0; i < 5; i++)
		Advance(scan);

	if (scan.file.substr(start, 5) == "false")
	{
		return true;
	}
	return false;
}

JsonValue ParseValue(ScanningState& scan)
{
	switch (Peek(scan))
	{
	case '{':
		return ParseObject(scan);
		break;
	case '[':
		return ParseArray(scan);
		break;
	case '"':
		return ParseString(scan);
		break;
	case 'n':
		if (ParseNull(scan))
			return JsonValue();
		break;
	case 't':
		if (ParseTrue(scan))
			return JsonValue(true);
		break;
	case 'f':
		if (ParseFalse(scan))
			return JsonValue(false);
		break;
	default:
		if (IsPartOfNumber(Peek(scan)))
			return ParseNumber(scan);
		break;
	}
	return JsonValue();
}

eastl::vector<JsonValue> ParseArray(ScanningState& scan)
{
	eastl::vector<JsonValue> array;
	Advance(scan); // advance past opening '['

	while (!IsAtEnd(scan) && Peek(scan) != ']')
	{
		AdvanceOverWhitespace(scan);
		array.push_back(ParseValue(scan));	
		AdvanceOverWhitespace(scan);

		char nextC = Peek(scan);
		if (nextC == ',')
			Advance(scan);
		else if (nextC == ']')
			continue;
		else
		{
			Log::Crit("Missing array element separator (%i): \n%s\n", scan.line, ExtractErrorPointer(scan, scan.current, "Expected ',' or ']'").c_str());
			return array;
		}
	}
	AdvanceOverWhitespace(scan);
	Advance(scan); // Advance over closing ']'
	return array;
}

eastl::map<eastl::string, JsonValue> ParseObject(ScanningState& scan)
{
	eastl::map<eastl::string, JsonValue> map;
	Advance(scan); // advance past opening '{'

	while (!IsAtEnd(scan) && Peek(scan) != '}')
	{
		AdvanceOverWhitespace(scan);
		// @Incomplete check for opening '"' and error otherwise
		eastl::string key = ParseString(scan);

		AdvanceOverWhitespace(scan);
		if (Advance(scan) != ':')
		{
			Log::Crit("Missing Key-Value separator (%i): \n%s", scan.line, ExtractErrorPointer(scan, scan.current - 1, "Expected ':'").c_str());
			return map;
		}
		AdvanceOverWhitespace(scan);

		map[key] = ParseValue(scan);
		AdvanceOverWhitespace(scan);

		char nextC = Peek(scan);
		if (nextC == ',')
			Advance(scan);
		else if (nextC == '}')
			continue;
		else
		{
			Log::Crit("Missing object element separator or object end (%i): \n%s\n", scan.line, ExtractErrorPointer(scan, scan.current, "Expected ',' or '}'").c_str());
			return map;
		}
	}
	AdvanceOverWhitespace(scan);
	Advance(scan); // Advance over closing '}'
	return map;
}

JsonValue ParseJsonFile(eastl::string& file)
{
    ScanningState scan;
	scan.file = file;
	scan.current = 0;
	scan.line = 1;
	AdvanceOverWhitespace(scan);
    return ParseValue(scan);
}