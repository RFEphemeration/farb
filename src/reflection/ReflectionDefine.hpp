#ifndef FARB_REFLECTION_DEFINE_H
#define FARB_REFLECTION_DEFINE_H

#include <memory>
#include <iostream>
#include <limits.h>

#include "ReflectionDeclare.h"


namespace Farb
{

namespace Reflection
{

template<typename T>
struct TypeInfoCustomLeaf : public TypeInfo
{
protected:
	bool (T::*pAssignBool)(bool);
	bool (T::*pAssignUInt)(uint);
	bool (T::*pAssignInt)(int);
	bool (T::*pAssignFloat)(float);
	bool (T::*pAssignString)(std::string);

public:
	TypeInfoCustomLeaf()
		: pAssignBool(nullptr)
		, pAssignUInt(nullptr)
		, pAssignInt(nullptr)
		, pAssignFloat(nullptr)
		, pAssignString(nullptr)
	{
	}

	virtual bool AssignBool(byte* obj, bool value) const override
	{
		if (pAssignBool == nullptr) { return false; }
		T* t = reinterpret_cast<T*>(obj);
		return (t->*pAssignBool)(value);
	}

	virtual bool AssignUInt(byte* obj, uint value) const override
	{
		if (pAssignUInt == nullptr) { return false; }
		T* t = reinterpret_cast<T*>(obj);
		return (t->*pAssignUInt)(value);
	}

	virtual bool AssignInt(byte* obj, int value) const override
	{
		if (pAssignInt == nullptr) { return false; }
		T* t = reinterpret_cast<T*>(obj);
		return (t->*pAssignInt)(value);
	}

	virtual bool AssignFloat(byte* obj, float value) const override
	{
		if (pAssignFloat == nullptr) { return false; }
		T* t = reinterpret_cast<T*>(obj);
		return (t->*pAssignFloat)(value);
	}

	virtual bool AssignString(byte* obj, std::string value) const override
	{
		if (pAssignString == nullptr) { return false; }
		T* t = reinterpret_cast<T*>(obj);
		return (t->*pAssignString)(value);
	}
};

template<typename T>
struct TypeInfoEnum : public TypeInfo
{
protected:
	std::vector<std::pair<HString, int> > vValues;

public:

	TypeInfoEnum(HString name, std::vector<std::pair<HString, int> > vEnumValues)
		: TypeInfo(name)
		, vValues(vEnumValues)
	{

	}

	virtual bool AssignInt(byte* obj, int value) const override
	{
		T* t = reinterpret_cast<T*>(obj);
		for (const auto & pair : vValues)
		{
			if (pair.second == value)
			{
				*t = static_cast<T>(pair.second);
				return true;
			}
		}
		return false;
	}

	virtual bool AssignUInt(byte* obj, uint value) const override
	{
		if (value > INT_MAX)
		{
			return false;
		}
		return AssignInt(obj, static_cast<int>(value));
	}

	virtual bool AssignString(byte* obj, std::string value) const override
	{
		HString sValue = value;
		T* t = reinterpret_cast<T*>(obj);
		for (const auto & pair : vValues)
		{
			if (pair.first == sValue)
			{
				*t = static_cast<T>(pair.second);
				return true;
			}
		}
		return false;
	}
};

template<typename T, typename TVal>
struct TypeInfoArray : public TypeInfo
{
	virtual ErrorOr<ReflectionObject> GetAtIndex(
		byte* obj,
		int index) const override
	{
		T* t = reinterpret_cast<T*>(obj);
		if (t->size() <= index)
		{
			return Error(
				name
				+ " array GetAtIndex "
				+ std::to_string(index)
				+ " is out of bounds, "
				+ std::to_string(t->size())
				+ ".");
		}
		return ReflectionObject::Construct((*t)[index]);
	}

	virtual bool PushBackDefault(byte* obj) const override
	{
		T* t = reinterpret_cast<T*>(obj);
		t->push_back(TVal());
		return true;
	}
};

template<typename T, typename TKey, typename TVal>
struct TypeInfoTable : public TypeInfo
{
	// rmf todo: @implement
};

template<typename T>
struct TypeInfoStruct;

template<typename T>
struct MemberInfo
{
	friend class TypeInfoStruct<T>;
protected:
	HString name;
	TypeInfo* typeInfo;

	MemberInfo(HString name, TypeInfo* typeInfo)
		: name(name)
		, typeInfo(typeInfo)
	{}

public:
	virtual ~MemberInfo() { }

public:
	virtual byte* GetLocation(T* t) const = 0;
};

template<typename T, typename TMem>
struct MemberInfoTyped : public MemberInfo<T>
{
protected:
	TMem T::* location;

public:
	MemberInfoTyped(HString name, TMem T::* location)
		: MemberInfo<T>(name, GetTypeInfo<TMem>())
		, location(location)
	{}

	virtual byte* GetLocation(T* t) const override
	{
		TMem* pMem = &(t->*location);
		return reinterpret_cast<byte*>(pMem);
	}
};

template<typename T>
struct TypeInfoStruct : public TypeInfo
{

	// rmf note: at first all TypeInfo would have the opportunity for a parent type
	// but I figured it was mostly unecessary for the others. Feel free to change in the future.
	TypeInfo* parentType;
	std::vector<MemberInfo<T>*> vMembers;

	TypeInfoStruct(
		HString name,
		TypeInfo* parentType,
		std::vector<MemberInfo<T>*> members)
		: TypeInfo(name)
		, parentType(parentType)
		, vMembers(members)
	{}

	~TypeInfoStruct()
	{
		for (auto pMember : vMembers)
		{
			delete pMember;
		}
	}

	virtual ErrorOr<ReflectionObject> GetAtKey(
		byte* obj,
		HString name) const override
	{
		T* t = reinterpret_cast<T*>(obj);
		for (auto member : vMembers)
		{
			if (member->name == name)
			{
				return ReflectionObject(member->GetLocation(t), member->typeInfo);
			}
		}

		if (parentType != nullptr)
		{
			return parentType->GetAtKey(obj, name);
		}
		return Error(
			this->name
			+ " struct GetAtKey "
			+ name
			+ " failed. No member exists with that name.");
	}
};


} // namespace Reflection

} // namespace Farb


#endif // FARB_REFLECTION_DEFINE_H
