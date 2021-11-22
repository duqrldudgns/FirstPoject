// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ArrowPickUp.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API AArrowPickUp : public AItem
{
	GENERATED_BODY()
	
public:

	AArrowPickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	int32 AmmoToAdd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	class USoundCue* PickUpSound;
};
