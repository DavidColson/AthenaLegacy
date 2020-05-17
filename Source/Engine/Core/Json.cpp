#include "Json.h"

#include "Log.h"
#include "ErrorHandling.h"
#include "Scanning.h"

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

int JsonValue::Count()
{
	ASSERT(type == Type::Array || type == Type::Object, "Attempting to treat this value as an array or object when it is not.");
	if (type == Type::Array)
		return (int)internalData.pArray->size();
	else
		return (int)internalData.pObject->size();
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

// Value Parsing
////////////////

eastl::vector<JsonValue> ParseArray(Scan::ScanningState& scan);
eastl::map<eastl::string, JsonValue> ParseObject(Scan::ScanningState& scan);

JsonValue ParseNumber(Scan::ScanningState& scan)
{	
	int start = scan.current;
	while (Scan::IsPartOfNumber(Peek(scan)))
	{
		Scan::Advance(scan);
	}

	bool hasFraction = false;
	if (Scan::Peek(scan) == '.' && Scan::IsPartOfNumber(PeekNext(scan)))
	{
		Scan::Advance(scan);
		while (Scan::IsPartOfNumber(Peek(scan)))
		{
			Scan::Advance(scan);
		}
		hasFraction = true;
	}

	long exponent = 0;
	if (Scan::Peek(scan) == 'e' || Scan::Peek(scan) == 'E')
	{
		Scan::Advance(scan); // advance past e or E
		if (!(Scan::IsPartOfNumber(Scan::Peek(scan)) || Scan::Peek(scan) == '+'))
			Scan::HandleError(scan, "Expected digits for the exponent, none provided", scan.current);

		int exponentStart = scan.current;
		while (Scan::IsPartOfNumber(Peek(scan)) || Peek(scan) == '+')
		{
			Scan::Advance(scan);
		}
		exponent = strtol(scan.file.substr(exponentStart, (scan.current - exponentStart)).c_str(), nullptr, 10);
	}

	if (hasFraction)
		return strtod(scan.file.substr(start, (scan.current - start)).c_str(), nullptr);
	
	return strtol(scan.file.substr(start, (scan.current - start)).c_str(), nullptr, 10) * (long)pow(10, exponent);
}

bool ParseNull(Scan::ScanningState& scan)
{
	int start = scan.current;
	for (int i = 0; i < 4; i++)
		Scan::Advance(scan);

	if (scan.file.substr(start, 4) == "null")
	{
		return true;
	}

	Scan::HandleError(scan, "Expected 'null'", start);
	return false;
}

bool ParseTrue(Scan::ScanningState& scan)
{
	int start = scan.current;
	for (int i = 0; i < 4; i++)
		Scan::Advance(scan);

	if (scan.file.substr(start, 4) == "true")
	{
		return true;
	}

	Scan::HandleError(scan, "Expected 'true'", start);
	return false;
}

bool ParseFalse(Scan::ScanningState& scan)
{
	int start = scan.current;
	for (int i = 0; i < 5; i++)
		Scan::Advance(scan);

	if (scan.file.substr(start, 5) == "false")
	{
		return true;
	}

	Scan::HandleError(scan, "Expected 'false'", start);
	return false;
}

JsonValue ParseValue(Scan::ScanningState& scan)
{
	switch (Scan::Peek(scan))
	{
	case '{':
		return ParseObject(scan);
		break;
	case '[':
		return ParseArray(scan);
		break;
	case '"':
		return Scan::ParseToString(scan, '"');
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
		if (Scan::IsPartOfNumber(Peek(scan)))
			return ParseNumber(scan);
		else
			Scan::HandleError(scan, "Unknown value, please give a known value", scan.current);
		break;
	}
	return JsonValue();
}

eastl::vector<JsonValue> ParseArray(Scan::ScanningState& scan)
{
	eastl::vector<JsonValue> array;
	Advance(scan); // advance past opening '['

	while (!Scan::IsAtEnd(scan) && Scan::Peek(scan) != ']')
	{
		Scan::AdvanceOverWhitespace(scan);
		array.push_back(ParseValue(scan));	
		Scan::AdvanceOverWhitespace(scan);

		char nextC = Scan::Peek(scan);
		if (nextC == ',')
			Scan::Advance(scan);
		else if (nextC == ']')
			continue;
		else
		{
			Scan::HandleError(scan, "Expected a ',' for next array element, or ']' to end the array", scan.current);
			return array;
		}
	}
	Scan::AdvanceOverWhitespace(scan);
	Scan::Advance(scan); // Advance over closing ']'
	return array;
}

eastl::map<eastl::string, JsonValue> ParseObject(Scan::ScanningState& scan)
{
	eastl::map<eastl::string, JsonValue> map;
	Scan::Advance(scan); // advance past opening '{'

	while (!Scan::IsAtEnd(scan) && Scan::Peek(scan) != '}')
	{
		Scan::AdvanceOverWhitespace(scan);
		
		if (Scan::Peek(scan) != '"')
			Scan::HandleError(scan, "Expected '\"' to start a new key", scan.current);
		eastl::string key = Scan::ParseToString(scan, '"');

		Scan::AdvanceOverWhitespace(scan);
		if (Scan::Advance(scan) != ':')
		{
			Scan::HandleError(scan, "Expected a key value separator here, ':'", scan.current - 1);
			return map;
		}
		Scan::AdvanceOverWhitespace(scan);

		map[key] = ParseValue(scan);
		Scan::AdvanceOverWhitespace(scan);

		char nextC = Scan::Peek(scan);
		if (nextC == ',')
			Scan::Advance(scan);
		else if (nextC == '}')
			continue;
		else
		{
			Scan::HandleError(scan, "Expected a ',' for next object element, or '}' to end the object", scan.current);
			return map;
		}
	}
	Scan::AdvanceOverWhitespace(scan);
	Scan::Advance(scan); // Advance over closing '}'
	return map;
}

JsonValue ParseJsonFile(eastl::string& file)
{
    Scan::ScanningState scan;
	scan.file = file;
	scan.current = 0;
	scan.line = 1;
	Scan::AdvanceOverWhitespace(scan);
	JsonValue json = ParseValue(scan);
	if (scan.encounteredError)
		return JsonValue();
    return json;
}