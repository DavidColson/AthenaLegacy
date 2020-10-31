#include "TypeSystem.h"
#include "ErrorHandling.h"

// @ I'd like these custom types to be taken outside of the core of the type system
#include "Scene.h"
#include "AssetDatabase.h"
#include "Scanning.h"

#include <EASTL/string.h>

// ***********************************************************************

TypeData::~TypeData()
{
	delete pTypeOps;

	// TODO: For loop over members and delete them
}

// ***********************************************************************

Variant TypeData::New()
{
	return pTypeOps->New();
}

// ***********************************************************************

bool TypeData::operator==(const TypeData& other)
{
	return other.id == this->id;
}

// ***********************************************************************

bool TypeData::operator!=(const TypeData& other)
{
	return other.id != this->id;
}

// ***********************************************************************

bool TypeData::IsValid()
{
	return id != 0;
}

// ***********************************************************************

TypeData_Struct& TypeData::AsStruct()
{
	return *static_cast<TypeData_Struct*>(this);
}

// ***********************************************************************

TypeData_Enum& TypeData::AsEnum()
{
	return *static_cast<TypeData_Enum*>(this);
}




// ***********************************************************************

JsonValue TypeData_Struct::ToJson(Variant value)
{
    JsonValue result = JsonValue::NewObject();

	// TODO: Order of members is lost using this method. We have our member offsets, see if we can use it somehow
    for (Member& member : value.GetType().AsStruct())
    {
        result[member.name] = member.GetType().ToJson(member.Get(value));
    }

    return result;
}

// ***********************************************************************

Variant TypeData_Struct::FromJson(const JsonValue& json)
{
    Variant var = New();

    for (const eastl::pair<eastl::string, JsonValue>& val : *json.internalData.pObject)
	{
		if (MemberExists(val.first.c_str()))
		{
			Member& mem = GetMember(val.first.c_str());

			Variant parsed = mem.GetType().FromJson(val.second);
			mem.Set(var, parsed);   
		}
    }

    return var;
}

// ***********************************************************************

bool TypeData_Struct::MemberExists(const char* _name)
{
	return memberOffsets.count(_name) == 1;
}

// ***********************************************************************

Member& TypeData_Struct::GetMember(const char* _name)
{
	ASSERT(memberOffsets.count(_name) == 1, "The member you're trying to access doesn't exist");
	return *members[memberOffsets[_name]];
}

// ***********************************************************************

Member& TypeData_Struct::MemberIterator::operator*() const 
{ 
	return *it->second;
}

// ***********************************************************************

bool TypeData_Struct::MemberIterator::operator==(const MemberIterator& other) const 
{
	return it == other.it;
}

// ***********************************************************************

bool TypeData_Struct::MemberIterator::operator!=(const MemberIterator& other) const 
{
	return it != other.it;
}

// ***********************************************************************

TypeData_Struct::MemberIterator& TypeData_Struct::MemberIterator::operator++()
{
	++it;
	return *this;
}

// ***********************************************************************

const TypeData_Struct::MemberIterator TypeData_Struct::begin() 
{
	return MemberIterator(members.begin());
}

// ***********************************************************************

const TypeData_Struct::MemberIterator TypeData_Struct::end()
{
	return MemberIterator(members.end());
}




// ***********************************************************************

TypeData_Enum::TypeData_Enum(uint32_t _id, const char* _name, size_t _size, TypeDataOps* _pTypeOps, TypeData::CastableTo _castableTo, std::initializer_list<Enumerator> cats) : TypeData(_id, _name, _size, _pTypeOps, _castableTo), categories(cats)
{
	TypeDatabase::Data::Get().typeNames.emplace(name, static_cast<TypeData*>(this));
}

// ***********************************************************************

JsonValue TypeData_Enum::ToJson(Variant var)
{
	// This will perform a reinterpret cast to int, probably enforce enums to be ints somehow
	int value = var.GetValue<int>();
	return JsonValue(categories[value].identifier);
}

// ***********************************************************************

Variant TypeData_Enum::FromJson(const JsonValue& val)
{

	eastl::vector<Enumerator>::iterator it = eastl::find_if(categories.begin(), categories.end(), [&val](Enumerator& enumerator) 
	{
		return enumerator.identifier == val.ToString();
	});

	if (it == categories.end())
		Log::Crit("Attempting to create an enum from an invalid jsonValue: %s", val.ToString().c_str());

	Variant result = New();
	result.GetValue<int>() = (int)eastl::distance(categories.begin(), it);
	return result;
}



namespace TypeDatabase
{
	Data* TypeDatabase::Data::pInstance{ nullptr };

	bool TypeExists(const char* name)
	{
		if (Data::Get().typeNames.count(name) == 1)
			return true;
		else
			return false;
	}

	TypeData& GetFromString(const char* name)
	{
		return *Data::Get().typeNames[name];
	}
}

// Primitive Types
//////////////////

// @TODO: Set typeDataops!

struct TypeData_Int : TypeData
{
	TypeData_Int() : TypeData{"int", sizeof(int)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("int", this);
		id = Type::Index<int>();
	}

	virtual JsonValue ToJson(Variant var) override
	{
		return JsonValue((long)var.GetValue<int>());
	}

	virtual Variant FromJson(const JsonValue& val) override
	{
		return (int)val.ToInt();
	}
};
template <>
TypeData& getPrimitiveTypeData<int>()
{
	static TypeData_Int typeData;
	return typeData;
}





// ***********************************************************************

struct TypeData_Float : TypeData
{
	TypeData_Float() : TypeData{"float", sizeof(float)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("float", this);
		id = Type::Index<float>();
	}

	virtual JsonValue ToJson(Variant var) override
	{
		return JsonValue((double)var.GetValue<float>());
	}

	virtual Variant FromJson(const JsonValue& val) override
	{
		return (float)val.ToFloat();
	}
};
template <>
TypeData& getPrimitiveTypeData<float>()
{
	static TypeData_Float typeData;
	return typeData;
}





// ***********************************************************************

struct TypeData_Double : TypeData
{
	TypeData_Double() : TypeData{"double", sizeof(double)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("double", this);
		id = Type::Index<double>();
	}

	virtual JsonValue ToJson(Variant var) override
	{
		return JsonValue(var.GetValue<double>());
	}

	virtual Variant FromJson(const JsonValue& val) override
	{
		return val.ToFloat();
	}
};
template <>
TypeData& getPrimitiveTypeData<double>()
{
	static TypeData_Double typeData;
	return typeData;
}





// ***********************************************************************

struct TypeData_String : TypeData
{
	TypeData_String() : TypeData{"eastl::string", sizeof(eastl::string)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("eastl::string", this);
		id = Type::Index<eastl::string>();
	}

	virtual JsonValue ToJson(Variant var) override
	{
		return JsonValue(var.GetValue<eastl::string>());
	}

	virtual Variant FromJson(const JsonValue& val) override
	{
		return val.ToString();
	}
};
template <>
TypeData& getPrimitiveTypeData<eastl::string>()
{
	static TypeData_String typeData;
	return typeData;
}





// ***********************************************************************

struct TypeData_Bool : TypeData
{
	TypeData_Bool() : TypeData{"bool", sizeof(bool)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("bool", this);
		id = Type::Index<bool>();
	}

	virtual JsonValue ToJson(Variant var) override
	{
		return JsonValue(var.GetValue<bool>());
	}

	virtual Variant FromJson(const JsonValue& val) override
	{
		return val.ToBool();
	}
};
template <>
TypeData& getPrimitiveTypeData<bool>()
{
	static TypeData_Bool typeData;
	return typeData;
}





// ***********************************************************************

struct TypeData_EntityID : TypeData
{
	TypeData_EntityID() : TypeData{"EntityID", sizeof(EntityID)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("EntityID", this);
		id = Type::Index<EntityID>();
	}

	virtual JsonValue ToJson(Variant var) override
	{
		JsonValue val = JsonValue::NewObject();
		val["EntityID"] = JsonValue((long)var.GetValue<EntityID>().Index());
		return val;
	}

	virtual Variant FromJson(const JsonValue& val) override
	{
		return EntityID::New((int)val.Get("EntityID").ToInt(), 0);
	}
};
template <>
TypeData& getPrimitiveTypeData<EntityID>()
{
	static TypeData_EntityID typeData;
	return typeData;
}





// ***********************************************************************

struct TypeData_AssetHandle: TypeData
{
	TypeData_AssetHandle() : TypeData{"AssetHandle", sizeof(AssetHandle)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("AssetHandle", this);
		id = Type::Index<AssetHandle>();
	}

	virtual JsonValue ToJson(Variant var) override
	{
		JsonValue val = JsonValue::NewObject();
		AssetHandle handle = var.GetValue<AssetHandle>();
		val["Asset"] = AssetDB::GetAssetIdentifier(handle);
		return val;
	}

	virtual Variant FromJson(const JsonValue& val) override
	{
		return AssetHandle(val.Get("Asset").ToString());
	}
};
template <>
TypeData& getPrimitiveTypeData<AssetHandle>()
{
	static TypeData_AssetHandle typeData;
	return typeData;
}