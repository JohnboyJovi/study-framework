// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Help/SFUtils.h"
#include "Logging/SFLoggingUtils.h"

#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"

#include "AudioDevice.h"

#include "Json.h"

#include "SFPlugin.h"
#include "SFGameInstance.h"


#include "IUniversalLogging.h"

void FSFUtils::OpenMessageBox(const FString Text, const bool bError/*=false*/)
{
	if (!IsPrimary())
	{
		return;
	}

	FSFLoggingUtils::Log(FString("[FSFUtils::OpenMessageBox(ERROR = ") + (bError ? "TRUE" : "FALSE") +
	    ")]: Opening Message Box with message: " + Text, bError);

	FText Title = FText::FromString(FString(bError ? "ERROR" : "Message"));
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Text), &Title);
}

int FSFUtils::OpenCustomDialog(const FString& Title, const FString& Content, const TArray<FString>& Buttons)
{

	TArray<SCustomDialog::FButton> Buttons_Text;
	for (const FString& Btn : Buttons) {
		Buttons_Text.Add(SCustomDialog::FButton(FText::FromString(Btn)));
	}

	TSharedRef<SCustomDialog> Dialog = SNew(SCustomDialog)
		.Title(FText(FText::FromString(Title)))
		.DialogContent(SNew(STextBlock).Text(FText::FromString(Content)))
		.Buttons(Buttons_Text);

	return Dialog->ShowModal();

}

int FSFUtils::OpenCustomDialogText(const FString& Title, const FString& Content, const FString& DefaultText, FString& OutText)
{
	auto VBox = SNew(SVerticalBox);
	auto Border = SNew(SBorder);
	auto ContentWidget = SNew(STextBlock).Text(FText::FromString(Content));
	auto InputWidget = SNew(SEditableText).Text(FText::FromString(DefaultText));
	InputWidget->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Border->SetContent(InputWidget);
	VBox->AddSlot().AttachWidget(ContentWidget);
	VBox->AddSlot().AttachWidget(Border);

	TSharedRef<SCustomDialog> Dialog = SNew(SCustomDialog)
		.Title(FText(FText::FromString(Title)))
		.DialogContent(VBox)
		.Buttons({
			SCustomDialog::FButton(FText::FromString("Submit"))
		});

	int Result = Dialog->ShowModal();
	
	if (Result < 0) {
		return -1;
	}

	OutText = InputWidget->GetText().ToString();
	return 0;

}

FString FSFUtils::JsonToString(TSharedPtr<FJsonObject> Json)
{
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	return OutputString;
}

TSharedPtr<FJsonObject> FSFUtils::StringToJson(FString String)
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<TCHAR>> Reader = FJsonStringReader::Create(String);
	FJsonSerializer::Deserialize(Reader, Json);

	return Json;
}

void FSFUtils::WriteJsonToFile(TSharedPtr<FJsonObject> Json, FString FilePath)
{
	FilePath = GetStudyFrameworkPath() + FilePath;
	FFileHelper::SaveStringToFile(JsonToString(Json), *(FilePath));
}

TSharedPtr<FJsonObject> FSFUtils::ReadJsonFromFile(FString FilePath)
{
	FString JsonString;
	FilePath = GetStudyFrameworkPath() + FilePath;
	if(!FFileHelper::LoadFileToString(JsonString, *(FilePath)))
	{
		return nullptr;
	}
	return StringToJson(JsonString);
}

UWorld* FSFUtils::GetWorld()
{
	return GEngine->GetWorld();
}

FString FSFUtils::GetStudyFrameworkPath()
{
	return FPaths::ProjectDir() + "StudyFramework/";
}

bool FSFUtils::IsPrimary()
{
	if (!IDisplayCluster::IsAvailable())
	{
		return true;
	}
	IDisplayClusterClusterManager* Manager = IDisplayCluster::Get().GetClusterMgr();
	if (Manager == nullptr)
	{
		return true; // if we are not in cluster mode, we are always the master
	}
	return Manager->IsMaster() || !Manager->IsSlave();
}

bool FSFUtils::IsHMD()
{
	// In editor builds: checks for EdEngine->IsVRPreviewActive()
	// In packaged builds: checks for `-vr` in commandline or bStartInVR in UGeneralProjectSettings
	return FAudioDevice::CanUseVRAudioDevice();
}
