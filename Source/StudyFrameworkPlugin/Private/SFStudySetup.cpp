#include "SFStudySetup.h"

#include "SFUtils.h"


USFStudySetup::USFStudySetup()
{
}

USFStudyPhase* USFStudySetup::AddStudyPhase(FString PhaseName)
{
	USFStudyPhase* Phase = NewObject<USFStudyPhase>(GetTransientPackage(), PhaseName);
	Phases.Add(Phase);
	return Phase;
}

bool USFStudySetup::CheckPhases()
{
	for (auto EntryPhase : Phases)
    {
        if (!EntryPhase->PhaseValid())
        {
            return false;
        }
    }

    return true;
}

