#include "TypeSystem.h"
#include "ErrorHandling.h"

// @ I'd like these custom types to be taken outside of the core of the type system
#include "Scene.h"
#include "AssetDatabase.h"

#include "Scanning.h"

#include <EASTL/string.h>

Variant Member::Get(Variant& instance)
{
	Variant var;
	var.pData = new char[pType->size];
	char* instancePtr = reinterpret_cast<char*>(instance.pData);
	memcpy(var.pData, instancePtr + offset, pType->size);
	var.pTypeData = pType;
	return var;
}

void Member::Set(void* instance, Variant newValue)
{
	char* location = reinterpret_cast<char*>(instance);
	memcpy(location + offset, newValue.pData, newValue.pTypeData->size);
}

void Member::Set(Variant& instance, Variant newValue)
{
	char* location = reinterpret_cast<char*>(instance.pData);
	memcpy(location + offset, newValue.pData, newValue.pTypeData->size);
}



TypeData::~TypeData()
{
	delete pConstructor;
}

Variant TypeData::New()
{
	return pConstructor->Invoke();
}

bool TypeData::MemberExists(const char* _name)
{
	return memberOffsets.count(_name) == 1;
}

Member& TypeData::GetMember(const char* _name)
{
	ASSERT(memberOffsets.count(_name) == 1, "The member you're trying to access doesn't exist");
	return members[memberOffsets[_name]];
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

// @TODO: Set constructors!
// @Improvement: the parse functions should return optionals telling you whether they failed so we can properly handle parsing within a whole document

struct TypeData_Int : TypeData
{
	TypeData_Int() : TypeData{"int", sizeof(int)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("int", this);
	}

	virtual eastl::string Serialize(Variant var) override
	{
		eastl::string out;
		out.sprintf("%i", var.GetValue<int>());
		return out;
	}

	virtual Variant Parse(eastl::string_view str) override
	{
		return (int)strtol(eastl::string(str).c_str(), nullptr, 10);
	}
};
template <>
TypeData& getPrimitiveTypeData<int>()
{
	static TypeData_Int typeData;
	return typeData;
}





struct TypeData_Float : TypeData
{
	TypeData_Float() : TypeData{"float", sizeof(float)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("float", this);
	}

	virtual eastl::string Serialize(Variant var) override
	{
		eastl::string out;
		out.sprintf("%.9g", var.GetValue<float>());
		return out;
	}

	virtual Variant Parse(eastl::string_view str) override
	{
		return strtof(eastl::string(str).c_str(), nullptr);
	}
};
template <>
TypeData& getPrimitiveTypeData<float>()
{
	static TypeData_Float typeData;
	return typeData;
}





struct TypeData_Double : TypeData
{
	TypeData_Double() : TypeData{"double", sizeof(double)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("double", this);
	}

	virtual eastl::string Serialize(Variant var) override
	{
		eastl::string out;
		out.sprintf("%.17g", var.GetValue<double>());
		return out;
	}

	virtual Variant Parse(eastl::string_view str) override
	{
		return strtod(eastl::string(str).c_str(), nullptr);
	}
};
template <>
TypeData& getPrimitiveTypeData<double>()
{
	static TypeData_Double typeData;
	return typeData;
}





struct TypeData_String : TypeData
{
	TypeData_String() : TypeData{"eastl::string", sizeof(eastl::string)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("eastl::string", this);
	}

	virtual eastl::string Serialize(Variant var) override
	{
		eastl::string out;
		out.sprintf("\"%s\"", var.GetValue<eastl::string>().c_str());
		return out;
	}

	virtual Variant Parse(eastl::string_view str) override
	{
		Scan::ScanningState scan;
		scan.file = str;
		scan.current = 0;
		scan.line = 1;

		return Scan::ParseToString(scan, '"');
	}
};
template <>
TypeData& getPrimitiveTypeData<eastl::string>()
{
	static TypeData_String typeData;
	return typeData;
}





struct TypeData_Bool : TypeData
{
	TypeData_Bool() : TypeData{"bool", sizeof(bool)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("bool", this);
	}

	virtual eastl::string Serialize(Variant var) override
	{
		eastl::string out;
		bool val = var.GetValue<bool>();
        out.sprintf("%s", val ? "true" : "false");
		return out;
	}

	virtual Variant Parse(eastl::string_view str) override
	{
		if (str == "true")
			return true;
		if (str == "false")
			return false;
		
		return false;
	}
};
template <>
TypeData& getPrimitiveTypeData<bool>()
{
	static TypeData_Bool typeData;
	return typeData;
}





struct TypeData_EntityID : TypeData
{
	TypeData_EntityID() : TypeData{"EntityID", sizeof(EntityID)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("EntityID", this);
	}

	virtual eastl::string Serialize(Variant var) override
	{
		eastl::string out = "EntityID()";
		return out;
	}

	virtual Variant Parse(eastl::string_view str) override
	{
		return EntityID();
	}
};
template <>
TypeData& getPrimitiveTypeData<EntityID>()
{
	static TypeData_EntityID typeData;
	return typeData;
}





struct TypeData_AssetHandle: TypeData
{
	TypeData_AssetHandle() : TypeData{"AssetHandle", sizeof(AssetHandle)} 
	{
		TypeDatabase::Data::Get().typeNames.emplace("AssetHandle", this);
	}

	virtual eastl::string Serialize(Variant var) override
	{
		eastl::string out;
		AssetHandle handle = var.GetValue<AssetHandle>();
        out.sprintf("AssetHandle(\"%s\")", AssetDB::GetAssetIdentifier(handle).c_str());
		return out;
	}

	virtual Variant Parse(eastl::string_view str) override
	{
		Scan::ScanningState scan;
		scan.file = str;
		scan.current = 0;
		scan.line = 1;

		if (Scan::Peek(scan) == 'A')
		{
			for (int i = 0; i < 12; i++)
				Scan::Advance(scan);
			
			if (scan.file.substr(0, 12) != "AssetHandle(")
			{
				Scan::HandleError(scan, "Expected \"AssetHandle(\" here", 0);
				return AssetHandle();
			}
			eastl::string identifier = Scan::ParseToString(scan, '"');
			if (Scan::Advance(scan) != ')')
			{
				Scan::HandleError(scan, "Expected end of asset handle \")\"", scan.current);
				return AssetHandle();
			}

			return AssetHandle(identifier);
		}
		else
		{
			return AssetHandle();
		}
	}
};
template <>
TypeData& getPrimitiveTypeData<AssetHandle>()
{
	static TypeData_AssetHandle typeData;
	return typeData;
}