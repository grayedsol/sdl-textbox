#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_image.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "components.hpp"
#include <cstring>

class TextRenderer {
private:

    SDL_Renderer** renderer;

	Font defaultFont;

	Font loadFont(SDL_Texture* texture, std::vector<float> widths, float heightChars, float emWidth);

	bool callForNewLine(TextBox& textbox);

	void printChar(Font font, char character, Position pos);

	float getNextWordLength(Font font, std::vector<char> string, int startIndex = 0);

	int printLine(TextBox textbox, Message msg, Position cursor, Font font, int start);
	bool printMultiLine(TextBox& textbox, Message msg, Font font);

	static SDL_Rect convertFloatRect(SDL_FRect floatRect) {
		return SDL_Rect{
			(int)floatRect.x, (int)floatRect.y,
			(int)floatRect.w, (int)floatRect.h
		};
	}
	
public:

	TextRenderer(SDLGame* game, const char* fontFile);

	~TextRenderer();

	bool processTextBox(TextBox& textbox, Message& msg);

	void process();

	Font loadFont(const rapidjson::Value& font);

	rapidjson::Document openJson(const char* path) {
		int BUFFER_SIZE = 65536;

		//Open json file safely
		FILE* fp;
		errno_t err = fopen_s(&fp, path, "rb");

		//Create a buffer, and pass it into the stream with the file
		std::unique_ptr<char[]> readBuffer = std::make_unique<char[]>(BUFFER_SIZE);
		rapidjson::FileReadStream is(fp, readBuffer.get(), sizeof(readBuffer));

		//Create a rapidjson Document, and parse it
		rapidjson::Document data;
		data.ParseStream(is);

		//Check that the document is valid
		assert(data.IsObject());
		//Close the file
		if (fp != 0) { fclose(fp); }

		return data;
	}

};