#include "Logging/SFLogObject.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "IDisplayCluster.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine.h"

USFLogObject::USFLogObject() {
	LoggingInfo.Init(FActorLoggingInformation(0, nullptr, ""), 0);
}

void USFLogObject::AddActor(AActor* Actor, int32 LogTimer) {
	LoggingInfo.Add(FActorLoggingInformation(LogTimer, Actor, Actor->GetFName().ToString()));
}

void USFLogObject::AddActorWithName(AActor* Actor, int32 LogTimer, FString LogName) {
	LoggingInfo.Add(FActorLoggingInformation(LogTimer, Actor, LogName));
}

void USFLogObject::AddActors(TArray<AActor*> ActorArray, int32 LogTimer) {
	for (int32 Index = 0; Index != ActorArray.Num(); ++Index) {
		AddActor(ActorArray[Index], LogTimer);
	}
}

void USFLogObject::Initialize() {
	// so nicht mehr -> "ismaster" o.ä. bzw. in universal logging schon implementiert (?)
	//if (FModuleManager::Get().IsModuleLoaded("nDisplay")
	//	&& IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster) { // Cave
	//	LogThis = IDisplayCluster::Get().GetClusterMgr()->GetNodeId().Equals(TEXT("node_main"));
	//}
	//else { // Desktop oder HMD
	//	LogThis = true;
	//}
	StaticDateTime = FDateTime::Now();
	ProbandID = 0;
	LoggingInfo.SetNum(0, true);
	//SetUpLogDir();
	UE_LOG(LogTemp, Warning , TEXT("Initialised LogObject"));
}

void USFLogObject::SetUpLogDir() {
	if (ProbandID == 0) {
		LogDir = FPaths::ProjectDir() + "/StudyLogs/" + FDateTime::Now().ToString();
	}
	else {
		LogDir = FPaths::ProjectDir() + "/StudyLogs/" + FString::Printf(TEXT("%i"), ProbandID) + "_" + FDateTime::Now().ToString();
	}
}