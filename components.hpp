#include "SDL3/SDL.h"

struct Font {
	SDL_Texture* texture;
	std::vector<SDL_FRect> sourceRects;
	//How tall each of the characters are.
	float heightChars;
};

struct TextBox {
	SDL_FRect dstRect;
	uint32_t lineSpacing;
	float offsetY = 0.0f;
	bool callingForNewLine = false;
	int newLinesRemaining = 0;
	int totalNewLines = 0;
	int currentBoxLine = 0;
};

struct Message {
	std::vector<char> text;
	int index = 0;
	float fontScaling = 1.0f;
	float typeSpeedInterval = 0.05f;
	float typeSpeedMultiplier = 1.0f;
	float timer = 0.0f;
};