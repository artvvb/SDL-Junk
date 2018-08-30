//==============================================================================
/*
Primary application class

3/11/2014
SDLTutorials.com
Tim Jones
*/
//==============================================================================
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>

#ifndef __APP_H__
#define __APP_H__

typedef enum {
	ColorPallette_White = 0,
	ColorPallette_Red,
	ColorPallette_Green,
	ColorPallette_Blue,
	ColorPallette_Black,
	ColorPallette_Yellow
} ColorPallette;

const SDL_Color PalletteToColor(const ColorPallette &pallette);

typedef std::pair<int, int> Location;

class Tile {
private:
	const int _move_weight;
	const Location _location;
public:
	Tile(const int &move_weight, const Location &location) :
		_move_weight(move_weight),
		_location(location)
	{

	}
	const Location &GetLocation() const {
		return _location;
	}
	const int &GetWeight() const {
		return _move_weight;
	}

	const bool &operator==(const Location &loc)const {
		return loc == _location;
	}
};

typedef std::pair<int, int> LocationDelta; // dx, dy
typedef std::pair<Location, LocationDelta> RangeElement; // loc, loc_delta
typedef std::pair<int, RangeElement> EdgeElement; // weight, range_element

class TileMap {
private:
	const std::pair<int, int> _size;
	std::vector<Tile> _tiles;
	std::vector<Location> _units;
	std::vector<Location> _hovered;
	std::vector<Location> _selected;
	std::vector<RangeElement> _selected_unit_range;
	const int _move_range;
	const int _weight_max;

	const Location PointToLocation(const SDL_Rect &region, const SDL_Point &target);
	const std::vector<Location> GetAdjacent(const Location &node) const;
	std::vector<RangeElement> FillRange(const Location &source, const int &distance);
	void MoveUnit(const Location &source, const Location &destination, const int &unit_range);
public:
	TileMap(const int width, const int height);
	const void Render(SDL_Renderer *renderer, const SDL_Rect &region);
	void Select(const SDL_Rect &region, const SDL_Point &target);
	void Hover(const SDL_Rect &region, const SDL_Point &target);
};

class App {
private:
	static App Instance;

	static const int WindowWidth = 1024;
	static const int WindowHeight = 768;
	
	TileMap tiles;

	int tile_hover;
	
	bool StateChanged = false;

	bool Running = true;

	SDL_Window* Window = nullptr;
	SDL_Renderer* Renderer = nullptr;
	SDL_Surface* PrimarySurface = nullptr;
	
private:
	App();

	// Capture SDL Events
	void OnEvent(SDL_Event* Event);

	// Initialize our SDL game / app
	bool Init();

	// Logic loop
	void Loop();

	// Render loop (draw)
	void Render();

	// Free up resources
	void Cleanup();

public:
	int Execute();

public:
	static App* GetInstance();

	static int GetWindowWidth();
	static int GetWindowHeight();
};

#endif