#pragma once

#include "resource.h"

enum Position
{
	Position_UP, Position_DOWN, Position_LEFT, Position_RIGHT, Position_LEFT_UP, Position_LEFT_DOWN, Position_RIGHT_UP, Position_RIGHT_DOWN
};
enum AIColor
{
	AIColor_WHITE, AIColor_BLACK
};
enum FreeLevel
{
	FreeLevel_ALL_FREE, FreeLevel_HALF_FREE, FreeLevel_UN_FREE
};