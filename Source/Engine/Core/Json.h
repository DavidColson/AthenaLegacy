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
		Number,
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

	// @Incomplete Destructors for all types
    // @Incomplete access API
    
	JsonValue();
	JsonValue(eastl::vector<JsonValue>& array);
	JsonValue(eastl::map<eastl::string, JsonValue> object);
	JsonValue(eastl::string string);
	JsonValue(double number);
	JsonValue(bool boolean);


};

JsonValue ParseJsonFile(eastl::string& file);