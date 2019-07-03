#ifndef FARB_REFLECTION_DECLARE_H
#define FARB_REFLECTION_DECLARE_H

#include <string>
#include <type_traits>
#include <vector>
#include <experimental/type_traits>

#include "../errors/ErrorOr.hpp"

namespace Farb
{

// rmf todo: @implement HString or another pooled string option
// ideally one that allows for a constexpr constructor
using HString = std::string;
using uint = unsigned int;
using byte = unsigned char;

namespace Reflection
{

struct TypeInfo;

struct ReflectionObject
{
	byte* location;
	TypeInfo* typeInfo;

	ReflectionObject()
		: location(nullptr)
		, typeInfo(nullptr)
	{ }

	ReflectionObject(byte* location, TypeInfo* typeInfo)
		: location(location)
		, typeInfo(typeInfo)
	{ }

	ReflectionObject(const ReflectionObject& other)
		: location(other.location)
		, typeInfo(other.typeInfo)
	{ }

	template<typename T>
	static ReflectionObject Construct(T& object);

	bool AssignBool(bool value) const;
	bool AssignUInt(uint value) const;
	bool AssignInt(int value) const;
	bool AssignFloat(float value) const;
	bool AssignString(std::string value) const;

	ErrorOr<ReflectionObject> GetAtKey(HString name) const;
	bool InsertKey(HString name) const;

	ErrorOr<ReflectionObject> GetAtIndex(int index) const;
	bool PushBackDefault() const;
};

struct TypeInfo
{	
	friend class ReflectionObject;
protected:
	HString name;

public:
	TypeInfo(HString name)
		: name(name)
	{
	}

	const HString& GetName() const { return name; }

	virtual bool AssignBool(byte* obj, bool value) const { return false; }
	virtual bool AssignUInt(byte* obj, uint value) const { return false; }
	virtual bool AssignInt(byte* obj, int value) const { return false; }
	virtual bool AssignFloat(byte* obj, float value) const { return false; }
	virtual bool AssignString(byte* obj, std::string value) const { return false; }

	virtual ErrorOr<ReflectionObject> GetAtKey(
		byte* obj,
		HString name) const
	{
		return Error(name + " GetAtKey not impelmented.");
	}

	virtual bool InsertKey(byte* obj, HString name) const { return false; }

	virtual ErrorOr<ReflectionObject> GetAtIndex(
		byte* obj,
		int index) const
	{
		return Error(name + " GetAtIndex not impelmented.");
	}

	virtual bool PushBackDefault(byte* obj) const { return false; }
};

/*
// helper for defining something when template arguments resolve correctly
// and therefore also being able to use sfinae when they don't


template<typename T, typename = void>
struct has_GetTypeInfo : std::false_type {};

// this resolution is preferred to the above because it specifies the second template type
// rather than relying on the default argument for the second template type
// and it's only possible if decltype(T::GetTypeInfo) is valid
// otherwise fails, but sfinae, so we fall back on the above
// partial specialization only works on structs, not functions, otherwise we could apply this
// directly in the function definition
template<typename T>
struct has_GetTypeInfo<T, void_t<decltype(T::GetTypeInfo)> > : std::true_type {};
*/

template<typename...> struct voider { using type = void; };
template<typename... Ts> using void_t = typename voider<Ts...>::type;

template <typename T>
struct TemplatedTypeInfo
{
	static TypeInfo* GetTypeInfo();
};

//template <class T>
//using has_typeInfo = decltype(T::typeInfo);

template <class T, typename = void>
struct has_typeInfo : std::false_type {};

template<typename T>
struct has_typeInfo<T, void_t<decltype(T::typeInfo)> > : std::true_type {};

template <typename T>
TypeInfo* GetTypeInfo()
{
	//if constexpr (std::experimental::is_detected<has_typeInfo, T>::value)
	if constexpr (has_typeInfo<T>::value)
	{
		return &T::typeInfo;
	}
	else
	{
		return TemplatedTypeInfo<T>::GetTypeInfo();
	}
}

template <class T, typename = void>
struct has_GetTypeInfo : std::false_type {};

template<typename T>
struct has_GetTypeInfo<T, void_t<decltype(std::declval<T>().GetTypeInfo())> > : std::true_type {};





template<typename T>
TypeInfo* GetTypeInfo(const T& obj)
{
	if constexpr (has_GetTypeInfo<T>::value)
	{
		return obj.GetTypeInfo();
	}
	else
	{
		return GetTypeInfo<T>();
	}
}

template<typename T>
ReflectionObject ReflectionObject::Construct(T& object)
{
	return ReflectionObject(
		reinterpret_cast<byte*>(&object),
		GetTypeInfo(object));
}

bool ReflectionObject::AssignBool(bool value) const
{
	return typeInfo->AssignBool(location, value);
}

bool ReflectionObject::AssignUInt(uint value) const
{
	return typeInfo->AssignUInt(location, value);
}

bool ReflectionObject::AssignInt(int value) const
{
	return typeInfo->AssignInt(location, value);
}

bool ReflectionObject::AssignFloat(float value) const
{
	return typeInfo->AssignFloat(location, value);
}

bool ReflectionObject::AssignString(std::string value) const
{
	return typeInfo->AssignString(location, value);
}

ErrorOr<ReflectionObject> ReflectionObject::GetAtKey(HString name) const
{
	return typeInfo->GetAtKey(location, name);
}

bool ReflectionObject::InsertKey(HString name) const
{
	return typeInfo->InsertKey(location, name);
}

ErrorOr<ReflectionObject> ReflectionObject::GetAtIndex(int index) const
{
	return typeInfo->GetAtIndex(location, index);
}

bool ReflectionObject::PushBackDefault() const
{
	return typeInfo->PushBackDefault(location);
}

struct ReflectionContext
{
	ReflectionContext* parentContext;
	TypeInfo* typeInfo;
	byte* object;
};

} // namespace Reflection

} // namespace Farb

#endif // FARB_REFLECTION_DECLARE_H
