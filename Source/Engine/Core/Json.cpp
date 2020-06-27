#include "Json.h"

#include "Log.h"
#include "ErrorHandling.h"
#include "Scanning.h"



// Tokenizer
///////////////////////////

enum TokenType
{
	// Single characters
	LeftBracket,
	RightBracket,
	LeftBrace,
	RightBrace,
	Comma,
	Colon,

	// Identifiers and keywords
	Boolean,
	Null,
	Identifier,
	
	// Everything else
	Number,
	String
};

struct Token
{
	Token(TokenType _type, int _line, int _column, int _index)
	 : type(_type), line(_line), column(_column), index(_index) {}

	Token(TokenType _type, int _line, int _column, int _index, eastl::string _stringOrIdentifier)
	 : type(_type), line(_line), column(_column), index(_index), stringOrIdentifier(_stringOrIdentifier) {}

	Token(TokenType _type, int _line, int _column, int _index, double _dNumber)
	 : type(_type), line(_line), column(_column), index(_index), dNumber(_dNumber) {}

	Token(TokenType _type, int _line, int _column, int _index, bool _boolean)
	 : type(_type), line(_line), column(_column), index(_index), boolean(_boolean) {}

	TokenType type;
	eastl::string stringOrIdentifier;
	double dNumber;
	bool boolean;

	int line;
	int column;
	int index;
};

eastl::string ParseString(Scan::ScanningState& scan, char bound)
{	
	int start = scan.current;
	eastl::string result;
	while (Scan::Peek(scan) != bound && !Scan::IsAtEnd(scan))
	{
		char c = Advance(scan);
		
		// Disallowed characters
		switch (c)
		{
		case '\n':
			break;
		case '\r':
			if (Scan::Peek(scan) == '\n') // CRLF line endings
				Scan::Advance(scan);
			break;
			Scan::HandleError(scan, "Unexpected end of line", scan.current-1); break;
		default:
			break;
		}

		if (c == '\\')
		{
			char next = Advance(scan);
			switch (next)
			{
			// Convert basic escape sequences to their actual characters
			case '\'': result += '\''; break;
			case '"': result += '"'; break;
			case '\\':result += '\\'; break;
			case 'b': result += '\b'; break;
			case 'f': result += '\f'; break;
			case 'n': result += '\n'; break;
			case 'r': result += '\r'; break;
			case 't': result += '\t'; break;
			case 'v': result += '\v'; break;
			case '0':result += '\0'; break;

			// Unicode stuff, not doing this for now
			case 'u':
				Scan::HandleError(scan, "This parser does not yet support unicode escape codes", scan.current - 1); break;
			
			// Line terminators, allowed but we do not include them in the final string
			case '\n':
				break;
			case '\r':
				if (Scan::Peek(scan) == '\n') // CRLF line endings
					Scan::Advance(scan);
				break;
			default:
				result += next; // all other escaped characters are kept as is, without the '\' that preceeded it
			}
		}
		else
			result += c;
	}
	Scan::Advance(scan);
	return result;
}

// TODO: Rename to parseNumber when we remove the old version
double ParseNumb(Scan::ScanningState& scan)
{	
	scan.current -= 1; // Go back to get the first digit or symbol
	int start = scan.current;

	// Hex number
	if (Scan::Peek(scan) == '0' && (Scan::PeekNext(scan) == 'x' || Scan::PeekNext(scan) == 'X'))
	{
		Scan::Advance(scan);
		Scan::Advance(scan);
		while (Scan::IsHexDigit(Scan::Peek(scan)))
		{
			Scan::Advance(scan);
		}
	}
	// Normal number
	else
	{
		while(Scan::IsDigit(Scan::Peek(scan)) 
		|| Scan::Peek(scan) == '+'
		|| Scan::Peek(scan) == '-'
		|| Scan::Peek(scan) == '.'
		|| Scan::Peek(scan) == 'E'
		|| Scan::Peek(scan) == 'e')
		{
			Scan::Advance(scan);
		}
	}

	// TODO: error report. This returns 0.0 if no conversion possible. We can look at the literal string and see
	// If it's 0.0, ".0", "0." or 0. if not there's been an error in the parsing. I know this is cheeky. I don't care.
	return strtod(scan.file.substr(start, (scan.current - start)).c_str(), nullptr);
}

eastl::vector<Token> TokenizeJson(eastl::string jsonText)
{
	Scan::ScanningState scan;
	scan.file = jsonText;
	scan.current = 0;
	scan.line = 1;

	eastl::vector<Token> tokens;

	while (!Scan::IsAtEnd(scan))
	{
		char c = Scan::Advance(scan);
		int column = scan.current - scan.currentLineStart;
		int loc = scan.current - 1;
		switch (c)
		{
		// Single character tokens
		case '[': 
			tokens.push_back(Token{LeftBracket, scan.line, column, loc}); break;
		case ']': 
			tokens.push_back(Token{RightBracket, scan.line, column, loc}); break;
		case '{': 
			tokens.push_back(Token{LeftBrace, scan.line, column,loc}); break;
		case '}': 
			tokens.push_back(Token{RightBrace, scan.line, column, loc}); break;
		case ':': 
			tokens.push_back(Token{Colon, scan.line, column, loc}); break;
		case ',': 
			tokens.push_back(Token{Comma, scan.line, column, loc}); break;

		// Comments!
		case '/':
			if (Scan::Match(scan, '/'))
			{
				while (Scan::Peek(scan) != '\n')
					Scan::Advance(scan);
			}
			else if (Scan::Match(scan, '*'))
			{
				while (!(Scan::Peek(scan) == '*' && Scan::PeekNext(scan) == '/'))
					Scan::Advance(scan);

				Scan::Advance(scan); // *
				Scan::Advance(scan); // /
			}
			break;
			
		// Whitespace
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			break;

		// String literals
		case '\'':
		{
			eastl::string string = ParseString(scan, '\'');
			tokens.push_back(Token{String, scan.line, column, loc, string}); break;
			break;
		}
		case '"':
		{
			eastl::string string = ParseString(scan, '"');
			tokens.push_back(Token{String, scan.line, column, loc, string}); break;
			break;		
		}

		default:
			// Numbers
			if (Scan::IsDigit(c) || c == '+' || c == '-' || c == '.')
			{
				double num = ParseNumb(scan);
				tokens.push_back(Token{Number, scan.line, column, loc, num});
				break;
			}
			
			// Identifiers and keywords
			if (Scan::IsAlpha(c))
			{
				while (Scan::IsAlphaNumeric(Scan::Peek(scan)))
					Scan::Advance(scan);
				
				eastl::string identifier = scan.file.substr(loc, scan.current - loc);

				// Check for keywords
				if (identifier == "true")
					tokens.push_back(Token{Boolean, scan.line, column, loc, true});
				else if (identifier == "false")
					tokens.push_back(Token{Boolean, scan.line, column, loc, false});
				else if (identifier == "null")
					tokens.push_back(Token{Null, scan.line, column, loc});
				else
					tokens.push_back(Token{Identifier, scan.line, column, loc, identifier});
			}
			break;
		}

	}
	return tokens;
}



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
		// For JSON5 this bound could be '
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

		// For JSON5 this could be a non quoted identifier
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
	eastl::vector<Token> tokens = TokenizeJson(file);



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