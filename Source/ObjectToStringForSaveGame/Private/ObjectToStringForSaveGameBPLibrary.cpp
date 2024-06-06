// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectToStringForSaveGameBPLibrary.h"
#include "ObjectToStringForSaveGame.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "UObject/Package.h"

UObjectToStringForSaveGameBPLibrary::UObjectToStringForSaveGameBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UObjectToStringForSaveGameBPLibrary::ObjectToString(UObject* Object, FString& OutJsonString, bool& Success)
{
    if (!Object)
    {
        Success = false;
        return;
    }

    TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    // Add the class type to the JSON object
    JsonObject->SetStringField(TEXT("Class"), Object->GetClass()->GetPathName());

    // Convert the UObject to a JsonObject
    if (FJsonObjectConverter::UStructToJsonObject(Object->GetClass(), Object, JsonObject, 0, 0))
    {
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonString);
        if (FJsonSerializer::Serialize(JsonObject, Writer))
        {
            Success = true;
            return;
        }
    }
    Success = false;
    return;
}

void UObjectToStringForSaveGameBPLibrary::StringToObject(const FString& JsonString, UObject*& OutObject, bool& Success)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    // Parse the JSON string into a JsonObject
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        // Get the class type from the JSON object
        FString ClassPath;
        if (JsonObject->TryGetStringField(TEXT("Class"), ClassPath))
        {
            // Find the class
            UClass* ObjectClass = FindObject<UClass>(ANY_PACKAGE, *ClassPath);
            if (ObjectClass)
            {
                // Create an instance of the class
                OutObject = NewObject<UObject>(GetTransientPackage(), ObjectClass);

                // Convert the JsonObject to a UObject
                if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), ObjectClass, OutObject, 0, 0))
                {
                    Success = true;
                    return;
                }
            }
        }
    }

    OutObject = nullptr;
    Success = false;
    return;
}

