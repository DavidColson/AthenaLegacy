#include "Json.h"

#include "Log.h"

eastl::vector<JsonValue> ParseArray(eastl::string& file, int& current);
eastl::map<eastl::string, JsonValue> ParseObject(eastl::string& file, int& current);

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

char Advance(eastl::string& file, int& current)
{
	current++;
	return file[current - 1];
}

char Peek(eastl::string& file, const int& current)
{
	return file[current];
}

bool IsWhitespace(char c)
{
	if(c == ' ' || c == '\r' || c == '\t' || c == '\n')
		return true;
	return false;
}

void AdvanceOverWhitespace(eastl::string &file, int& current)
{
	char c = file[current];
	while (IsWhitespace(c))
	{
		Advance(file, current);
		c = file[current];
	}
}

bool IsAtEnd(eastl::string& file, const int& current)
{
	return current >= file.size();
}

bool IsPartOfNumber(char c)
{
	return (c >= '0' && c <= '9') || c == '-';
}

// Value Parsing
////////////////

double ParseNumber(eastl::string& file, int& current)
{	
	int start = current;

	while (IsPartOfNumber(Peek(file, current)))
	{
		Advance(file, current);
	}

	if (Peek(file, current) == '.' && IsPartOfNumber(Peek(file, current + 1)))
	{
		Advance(file, current);
		while (IsPartOfNumber(Peek(file, current)))
		{
			Advance(file, current);
		}
	}
	return strtod(file.substr(start, (current - start)).c_str(), nullptr);
}

eastl::string ParseString(eastl::string& file, int& current)
{
	int start = current;
	Advance(file, current); // advance over initial '"'
	while (Peek(file, current) != '"' && !IsAtEnd(file, current))
	{
		Advance(file, current);
	}
	Advance(file, current);
	return file.substr(start + 1, (current - start) - 2);
}

bool ParseNull(eastl::string& file, int& current)
{
	int start = current;
	for (int i = 0; i < 4; i++)
		Advance(file, current);

	if (file.substr(start, 4) == "null")
	{
		return true;
	}
	return false;
}

bool ParseTrue(eastl::string& file, int& current)
{
	int start = current;
	for (int i = 0; i < 4; i++)
		Advance(file, current);

	if (file.substr(start, 4) == "true")
	{
		return true;
	}

	return false;
}

bool ParseFalse(eastl::string& file, int& current)
{
	int start = current;
	for (int i = 0; i < 5; i++)
		Advance(file, current);

	if (file.substr(start, 5) == "false")
	{
		return true;
	}
	return false;
}

JsonValue ParseValue(eastl::string& file, int& current)
{
	switch (file[current])
	{
	case '{':
		return ParseObject(file, current);
		break;
	case '[':
		return ParseArray(file, current);
		break;
	case '"':
		return ParseString(file, current);
		break;
	case 'n':
		if (ParseNull(file, current))
			return JsonValue();
		break;
	case 't':
		if (ParseTrue(file, current))
			return JsonValue(true);
		break;
	case 'f':
		if (ParseFalse(file, current))
			return JsonValue(false);
		break;
	default:
		if (IsPartOfNumber(file[current]))
			return ParseNumber(file, current);
		break;
	}
	return JsonValue();
}

eastl::vector<JsonValue> ParseArray(eastl::string& file, int& current)
{
	eastl::vector<JsonValue> array;
	Advance(file, current); // advance past opening '['

	while (!IsAtEnd(file, current) && Peek(file, current) != ']')
	{
		AdvanceOverWhitespace(file, current);
		array.push_back(ParseValue(file, current));	
		AdvanceOverWhitespace(file, current);

		if (Peek(file, current) != ',' && Peek(file, current) != ']')
		{
			Log::Crit("Missing array element separator");
			return array;
		}

		char nextC = Peek(file, current);
		if (nextC == ',')
			Advance(file, current);
		else if (nextC == ']')
			continue;
		else
		{
			Log::Crit("Missing array element separator");
			return array;
		}
	}
	AdvanceOverWhitespace(file, current);
	Advance(file, current); // Advance over closing ']'
	return array;
}

eastl::map<eastl::string, JsonValue> ParseObject(eastl::string& file, int& current)
{
	eastl::map<eastl::string, JsonValue> map;
	Advance(file, current); // advance past opening '{'

	while (!IsAtEnd(file, current) && Peek(file, current) != '}')
	{
		AdvanceOverWhitespace(file, current);
		// @Incomplete check for opening '"' and error otherwise
		eastl::string key = ParseString(file, current);

		AdvanceOverWhitespace(file, current);
		if (Advance(file, current) != ':')
		{
			Log::Crit("Missing key value separator");
			return map;
		}
		AdvanceOverWhitespace(file, current);

		map[key] = ParseValue(file, current);
		AdvanceOverWhitespace(file, current);

		char nextC = Peek(file, current);
		if (nextC == ',')
			Advance(file, current);
		else if (nextC == '}')
			continue;
		else
		{
			Log::Crit("Missing object element separator");
			return map;
		}
	}
	AdvanceOverWhitespace(file, current);
	Advance(file, current); // Advance over closing '}'
	return map;
}

JsonValue ParseJsonFile(eastl::string& file)
{
    int current = 0;
	AdvanceOverWhitespace(file, current);
    return ParseValue(file, current);
}