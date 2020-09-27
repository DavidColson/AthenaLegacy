#pragma once

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/string.h>

struct JsonValue
{
	enum Type
	{
		Object,
		Array,
		Floating,
		Integer,
		Boolean,
		String,
		Null
	};

	Type type;
	union Data
	{
		eastl::vector<JsonValue>* pArray;
		eastl::map<eastl::string, JsonValue>* pObject;
		eastl::string* pString;
		double floatingNumber;
		long integerNumber;
		bool boolean;
	} internalData;

	JsonValue();

	JsonValue(const JsonValue& copy);
	JsonValue(JsonValue&& copy);
	JsonValue& operator=(const JsonValue& copy);
	JsonValue& operator=(JsonValue&& copy);

	JsonValue(eastl::vector<JsonValue>& array);
	JsonValue(eastl::map<eastl::string, JsonValue>& object);
	JsonValue(eastl::string string);
	JsonValue(const char* string);
	JsonValue(double number);
	JsonValue(long number);
	JsonValue(bool boolean);

	static JsonValue NewObject();
	static JsonValue NewArray();

	bool IsNull();
	bool HasKey(eastl::string identifier);
	int Count();

	eastl::string ToString();
	double ToFloat();
	long ToInt();
	bool ToBool();

	JsonValue& operator[](eastl::string identifier);
	JsonValue& operator[](size_t index);

	template <typename T>
	void Append(T newVal)
	{
		ASSERT(type == Type::Array, "Attempting to treat this value as an array when it is not.");
		internalData.pArray->push_back(newVal);
	}

	~JsonValue();
};

JsonValue ParseJsonFile(eastl::string& file);
eastl::string SerializeJsonValue(JsonValue json, eastl::string indentation = "");