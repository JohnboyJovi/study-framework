#pragma once

#include "CoreMinimal.h"
#include "Class.h"

// USTRUCT(BlueprintType)
// namespace EDistance
// {
// 
// }

struct FStudySetting
{
	FString NameOfConfiguration;

	void* Function;

	int NumberOfConfigurations;

	bool operator==(FStudySetting const & Other) const {
		return this->NameOfConfiguration == Other.NameOfConfiguration;
	}

	bool operator==(FString const & Other) const {
		return this->NameOfConfiguration == Other;
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