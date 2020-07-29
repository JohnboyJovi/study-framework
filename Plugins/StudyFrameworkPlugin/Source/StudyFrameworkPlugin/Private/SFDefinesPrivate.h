#pragma once

#include "CoreMinimal.h"
#include "Class.h"

#include "SFDefinesPublic.h"

// USTRUCT(BlueprintType)
// namespace EDistance
// {
// 
// }


struct FStudySettingU
{
	FString Name				= "";
	int		Count				= 1;
	int		Priority			= EPriority::Neutral;		// TODO Make use of it
	bool	bBehindEachOther	= false;					// TODO Make use of it

	bool operator==(FStudySetting const & Other) const {
		return this->Name == Other.Name;
	}

	bool operator==(FString const & Other) const {
		return this->Name == Other;
	}

	FStudySettingU& operator=(const FStudySetting& a)
	{
		this->Name				= a.Name;
		this->Count				= a.Count;
		this->Priority			= a.Priority;
		this->bBehindEachOther	= a.bBehindEachOther;

		return *this;
	}

	FStudySettingU()
	{
	}
	
	FStudySettingU(const FStudySetting& a)
	{
		this->Name				= a.Name;
		this->Count				= a.Count;
		this->Priority			= a.Priority;
		this->bBehindEachOther	= a.bBehindEachOther;
	}
};
					 
					 