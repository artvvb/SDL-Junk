// SDL-Junk.cpp : Defines the entry point for the console application.
//

#define SDL_MAIN_HANDLED
#include "Game.h"
#include "stdafx.h"

int main()
{
    return (int) Game::GetInstance()->Execute();
}

