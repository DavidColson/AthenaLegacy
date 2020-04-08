#include "Json.h"

#include "Log.h"
#include "ErrorHandling.h"

// JsonValue implementation
///////////////////////////

JsonValue::~JsonValue()
{
	if (type == Type::Array)
		delete internalData.pArray;	
	else if (type == Type::Object)
		delete internalData.pObject;
	else if (type == Type::String)
	{
		Log::Debug("Deleting a jsonvalue string with value %s", internalData.pString->c_str());
		delete internalData.pString;
	}
}

JsonValue::JsonValue()
{
    type = Type::Null;
    internalData.pArray = nullptr;
}

JsonValue::JsonValue(const JsonValue& copy)
{
	// Clear out internal stuff
	if (type == Type::Array)
		delete internalData.pArray;	
	else if (type == Type::Object)
		delete internalData.pObject;
	else if (type == Type::String)
		delete internalData.pString;

	// Copy data from the other value
	switch (copy.type)
	{
	case Type::Array:
		internalData.pArray = new eastl::vector<JsonValue>(copy.internalData.pArray->begin(), copy.internalData.pArray->end());
		break;
	case Type::Object:
		internalData.pObject = new eastl::map<eastl::string, JsonValue>(copy.internalData.pObject->begin(), copy.internalData.pObject->end());
		break;
	case Type::String:
   		internalData.pString = new eastl::string(*(copy.internalData.pString));
	default:
		internalData = copy.internalData;
		break;
	}
	type = copy.type;
}

JsonValue::JsonValue(JsonValue&& copy)
{
	internalData = copy.internalData;
	copy.internalData.pArray = nullptr;

	type = copy.type;
	copy.type = Type::Null;	
}

JsonValue& JsonValue::operator=(const JsonValue& copy)
{
	// Clear out internal stuff
	if (type == Type::Array)
		delete internalData.pArray;	
	else if (type == Type::Object)
		delete internalData.pObject;
	else if (type == Type::String)
		delete internalData.pString;

	// Copy data from the other value
	switch (copy.type)
	{
	case Type::Array:
		internalData.pArray = new eastl::vector<JsonValue>(copy.internalData.pArray->begin(), copy.internalData.pArray->end());
		break;
	case Type::Object:
		internalData.pObject = new eastl::map<eastl::string, JsonValue>(copy.internalData.pObject->begin(), copy.internalData.pObject->end());
		break;
	case Type::String:
   		internalData.pString = new eastl::string(*(copy.internalData.pString));
	default:
		internalData = copy.internalData;
		break;
	}
	type = copy.type;

	return *this;
}

JsonValue& JsonValue::operator=(JsonValue&& copy)
{
	internalData = copy.internalData;
	copy.internalData.pArray = nullptr;

	type = copy.type;
	copy.type = Type::Null;	

	return *this;
}

JsonValue::JsonValue(eastl::vector<JsonValue>& array)
{
    internalData.pArray = nullptr;
    internalData.pArray = new eastl::vector<JsonValue>(array.begin(), array.end());
    type = Type::Array;
}

JsonValue::JsonValue(eastl::map<eastl::string, JsonValue>& object)
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

JsonValue::JsonValue(const char* string)
{
    internalData.pArray = nullptr;
    internalData.pString = new eastl::string(string);
    type = Type::String;
}

JsonValue::JsonValue(double number)
{
    internalData.pArray = nullptr;
    internalData.floatingNumber = number;
    type = Type::Floating;
}

JsonValue::JsonValue(long number)
{
    internalData.pArray = nullptr;
    internalData.integerNumber = number;
    type = Type::Integer;
}

JsonValue::JsonValue(bool boolean)
{
    internalData.pArray = nullptr;
    internalData.boolean = boolean;
    type = Type::Boolean;
}

eastl::string JsonValue::ToString()
{
	if (type == Type::String)
		return *(internalData.pString);
	return eastl::string();
}

double JsonValue::ToFloat()
{
	if (type == Type::Floating)
		return internalData.floatingNumber;
	return 0.0f;
}

long JsonValue::ToInt()
{
	if (type == Type::Integer)
		return internalData.integerNumber;
	return 0;
}

bool JsonValue::ToBool()
{
	if (type == Type::Boolean)
		return internalData.boolean;
	return 0;
}

bool JsonValue::IsNull()
{
	return type == Type::Null;
}

bool JsonValue::HasKey(eastl::string identifier)
{
	return internalData.pObject->count(identifier) >= 1;
}

JsonValue& JsonValue::operator[](eastl::string identifier)
{
	ASSERT(type == Type::Object, "Attempting to treat this value as an object when it is not.");
	return internalData.pObject->operator[](identifier);
}

JsonValue& JsonValue::operator[](size_t index)
{
	ASSERT(type == Type::Array, "Attempting to treat this value as an array when it is not.");
	ASSERT(internalData.pArray->size() > index, "Accessing an element that does not exist in this array, you probably need to append");
	return internalData.pArray->operator[](index);
}

JsonValue JsonValue::NewObject()
{
	return JsonValue(eastl::map<eastl::string, JsonValue>());
}

JsonValue JsonValue::NewArray()
{
	return JsonValue(eastl::vector<JsonValue>());
}

// Scanning Utilities
/////////////////////

struct ScanningState
{
	eastl::string file;
	int current{ 0 };
	int currentLineStart{ 0 };
	int line{ 1 };
	bool encounteredError{ false };
};

char Advance(ScanningState& scan)
{
	scan.current++;
	if (scan.file[scan.current - 1] == '\n')
	{
		scan.line++;
		scan.currentLineStart = scan.current;	
	}
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
		{
			scan.line++;
			scan.currentLineStart = scan.current + 1;
		}
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

eastl::string ExtractLineWithError(ScanningState& scan, int errorAt)
{
	// We give back the last two lines before the error, in case of cascading errors
	eastl::vector<eastl::string> lines;

	// Find the end of the line in which the error occured
	int lineEnd = errorAt;
	while (lineEnd < scan.file.size())
	{
		if (scan.file[lineEnd] == '\n')
		{
			break;
		}
		lineEnd++;
	}

	// Count backward finding the last 2 lines
	int lineStart = errorAt;

	while (lineStart >= 0 && lines.size() < 2)
	{
		if (scan.file[lineStart] == '\n')
		{
			// We use +1 and -1 to trim the newline characters
			lines.push_back(scan.file.substr(lineStart + 1, (lineEnd - lineStart)-1));
			lineEnd = lineStart + 1;
		}
		lineStart--;
	}
	
	eastl::string error;
	for (int i = (int)lines.size() - 1; i >= 0; i--)
	{
		error.append_sprintf("%5i|%s", scan.line - i, lines[i].c_str());
	}
	error += "\n";
	return error;
}

void HandleError(ScanningState& scan, const char* message, int location)
{
	if (!scan.encounteredError)
	{
		eastl::string errorDiagram = ExtractLineWithError(scan, location);

		int columnToPointTo = (location - scan.currentLineStart) + 6; // 6 to account for the padded line number text
		for(int i = 0; i < columnToPointTo; i++)
			errorDiagram += ' ';
		errorDiagram.append_sprintf("^----- %s", message);

		Log::Crit("Encountered Parsing Error on line %i: \n%s\n", 
			scan.line, 
			errorDiagram.c_str()
			);
		scan.encounteredError = true;
	}
}

// Value Parsing
////////////////

eastl::vector<JsonValue> ParseArray(ScanningState& scan);
eastl::map<eastl::string, JsonValue> ParseObject(ScanningState& scan);

JsonValue ParseNumber(ScanningState& scan)
{	
	int start = scan.current;
	while (IsPartOfNumber(Peek(scan)))
	{
		Advance(scan);
	}

	bool hasFraction = false;
	if (Peek(scan) == '.' && IsPartOfNumber(PeekNext(scan)))
	{
		Advance(scan);
		while (IsPartOfNumber(Peek(scan)))
		{
			Advance(scan);
		}
		hasFraction = true;
	}

	long exponent = 0;
	if (Peek(scan) == 'e' || Peek(scan) == 'E')
	{
		Advance(scan); // advance past e or E
		if (!(IsPartOfNumber(Peek(scan)) || Peek(scan) == '+'))
			HandleError(scan, "Expected digits for the exponent, none provided", scan.current);

		int exponentStart = scan.current;
		while (IsPartOfNumber(Peek(scan)) || Peek(scan) == '+')
		{
			Advance(scan);
		}
		exponent = strtol(scan.file.substr(exponentStart, (scan.current - exponentStart)).c_str(), nullptr, 10);
	}

	if (hasFraction)
		return strtod(scan.file.substr(start, (scan.current - start)).c_str(), nullptr);
	
	return strtol(scan.file.substr(start, (scan.current - start)).c_str(), nullptr, 10) * (long)pow(10, exponent);
}

eastl::string ParseString(ScanningState& scan)
{	
	// @Incomplete disallow unsupported escape characters
	// @Incomplete control characters such as line breaks are not allowed here, but we currently read them
	// @Incomplete interpret "\n" as a control character along with other control characters and convert to actual chars
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

	HandleError(scan, "Expected 'null'", start);
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

	HandleError(scan, "Expected 'true'", start);
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

	HandleError(scan, "Expected 'false'", start);
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
		else
			HandleError(scan, "Unknown value, please give a known value", scan.current);
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
			HandleError(scan, "Expected a ',' for next array element, or ']' to end the array", scan.current);
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
		
		if (Peek(scan) != '"')
			HandleError(scan, "Expected '\"' to start a new key", scan.current);
		eastl::string key = ParseString(scan);

		AdvanceOverWhitespace(scan);
		if (Advance(scan) != ':')
		{
			HandleError(scan, "Expected a key value separator here, ':'", scan.current - 1);
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
			HandleError(scan, "Expected a ',' for next object element, or '}' to end the object", scan.current);
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
	JsonValue json = ParseValue(scan);
	if (scan.encounteredError)
		return JsonValue();
    return json;
}