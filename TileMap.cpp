#include "App.h"
#include <algorithm>
#include <tuple>
#include <iostream>

TileMap::TileMap(const int width, const int height) :
	_size({ width, height }),
	_move_range(4),
	_weight_max(3),
	_tiles(),
	_units(),
	_hovered(),
	_selected(),
	_selected_unit_range()
{
	auto &[x_max, y_max] = _size;
	for (int y = 0; y < y_max; y++) {
		for (int x = 0; x < x_max; x++) {
			_tiles.push_back(Tile(x % 4 + 1, { x, y }));
		}
	}

	_units.push_back({ 2, 2 });
	_units.push_back({ 3, 2 });
}

const Location TileMap::PointToLocation(const SDL_Rect &region, const SDL_Point &target)
{
	return {
		(target.x - region.x) / (region.w / _size.first),
		(target.y - region.y) / (region.h / _size.second)
	};
}

const void SetRenderDrawColorFromPallette(SDL_Renderer *renderer, const ColorPallette &pallette, const float &brightness)
{
	const auto color = PalletteToColor(pallette);
	SDL_SetRenderDrawColor(renderer, (Uint8)(color.r * brightness), (Uint8)(color.g * brightness), (Uint8)(color.b * brightness), color.a);
}

typedef struct {
	int tile_width;
	int tile_height;
	int x_offset;
	int y_offset;
	int width;
	int height;
	ColorPallette color;
	bool fill;
} TileRenderer;

void DoRender(SDL_Renderer *sdl_renderer, const Location &location, const TileRenderer &renderer, const float &brightness) {
	const SDL_Rect tile_region = {
		location.first * renderer.tile_width + renderer.x_offset,
		location.second * renderer.tile_height + renderer.y_offset,
		renderer.width,
		renderer.height
	};
	SetRenderDrawColorFromPallette(sdl_renderer, renderer.color, brightness);
	if (renderer.fill) {
		SDL_RenderFillRect(sdl_renderer, &tile_region);
	}
	else {
		SDL_RenderDrawRect(sdl_renderer, &tile_region);
	}
}

const void TileMap::Render(SDL_Renderer *sdl_renderer, const SDL_Rect &region)
{
	const int tile_width  = region.w / _size.first;
	const int tile_height = region.h / _size.second;


	std::vector<std::pair<std::vector<Tile>*, TileRenderer>> arr = {
		{&_tiles, { tile_width, tile_height, region.x, region.y, tile_width, tile_height, ColorPallette_White, false } },
		{&_tiles, { tile_width, tile_height, region.x+1, region.y+1, tile_width-2, tile_height-2, ColorPallette_White, true } }
	};
	for (auto &[vec, renderer] : arr) {
		for (auto tile : *(vec)) {
			float brightness;
			if (renderer.fill)
				brightness = ((float)(tile.GetWeight()-1) / _weight_max);
			else
				brightness = 1.0;
			DoRender(sdl_renderer, tile.GetLocation(), renderer, brightness);
		}
	}
	std::vector<Location> range;
	for (auto [loc, delta] : _selected_unit_range) {
		range.push_back(loc);
	}
	std::vector <std::pair<std::vector<Location>*, TileRenderer>> rdr_tile_ptr = {
		{ &_units,              { tile_width, tile_height, region.x + tile_width / 8,     region.y + tile_height / 8,     tile_width * 3 / 4, tile_height * 3 / 4, ColorPallette_Red,   true  } },
		{ &_selected,           { tile_width, tile_height, region.x + tile_width / 8,     region.y + tile_height / 8,     tile_width * 3 / 4, tile_height * 3 / 4, ColorPallette_White, false } },
		{ &range,				{ tile_width, tile_height, region.x + tile_width / 4,     region.y + tile_height / 4,     tile_width / 2,     tile_height / 2,     ColorPallette_Green, false } },
		{ &_hovered,            { tile_width, tile_height, region.x,                      region.y,                       tile_width,         tile_height,         ColorPallette_Blue,  false } }
	};
	for (auto &[vec, renderer] : rdr_tile_ptr) {
		for (auto location : *(vec)) {
			DoRender(sdl_renderer, location, renderer, 1.0);
		}
	}

}

const std::vector<Location> TileMap::GetAdjacent(const Location &node) const
{
	std::vector<Location> vec = { { -1,0 },{ 1,0 },{ 0,-1 },{ 0,1 } };
	
	std::vector<Location> r;
	const auto &[nx, ny] = node;
	for (auto &[x, y] : vec) {
		const Location new_location = { nx + x, ny + y };
		if (std::find(_tiles.begin(), _tiles.end(), new_location) == _tiles.end())
			continue;
		r.push_back(new_location);
	}
	return r;
}


std::vector<RangeElement> TileMap::FillRange(const Location &source, const int &max_distance)
{
	const static std::vector<LocationDelta> vec = { { -1,0 },{ 1,0 },{ 0,-1 },{ 0,1 } };
	std::vector<EdgeElement> edge;
	std::vector<RangeElement> range;

	edge.push_back({ 0, { source, { 0, 0 } } });
	
	while (edge.size() > 0) {
		const auto [best_distance, best_node] = edge.back();
		const auto &[best_location, best_delta] = best_node;
		const auto &[x, y] = best_location;
		edge.pop_back();
		range.push_back(best_node);
		
		for (LocationDelta inst_delta : vec) {
			const auto &[dx, dy] = inst_delta;
			const Location inst_location = { x + dx, y + dy };
			
			const auto tile = std::find(_tiles.begin(), _tiles.end(), inst_location);
			if (tile == _tiles.end()) continue; // not a valid location

			const int inst_distance = tile->GetWeight() + best_distance;
			if (inst_distance > max_distance) continue; // out of range

			const auto unit = std::find(_units.begin(), _units.end(), inst_location);
			if (unit != _units.end() && *unit != source) continue; // disallow movement through/into other units

			auto range_pattern = [&inst_location](RangeElement const& element) { return element.first == inst_location; };
			if (std::find_if(range.begin(), range.end(), range_pattern) != range.end()) continue; // better path already exists

			auto edge_pattern = [&inst_location](EdgeElement const& element) { return element.second.first == inst_location; };
			const auto itr = std::find_if(edge.begin(), edge.end(), edge_pattern);
			
			const RangeElement inst_range = { inst_location, inst_delta };
			const EdgeElement inst_edge = { inst_distance, inst_range };
			if (itr == edge.end()) { // candidate location not already in edge
				edge.push_back(inst_edge);
			} else if (inst_distance < (*itr).first) { // candidate location already in edge, but instance is a shorter path
				edge.erase(itr);
				edge.push_back(inst_edge);
			}
		}
		std::sort(edge.begin(), edge.end(), [](const EdgeElement &a, const EdgeElement &b) { return a.first > b.first; });
	}
	return range;
}

void TileMap::MoveUnit(const Location &source, const Location &destination, const int &unit_range) {
	if (std::find(_tiles.begin(), _tiles.end(), source) == _tiles.end()) return; // cannot move from a location that does not exist
	if (std::find(_tiles.begin(), _tiles.end(), destination) == _tiles.end()) return; // cannot move to a location that does not exist
	auto unit_iter = std::find(_units.begin(), _units.end(), source);
	if (unit_iter == _units.end()) return; // cannot move a unit that does not exist
	if (std::find(_units.begin(), _units.end(), destination) != _units.end()) return; // cannot move onto another unit
	auto range = FillRange(source, unit_range);
	auto range_pattern = [&destination](const RangeElement &element) { return element.first == destination; };
	if (std::find_if(range.begin(), range.end(), range_pattern) == range.end()) return; // cannot move if the unit is not in range
	_units.erase(unit_iter);
	_units.push_back(destination);
}

void TileMap::Select(const SDL_Rect &region, const SDL_Point &target)
{
	const auto location = PointToLocation(region, target);
	
	if (_selected.size() == 0) {
		if (std::find(_tiles.begin(), _tiles.end(), location) == _tiles.end()) return; // cannot select a tile that does not exist
		if (std::find(_units.begin(), _units.end(), location) == _units.end()) return; // cannot select a unit that does not exist
		_selected.push_back(location); // select the unit
		_selected_unit_range = FillRange(location, _move_range); // load range with locations the unit can move to
	} else { // NOTE: selected should not contain more than one entry
		auto source = _selected.back();
		MoveUnit(source, location, _move_range);
		_selected.pop_back(); // unselect the unit
		_selected_unit_range.clear(); // clear range
	}
	return;
}

void TileMap::Hover(const SDL_Rect &region, const SDL_Point &target)
{
	const auto location = PointToLocation(region, target);
	if (std::find(_tiles.begin(), _tiles.end(), location) == _tiles.end())
		return;
	_hovered.clear();
	_hovered.push_back(location);
}