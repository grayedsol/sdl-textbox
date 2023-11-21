# sdl-textbox
Example code for a textbox that works like Dragon Warrior 7, and the DS remakes of Dragon Quest 4, 5, and 6.

This is not intended to work out of the box.

Uses: SDL3, rapidjson

The code creates an abstract textbox with the following properties:
 - Renders each character in the message on to the screen, one by one.
 - Words are not broken across lines, unless they are greater than the width of the entire textbox.
 - Player must press a button (in the code, it is E) to proceed under one of three conditions:
   * Message has typed out to a new line character.
   * Message has filled the textbox with all new text since the last time the button was pressed or the message was started.
   * Message has completely been typed out (pressing button ends message).
 - In addition, the textbox will scroll upwards to make room for a new line whenever necessary.
