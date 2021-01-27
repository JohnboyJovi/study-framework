#pragma once

#include "CoreMinimal.h"
#include "Class.h"

// USTRUCT(BlueprintType)
// namespace EDistance
// {
// 
// }

enum EPriority
{
	Last	= 1,
	Low		= 2,
	Neutral = 3,
	High	= 4,
	First	= 5
};

struct FStudySetting
{
	FString Name				= "";
	int		Count				= 1;
	void*	Function			= nullptr;
	int		Priority			= EPriority::Neutral;		// TODO Make use of it
	bool	bBehindEachOther	= false;					// TODO Make use of it

	bool operator==(FStudySetting const & Other) const {
		return this->Name == Other.Name;
	}

	bool operator==(FString const & Other) const {
		return this->Name == Other;
	}
};


/*
template <typename T>
struct array {
	size_t x;
	T *ary;
};


int f(int a, int b, bool c)
{
	return c ? a + b : a - b;
}

template <typename R, typename... Args>
R callFunctionUsingMemAddress(void* funcaddr, Args... args)
{
	typedef R(*Function)(Args...);
	Function fnptr = (Function)funcaddr;
	return fnptr(args...);
}
int main()
{
	std::cout << callFunctionUsingMemAddress<int>((void*)f, 42, 10, true) << '\n';
	std::cout << callFunctionUsingMemAddress<int>((void*)f, 42, 10, false) << '\n';
}
*/