// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Reference to the UMG asset in the editor 
	*	블루프린트에서 설정함
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")	
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	/** Variable to hold the widget after creating it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "widgets")
	UUserWidget* EnemyHealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "widgets")
	UUserWidget* PauseMenu;

	bool bEnemyHealthBarVisible;

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	FVector EnemyLocation;


	bool bPauseMenuVisible;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayPauseMenu();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void TogglePauseMenu();


protected:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

};
