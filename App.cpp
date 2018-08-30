//==============================================================================
#include "App.h"
#include "Log.h"

const SDL_Color PalletteToColor(const ColorPallette &pallette) {
	switch (pallette) {
	case ColorPallette_White:  return SDL_Color({ 0xFF, 0xFF, 0xFF, 0xFF });
	case ColorPallette_Red:    return SDL_Color({ 0xFF, 0x00, 0x00, 0xFF });
	case ColorPallette_Green:  return SDL_Color({ 0x00, 0xFF, 0x00, 0xFF });
	case ColorPallette_Blue:   return SDL_Color({ 0x00, 0x00, 0xFF, 0xFF });
	case ColorPallette_Black:  return SDL_Color({ 0x00, 0x00, 0x00, 0xFF });
	case ColorPallette_Yellow: return SDL_Color({ 0xFF, 0xFF, 0x00, 0xFF });
	default: return SDL_Color({ 0xFF, 0xFF, 0xFF, 0xFF });
	}
}

App App::Instance;

//==============================================================================
App::App() :
	tiles(TileMap(16, 16))
{
}

//------------------------------------------------------------------------------
void App::OnEvent(SDL_Event* Event) {
	switch (Event->type) {
	case SDL_MOUSEBUTTONDOWN:
		tiles.Select(SDL_Rect{ 0,0,WindowWidth,WindowHeight }, SDL_Point{ Event->motion.x, Event->motion.y });
		break;
	case SDL_MOUSEMOTION:
		tiles.Hover(SDL_Rect{ 0,0,WindowWidth,WindowHeight }, SDL_Point{ Event->motion.x, Event->motion.y });
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if (Event->key.keysym.sym == SDLK_ESCAPE)
			Running = false;
		break;
	}
}

//------------------------------------------------------------------------------
void App::Render() {
	SDL_RenderClear(Renderer);

	tiles.Render(Renderer, SDL_Rect{ 0,0,WindowWidth,WindowHeight });

	SDL_SetRenderDrawColor(Renderer, 0x00, 0x00, 0x00, 0x00);

	SDL_RenderPresent(Renderer);
}

//------------------------------------------------------------------------------
bool App::Init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		Log("Unable to Init SDL: %s", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		Log("Unable to Init hinting: %s", SDL_GetError());
	}

	if ((Window = SDL_CreateWindow(
		"My SDL Game",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WindowWidth, WindowHeight, SDL_WINDOW_SHOWN)
		) == NULL) {
		Log("Unable to create SDL Window: %s", SDL_GetError());
		return false;
	}

	PrimarySurface = SDL_GetWindowSurface(Window);

	if ((Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
		Log("Unable to create renderer");
		return false;
	}

	SDL_SetRenderDrawColor(Renderer, 0x00, 0x00, 0x00, 0xFF);
	
	return true;
}

//------------------------------------------------------------------------------
void App::Loop() {
}

//------------------------------------------------------------------------------
void App::Cleanup() {
	if (Renderer) {
		SDL_DestroyRenderer(Renderer);
		Renderer = NULL;
	}

	if (Window) {
		SDL_DestroyWindow(Window);
		Window = NULL;
	}

	SDL_Quit();
}

//------------------------------------------------------------------------------
int App::Execute() {
	if (!Init()) return 0;

	SDL_Event Event;

	while (Running) {
		while (SDL_PollEvent(&Event) != 0) {
			OnEvent(&Event);

			if (Event.type == SDL_QUIT) Running = false;
		}

		Loop();
		Render();

		SDL_Delay(1); // Breath
	}

	Cleanup();

	return 1;
}

//==============================================================================
App* App::GetInstance() { return &App::Instance; }

int App::GetWindowWidth() { return WindowWidth; }
int App::GetWindowHeight() { return WindowHeight; }

//==============================================================================