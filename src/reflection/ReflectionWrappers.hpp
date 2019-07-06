#ifndef FARB_REFLECTION_WRAPPERS_H
#define FARB_REFLECTION_WRAPPERS_H

#include "ReflectionDeclare.h"
#include "ReflectionDefine.hpp"
#include "../core/NamedType.hpp"
#include "../core/ValueCheckedType.hpp"

namespace Farb
{

namespace Reflection
{

template<typename T, typename Tag>
struct TemplatedTypeInfo<NamedType<T, Tag>>
{
	template<typename TVal>
	static bool Assign(NamedType<T, Tag>& obj, TVal value)
	{
		auto reflect = Reflect(obj.value);
		auto pTypeInfo = dynamic_cast<TypeInfoCustomLeaf<T>*>(reflect.typeInfo);
		if (pTypeInfo == nullptr)
		{
			// rmf todo: @implement ErrorOr for all of the api
			return false;
		}
		return pTypeInfo->Assign(reflect.location, value);
	}

	static TypeInfo* Get()
	{
		static auto namedTypeInfo = TypeInfoCustomLeaf<NamedType<T, Tag> >::Construct(
			Tag::GetName(),
			Assign<bool>,
			Assign<uint>,
			Assign<int>,
			Assign<float>,
			Assign<std::string>
		);

		return &namedTypeInfo;
	}
};

template<typename T, typename Tag>
struct TemplatedTypeInfo<ValueCheckedType<T, Tag>>
{
	template<typename TVal>
	static bool Assign(ValueCheckedType<T, Tag>& obj, TVal value)
	{
		auto pTypeInfo = GetTypeInfo<T>();
		auto pTypeInfoLeaf = dynamic_cast<TypeInfoCustomLeaf<T>*>(pTypeInfo);
		if (pTypeInfoLeaf == nullptr)
		{
			// rmf todo: @implement ErrorOr for all of the api
			return false;
		}
		T unchecked{};
		auto uncheckedReflect = Reflect(unchecked);
		bool success = pTypeInfoLeaf->Assign(uncheckedReflect.location, value);
		if (!success)
		{
			return false;
		}
		if (!Tag::IsValid(unchecked))
		{
			return false;
		}
		obj = ValueCheckedType<T, Tag>(unchecked);
		return true;
	}

	static TypeInfo* Get()
	{
		static auto namedTypeInfo = TypeInfoCustomLeaf<ValueCheckedType<T, Tag> >::Construct(
			Tag::GetName(),
			Assign<bool>,
			Assign<uint>,
			Assign<int>,
			Assign<float>,
			Assign<std::string>
		);

		return &namedTypeInfo;
	}
};



} // namespace Reflection

} // namespace Farb


#endif // FARB_REFLECTION_WRAPPERS_H