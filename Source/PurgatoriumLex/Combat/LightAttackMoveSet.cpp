// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/LightAttackMoveSet.h"

FLightAttackMoveDef ULightAttackMoveSet::FindMove(EPosture Posture) const
{
	for (const FLightAttackMoveDef& Move : Moves)
	{
		if (Move.Posture == Posture)
		{
			return Move;
		}
	}

	// Fallback so missing data still produces a complete move (easy to spot in logs).
	FLightAttackMoveDef Fallback;
	Fallback.Posture = Posture;
	Fallback.StartupFrames = 8;
	Fallback.ActiveFrames = 3;
	Fallback.RecoveryFrames = 12;
	Fallback.Damage = 10.f;
	return Fallback;
}
