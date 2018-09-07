#pragma once

#include <vector>
#include <SDL.h>
#include <functional>
#include <iostream>

class Tilemap;

typedef struct Point {
	int x;
	int y;
	inline Point operator+(const Point& other) const { return {x+other.x, y+other.y}; }
	inline bool operator==(const Point& other) const { return x == other.x && y == other.y; }
} Point;

class Renderer;

class Unit {
private:
	Point _loc;
public:
	Unit(const Point& loc) : _loc(loc) {}
	inline const Point& GetPoint() const { return _loc; }
	//void Move(const TileMap& tilemap, const Point& destination);
	//void Render() {}
};

class TileMap {
private:
	Point _size;
	Point *_hover = nullptr;
public:
	std::vector<Point> GetPoints() const {
		std::vector<Point> ret;
		for (int x = 0; x < _size.x; x++) {
			for (int y = 0; y < _size.y; y++) {
				ret.push_back({ x, y });
			}
		}
		return ret;
	}
	bool ValidPoint(const Point& loc) const {
		return loc.x >= 0 && loc.y >= 0 && loc.x < _size.x && loc.y < _size.y;
	}
	
	inline TileMap(const Point& size) : _size(size) {}
	
	void SetHover(const SDL_Point& mouse, std::function<Point(const SDL_Point&, const Point&)> transform) {
		if (_hover != nullptr) {
			delete _hover;
		}
		_hover = new Point(transform(mouse, _size));
	}
	//void Render(SDL_Renderer *renderer, std::function<SDL_Point(const Point&, const Point&)> transform) const {
	//
	//	if (_hover != nullptr) {
	//		DrawFunction(*_hover, { 0xFF,0x00,0x00,0xFF }, true);
	//	}
	//	for (auto p : GetPoints()) {
	//		DrawFunction(p, { 0xFF,0xFF,0xFF,0xFF }, false);
	//	}
	//}
	const std::vector<Point>& GetAdjacent(const TileMap& tilemap, const Point& source) const {
		std::vector<Point> ret;
		std::vector<Point> deltas = { { -1,0 },{ +1,0 },{ 0,-1 },{ 0,+1 } };
		for (auto d : deltas) {
			const auto loc = source + d;
			if (tilemap.ValidPoint(loc))
				ret.push_back(loc);
		}
		return ret;
	}
	inline const Point& GetSize() const { return _size; }

	inline bool HasHover() const { return _hover != nullptr; }
	inline const Point& GetHover() const { return *_hover; }

	void Cleanup() {
		if (_hover != nullptr) {
			delete _hover;
			_hover = nullptr;
		}
	}
};

class Renderer {
private:
	SDL_Renderer* _renderer = nullptr;
	std::vector<Unit>* _units = nullptr;
	TileMap* _tilemap = nullptr;
public:
	Renderer(std::vector<Unit>* units, TileMap* tilemap) :
		_units(units),
		_tilemap(tilemap)
	{}
	void Render(std::function<SDL_Point(const std::pair<double, double>&, const Point&)> transform) const {
		if (_renderer == nullptr) return;

		SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(_renderer);
		SDL_SetRenderDrawColor(_renderer, 0xFF, 0xFF, 0xFF, 0xFF);

		if (_tilemap != nullptr) {
			auto size = _tilemap->GetSize();
			auto DrawFunction = [&](const Point &point, const SDL_Color& color, const bool& fill, const double& scale = 1.0) {
				double offset = (1.0 - scale) / 2.0;
				auto a = transform({point.x + offset, point.y + offset}, size);
				offset = 1.0 - offset;
				auto b = transform({ point.x + offset, point.y + offset }, size);
				auto rect = SDL_Rect{ a.x, a.y,  b.x - a.x, b.y - a.y };
				SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
				auto RenderFunction = (fill) ? SDL_RenderFillRect : SDL_RenderDrawRect;
				RenderFunction(_renderer, &rect);
				if (fill) {
					std::cout << a.x << " " << a.y << " " << b.x << " " << b.y << std::endl;
				}
			};

			if (_tilemap->HasHover()) {
				DrawFunction(_tilemap->GetHover(), { 0xFF, 0x00, 0x00, 0xFF }, true, 1.0);
			}
			for (auto p : _tilemap->GetPoints()) {
				DrawFunction(p, { 0xFF, 0xFF, 0xFF, 0xFF }, false);
			}
			if (_units != nullptr) {
				for (auto u : *_units) {
					DrawFunction(u.GetPoint(), { 0x00, 0x00, 0xFF, 0xFF }, false, 0.75);
				}
			}

		}

		SDL_RenderPresent(_renderer);
	}
	bool Init(SDL_Window *window) {
		if ((_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == nullptr) { return false; }
		SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x00, 0xFF);
		return true;
	}
	void Cleanup() {
		if (_renderer != nullptr) {
			SDL_DestroyRenderer(_renderer);
			_renderer = nullptr;
		}
	}
	~Renderer() {
		_units = nullptr;
		_tilemap = nullptr;
	}
};

class Game {
private:
	static Game _instance;

	std::vector<Unit> _units;
	TileMap _tilemap;
	std::pair<double, double> _zoom;
	SDL_Point _window_size;
	
	Renderer renderer;

	SDL_Window *_window = nullptr;
	SDL_Surface *_surface = nullptr;

	bool _running = true;

	std::function<SDL_Point(const std::pair<double, double>&, const Point&)> _render_transform;
	std::function<Point(const SDL_Point&, const Point&)> _mouse_transform;

	Game() :
		_tilemap(TileMap(Point({ 16, 16 }))),
		_window_size({ 512, 512 }),
		renderer(Renderer(&_units, &_tilemap))
	{
		_units.push_back(Unit(Point{ 1,1 }));
		_render_transform = [&](const std::pair<double, double>& p, const Point& size) {
			return SDL_Point({
				(int)(p.first * (_window_size.x - 1) / (size.x - 1)),
				(int)(p.second * (_window_size.y - 1) / (size.y - 1))
			});
		};
		_mouse_transform = [&](const SDL_Point& p, const Point& size) {
			return Point({
				p.x * (size.x - 1) / (_window_size.x - 1),
				p.y * (size.x - 1) / (_window_size.y - 1)
			});
		};
	}
	void Render() const
	{
		renderer.Render(_render_transform);
	}
	
	void OnEvent(SDL_Event* sdl_event)
	{
		switch (sdl_event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (sdl_event->key.keysym.sym == SDLK_ESCAPE) {
				_running = false;
			}
			break;
		case SDL_MOUSEMOTION:
			_tilemap.SetHover(SDL_Point{ sdl_event->motion.x, sdl_event->motion.y }, _mouse_transform);
			break;
		}
	}

	bool Init()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0) { return false; }
		//if (0 == SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"));
		if ((_window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _window_size.x, _window_size.y, SDL_WINDOW_SHOWN)) == nullptr) { return false; }
		_surface = SDL_GetWindowSurface(_window);
		if (!renderer.Init(_window)) return false;
		return true;
	}

	void Loop()
	{
	}

	void Cleanup()
	{
		renderer.Cleanup();
		if (_window) {
			SDL_DestroyWindow(_window);
			_window = nullptr;
		}
	}
public:
	bool Execute()
	{
		if (!Init()) return false;
		SDL_Event sdl_event;
		while (_running) {
			while (SDL_PollEvent(&sdl_event) != 0) {
				OnEvent(&sdl_event);
				if (sdl_event.type == SDL_QUIT) {
					_running = false;
				}
			}
			Loop();
			Render();
			SDL_Delay(1);
		}
		_tilemap.Cleanup();
		Cleanup();
		return true;
	}

	inline static Game* GetInstance() { return &_instance; }
};

Game Game::_instance;