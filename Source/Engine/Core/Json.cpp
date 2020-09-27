#include "Json.h"

#include "Log.h"
#include "ErrorHandling.h"
#include "Scanning.h"
#include "TypeSystem.h"


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

	Token(TokenType _type, int _line, int _column, int _index, double _number)
	 : type(_type), line(_line), column(_column), index(_index), number(_number) {}

	Token(TokenType _type, int _line, int _column, int _index, bool _boolean)
	 : type(_type), line(_line), column(_column), index(_index), boolean(_boolean) {}

	TokenType type;
	eastl::string stringOrIdentifier;
	double number;
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

double ParseNumber(Scan::ScanningState& scan)
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
				double num = ParseNumber(scan);
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

eastl::map<eastl::string, JsonValue> ParseObject(eastl::vector<Token>& tokens, int& currentToken);
eastl::vector<JsonValue> ParseArray(eastl::vector<Token>& tokens, int& currentToken);

JsonValue ParseValue(eastl::vector<Token>& tokens, int& currentToken)
{
	Token& token = tokens[currentToken];

	switch (token.type)
	{
	case LeftBrace:
		return JsonValue(ParseObject(tokens, currentToken)); break;
	case LeftBracket:
		return JsonValue(ParseArray(tokens, currentToken)); break;
	case String:
		currentToken++;
		return JsonValue(token.stringOrIdentifier); break;
	case Number:
	{
		currentToken++;
		double n = token.number;
		double intPart;
		if (modf(n, &intPart) == 0.0)
			return JsonValue((long)intPart);
		else
			return JsonValue(n);
		break;
	}
	case Boolean:
		currentToken++;
		return JsonValue(token.boolean); break;
	case Null:
		currentToken++;
	default:
		return JsonValue(); break;
	}
	return JsonValue();
}

eastl::map<eastl::string, JsonValue> ParseObject(eastl::vector<Token>& tokens, int& currentToken)
{
	currentToken++; // Advance over opening brace

	eastl::map<eastl::string, JsonValue> map;
	while (currentToken < tokens.size() && tokens[currentToken].type != RightBrace)
	{
		// We expect, 
		// identifier or string
		if (tokens[currentToken].type != Identifier && tokens[currentToken].type != String)
			Log::Crit("Expected identifier or string");

		eastl::string key = tokens[currentToken].stringOrIdentifier;
		currentToken += 1;

		// colon
		if (tokens[currentToken].type != Colon)
			Log::Crit("Expected colon");
		currentToken += 1;
		
		// String, Number, Boolean, Null
		// If left bracket or brace encountered, skip until closing
		map[key] = ParseValue(tokens, currentToken);

		// Comma, or right brace
		if (tokens[currentToken].type == RightBrace)
			break;
		if (tokens[currentToken].type != Comma)
			Log::Crit("Expected comma or Right Curly Brace");
		currentToken += 1;
	}
	currentToken++; // Advance over closing brace
	return map;
}

eastl::vector<JsonValue> ParseArray(eastl::vector<Token>& tokens, int& currentToken)
{
	currentToken++; // Advance over opening bracket

	eastl::vector<JsonValue> array;
	while (currentToken < tokens.size() && tokens[currentToken].type != RightBracket)
	{
		// We expect, 
		// String, Number, Boolean, Null
		array.push_back(ParseValue(tokens, currentToken));

		// Comma, or right brace
		if (tokens[currentToken].type == RightBracket)
			break;
		if (tokens[currentToken].type != Comma)
			Log::Crit("Expected comma or right bracket");
		currentToken += 1;
	}
	currentToken++; // Advance over closing bracket
	return array;
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
	type = Type::Null;
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
		internalData.pArray = new eastl::vector<JsonValue>(*copy.internalData.pArray);
		break;
	case Type::Object:
		internalData.pObject = new eastl::map<eastl::string, JsonValue>(*copy.internalData.pObject);
		break;
	case Type::String:
   		internalData.pString = new eastl::string(*(copy.internalData.pString));
		break;
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

JsonValue ParseJsonFile(eastl::string& file)
{
	eastl::vector<Token> tokens = TokenizeJson(file);

	int firstToken = 0;
	JsonValue json5 = ParseValue(tokens, firstToken);
	return json5;
}

eastl::string SerializeJsonValue(JsonValue json, eastl::string indentation)
{
	eastl::string result = "";
	switch (json.type)
	{
	case JsonValue::Type::Array:
		result.append("[");
		if (json.Count() > 0)
			result.append("\n");

		for (const JsonValue& val : *json.internalData.pArray)
		{
			result.append_sprintf("    %s%s,\n", indentation.c_str(), SerializeJsonValue(val, indentation + "    ").c_str());
		}

		if (json.Count() > 0)
			result.append_sprintf("%s", indentation.c_str());
		result.append("]");
		break;
	case JsonValue::Type::Object:
	{
		result.append("{");
		if (json.Count() > 0)
			result.append("\n");

		for (const eastl::pair<eastl::string, JsonValue>& val : *json.internalData.pObject)
		{
			result.append_sprintf("    %s%s: %s,\n", indentation.c_str(), val.first.c_str(), SerializeJsonValue(val.second, indentation + "    ").c_str());
		}

		if (json.Count() > 0)
			result.append_sprintf("%s", indentation.c_str());
		result.append("}");
	}
	break;
	case JsonValue::Type::Floating:
		result.append(TypeDatabase::Get<double>().Serialize(json.ToFloat()));
		break;
		// TODO: Serialize with exponentials like we do with floats
	case JsonValue::Type::Integer:
		result.append(TypeDatabase::Get<int>().Serialize((int)json.ToInt()));
		break;
	case JsonValue::Type::Boolean:
		result.append(TypeDatabase::Get<bool>().Serialize(json.ToBool()));
		break;
	case JsonValue::Type::String:
		result.append(TypeDatabase::Get<eastl::string>().Serialize(json.ToString()));
		break;
	case JsonValue::Type::Null:
		result.append("null");
		break;
	default:
		result.append("CANT SERIALIZE YET");
		break;
	}
	return result;
}