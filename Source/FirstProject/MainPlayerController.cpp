// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
	if (WEnemyHealthBar)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment);	// 평평하고 화면을 향하도록 설정
	}
	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMainPlayerController::Tick(float DeltaTime)
{
	// 이 설정을 하지 않고 보여주게 될 경우 뷰포트화면 꽉차게 EnemyHealthBar가 보임
	if (EnemyHealthBar)
	{
		FVector2D PositionInViewport;	// 체력바는 2D
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);	//World Space 3D 위치를 2D Screen Space 위치로 변환
		PositionInViewport.Y -= 50.f;

		FVector2D SizeInViewport(300.f, 25.f);

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);	// 위치 조정
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);	// 크기 조정
	}
}

void AMainPlayerController::DisplayEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputModeGameAndUI;
		SetInputMode(InputModeGameAndUI);	// UI 클릭 할 수 있게 설정
		bShowMouseCursor = true;		// 마우스 보이게
	}
}
void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = false;
		//PauseMenu->SetVisibility(ESlateVisibility::Hidden);	// BP에서 실행함

		FInputModeGameOnly InputModeGameOnly;
		SetInputMode(InputModeGameOnly);
		bShowMouseCursor = false;
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu();
	}
	else
	{
		DisplayPauseMenu();
	}
}
