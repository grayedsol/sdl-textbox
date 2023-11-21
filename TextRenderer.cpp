#include "TextRenderer.hpp"

bool TextRenderer::callForNewLine(TextBox& textbox) {
	if (textbox.callingForNewLine) {
		textbox.offsetY -= 1;
		return true;
	}
	else if (textbox.newLinesRemaining > 0) {
		textbox.callingForNewLine = true;
		textbox.newLinesRemaining--;
		textbox.totalNewLines++;
		return true;
	}
	else {
		return false;
	}
}

void TextRenderer::printChar(Font font, char character, Position pos) {
	SDL_FRect dstrect{
		(pos.x + universalOffsetX),
		(pos.y + universalOffsetY),
		font.sourceRects.at((size_t)character - ' ').w,
		font.sourceRects.at((size_t)character - ' ').h
	};
	//Subtract 32 because that's how the fonts work.
	//They start at ASCII 32, so the first character in the font is space.
	SDL_RenderTexture(*renderer, font.texture, &font.sourceRects.at((size_t)character - 32), &dstrect);
}

float TextRenderer::getNextWordLength(Font font, std::vector<char> string, int startIndex) {
	float len = 0.0f;
	for (int i = startIndex; i < string.size(); i++) {
		if (string.at(i) == '\n' || string.at(i) == ' ') { break; }
		len += font.sourceRects.at((size_t)string.at(i) - ' ').w;
	}
	return len;
}

int TextRenderer::printLine(TextBox textbox, Message msg, Position cursor, Font font, int start)
{
	//Define the width of the line
	float widthRemaining = textbox.dstRect.w;
	for (int i = start; i < msg.index; i++) {
		int character = msg.text.at(i);
		//If it's the new line character, we don't want to print to this line anymore,
		//so return sending the next index.
		if (character == '\n') { return i + 1; }
		//Define the sourceRect of the target character (used to determine its width).
		SDL_FRect sourceRect = font.sourceRects.at((size_t)character - 32);

		if (character == ' ') {
			//See if the next word will fit within the remaining space
			float wordWidth = sourceRect.w + getNextWordLength(font, msg.text, i + 1);
			//If it will not fit in the width remaining, but it can fit on a new line,
			//then return sending the next index that is the start of the word.
			if (wordWidth > widthRemaining && wordWidth < textbox.dstRect.w) {
				return i + 1;
			}
		}
		//The width of a character in the font should not exceed the width of the entire textbox.
		if (sourceRect.w > textbox.dstRect.w) {
			SDL_Log("Tried to print to a textbox, but a character in the message was too big to fit.\n");
			SDL_Log("Try reducing the font scaling of the message or enlarging the textbox.\n");
			assert(false);
			return 0;
		}
		//Print the character if it will fit in the remaining space
		else if (widthRemaining >= sourceRect.w) {
			printChar(font, character, cursor);
			cursor.x += sourceRect.w;
			widthRemaining -= sourceRect.w;
		}
		//Since there wasn't space on this line,
		//Return the index so that it will print to the next line.
		else {
			return i;
		}
	}
	return msg.index;
}

bool TextRenderer::printMultiLine(TextBox& textbox, Message msg, Font font)
{
	Position cursor = Position{ 0, 0 + textbox.offsetY};
	textbox.currentBoxLine = -textbox.totalNewLines;
	//Index of the current character to be typed in the message. Starts at 0, and will increase
	//to type all characters up to the message's index value.
	int currentIndex = 0;
	while (currentIndex < msg.index) {
		//If there is no space to fit the line, then try calling for a new line.
		if (cursor.y + textbox.lineSpacing + font.heightChars > textbox.dstRect.h) {
			return callForNewLine(textbox);
		}
		//Print the line, updating the currentIndex with printLine's return value.
		currentIndex = printLine(textbox, msg, cursor, font, currentIndex);
		cursor.x = 0;
		cursor.y += textbox.lineSpacing + font.heightChars;
		textbox.currentBoxLine++;
	}
	//If we have typed up to the new line character, scroll the text until only the only text visible
	//is the new text that has appeared since the last time the player pressed the button.
	if (msg.text.at(msg.index) == '\n') {
		if (cursor.y > (textbox.currentBoxLine - textbox.newLinesRemaining) * (textbox.lineSpacing + font.heightChars)) {
			textbox.callingForNewLine = true;
			return callForNewLine(textbox);
		}
		//Update the totalNewLines and newLinesRemaining values once we are done scrolling.
		else { textbox.totalNewLines += textbox.newLinesRemaining; textbox.newLinesRemaining = 0; }
	}
	//We had space to print all the characters, so turn off callingForNewLine in case it was true.
	textbox.callingForNewLine = false;
	//If we are at the newline character, return false because the player needs to press the button
	//to proceed.
	return (msg.text.at(msg.index) != '\n');
}

TextRenderer::TextRenderer(const char* fontFile) {
	defaultFont = loadFont(openJson(fontFile));
}

TextRenderer::~TextRenderer() {
	SDL_DestroyTexture(defaultFont.texture);
}

bool TextRenderer::processTextBox(TextBox& textbox, Message& msg)
{
	bool messageComplete = false;
	//Convert the textbox destination rectangle to a viewport by changing to an int rect
	const SDL_Rect viewport = convertFloatRect(textbox.dstRect);
	//Change the viewport to only render in the textbox area
	SDL_SetRenderViewport(*renderer, &viewport);
	if (printMultiLine(textbox, msg, defaultFont)) {
		//Update timer
		msg.timer -= game->getDelta() * msg.typeSpeedMultiplier;
		if (msg.timer <= 0 && !textbox.callingForNewLine) { msg.index++; msg.timer = msg.typeSpeedInterval; }
		//Speed up the message if the player presses the button.
		if (game->getSingleKeyInput() == SDL_SCANCODE_E) {
			msg.timer = 0; msg.typeSpeedMultiplier += 5.0f;
		}
	}
	else {
		//Wait for player to press button to proceed to next line, or end the interaction if done.
		if (game->getSingleKeyInput() == SDL_SCANCODE_E) {
			//Give the box newLinesRemaining based on where the line where the
			//new text will start printing.
			textbox.newLinesRemaining = textbox.currentBoxLine;
			msg.index++;
			msg.typeSpeedMultiplier = 1.0f;
			//End the interaction if it has completely typed out the message.
			if (msg.index >= msg.text.size()) { messageComplete = true; }
		}
	}
	//Change the viewport back to normal
	SDL_SetRenderViewport(*renderer, NULL);
	return messageComplete;
}

void TextRenderer::process() {
	//Retrieve textbox and message data from your game, then run something like:
	//if (processTextBox(textbox, msg)) {
		//endDialogue(textbox, msg);
	//}
}

Font TextRenderer::loadFont(SDL_Texture* texture, std::vector<float> widths, float heightChars, float emWidth) {
	Font font;
	font.heightChars = heightChars;
	font.texture = texture;

	int textureWidth;
	int textureHeight;
	SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);

	float currentWidth = 0;
	float currentHeight = 0;
	int index = 0;

	while (currentHeight + heightChars <= textureHeight) {
		while (currentWidth + widths.at(index) <= textureWidth) {
			SDL_FRect character{
				currentWidth, currentHeight,
				widths.at(index), heightChars };
			font.sourceRects.push_back(character);
			currentWidth += emWidth;
			index++;
			if (index >= widths.size()) { return font; }
		}
		currentWidth = 0;
		currentHeight += heightChars;
	}
	return font;
}

Font TextRenderer::loadFont(const rapidjson::Value& font) {
	//Use your own texture loading system
	SDL_Texture* texture;// = game->loadTexture(font["texture"].GetString());

	std::vector<float> widths;
	for (auto& value : font["widths"].GetArray()) { widths.push_back(value.GetFloat()); }

	float heightChars = font["heightChars"].GetFloat();
	float emWidth = font["emWidth"].GetFloat();

	return loadFont(texture, widths, heightChars, emWidth);
}
