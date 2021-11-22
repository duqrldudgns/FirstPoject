// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageNumbers.generated.h"

UCLASS()
class FIRSTPROJECT_API ADamageNumbers : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamageNumbers();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	class UWidgetComponent* DamageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FText DamageNumbers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool Headshot;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
