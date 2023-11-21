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

	/// <summary>
	/// Load a font using parameters directly.
	/// </summary>
	/// <param name="texture:">Texture of the font.</param>
	/// <param name="widths:">Width of each character in the font. The first width should be ASCII 32 (space),
	///						  and it should continue numerically to ASCII 127 (DEL).</param>
	/// <param name="heightChars:">The height of each character in the font. This is a single value that will be
	///							   the same for all characters in the font.</param>
	/// <param name="emWidth:">Unit of measurement that describes the maximum width of a character, as well as
	///						   how far spaced apart they are in the texture.</param>
	/// <returns>The loaded font.</returns>
	Font loadFont(SDL_Texture* texture, std::vector<float> widths, float heightChars, float emWidth);

	/// <summary>
	/// Makes room for a new line in a textbox by increasing its offsetY value.
	/// Only 
	/// </summary>
	/// <param name="textbox:">Textbox that needs a new line.</param>
	/// <returns>true if successfully started or continued the scrolling action to make room for a new line.
	///			 false if it could not due to the textbox not having at least 1 in 'newLinesRemaining' value.
	/// </returns>
	bool callForNewLine(TextBox& textbox);

	/// <summary>
	/// Prints a character to the screen.
	/// </summary>
	/// <param name="font:">Font to use.</param>
	/// <param name="character:">Character to print.</param>
	/// <param name="pos:">Location to print.</param>
	void printChar(Font font, char character, Position pos);

	/// <summary>
	/// Returns the width of a word in a string in pixels, based on the font given.
	/// The "end" of the word is defined by the next space or new line character.
	/// </summary>
	/// <param name="font:">Font to use to measure width.</param>
	/// <param name="string:">String that contains the word.</param>
	/// <param name="startIndex:">The index in the string that is the first character in the word.</param>
	/// <returns>Width of the word.</returns>
	float getNextWordLength(Font font, std::vector<char> string, int startIndex = 0);

	/// <summary>
	/// Prints a line of text to a textbox. Starts printing at the 'start' index of the message.
	/// Ends printing at the message's index value.
	/// </summary>
	/// <param name="textbox:">The destination textbox.</param>
	/// <param name="msg:">The message data.</param>
	/// <param name="cursor:">Position to begin printing the line at.</param>
	/// <param name="font:">Font to use when printing.</param>
	/// <param name="start:">Index of the message to start at.</param>
	/// <returns>The index of the next character to print that couldn't fit on the line,
	///		     or if the message's index value if it printed up to that index.</returns>
	int printLine(TextBox textbox, Message msg, Position cursor, Font font, int start);

	/// <summary>
	/// Prints a message to a textbox using multiple lines, if needed.
	/// Starts from the beginning of the message, printing up to the message's index value.
	/// </summary>
	/// <param name="textbox:">Textbox to print to.</param>
	/// <param name="msg:">Message data to use.</param>
	/// <param name="font:">Font to use.</param>
	/// <returns>true if the message was printed up to the message's index value, or if it didn't
	///			 but was able to successfully start/continue making space for a new line.
	///			 false if the message was not printed up to the message's index value,
	///			 and it cannot call for a new line due to the textbox not having enough
	///			 newLinesRemaining, or if the message's index has reach a new line character.
	///			 Returning false means to wait for the player to press the button.</returns>
	bool printMultiLine(TextBox& textbox, Message msg, Font font);

	static SDL_Rect convertFloatRect(SDL_FRect floatRect) {
		return SDL_Rect{
			(int)floatRect.x, (int)floatRect.y,
			(int)floatRect.w, (int)floatRect.h
		};
	}
	
public:

	TextRenderer(const char* fontFile);

	~TextRenderer();

	/// <summary>
	/// Processes a textbox, given a message. The behavior of a textbox on a high level is as follows:
	/// Print each character in the message on to the screen, one by one.
	/// Words are not broken across lines, unless they are greater than the width of the entire textbox.
	/// Player must press a button to proceed under one of three conditions:
	///		Message has typed out to a new line character.
	///		Message has filled the textbox with all new text since the last time the button was pressed.
	///		Message has completely been typed out (pressing button ends message).
	/// In addition, the textbox will scroll upwards to make room for a new line whenever necessary.
	/// </summary>
	/// <param name="textbox:">Textbox to print to.</param>
	/// <param name="msg:">Message data.</param>
	/// <returns>true if the message has completed typed out and the player pressed the button.
	///	     false otherwise.</returns>
	bool processTextBox(TextBox& textbox, Message& msg);

	void process();

	/// <summary>
	/// Load a font from a JSON file.
	/// </summary>
	/// <param name="font:">Rapidjson value containing the entire font file.</param>
	/// <returns>The loaded font.</returns>
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
