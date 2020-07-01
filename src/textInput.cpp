#include <textInput.h>

uint32_t textPointer = 0;
boolean btnHeld = 0;
boolean btnHeldField[10] = {0,0,0,0,0,0,0,0,0,0};
bool textLimitFlag = 0;
unsigned long prevKeyMillis = 0;
Buttons buttons;

int multi_tap(byte key)
{
	static boolean upperCase = true;
	static byte prevKeyPress = NO_KEY, cyclicPtr = 0;
	static const char multi_tap_mapping[10][map_width] = {
		{'0', '#', '$', '.', '?', '"', '&'},
		{'1', '+', '-', '*', '/', '\'', ':'},
		{'A', 'B', 'C', '2', '!', ';', '<'},
		{'D', 'E', 'F', '3', '%', '[', '='},
		{'G', 'H', 'I', '4', '(', '\\', '>'},
		{'J', 'K', 'L', '5', ')', ']', '^'},
		{'M', 'N', 'O', '6', '@', '_', '`'},
		{'P', 'Q', 'R', 'S', '7', '{', '|'},
		{'T', 'U', 'V', '8', ',', '}', '~'},
		{'W', 'X', 'Y', 'Z', '9', ' ', 0}};
	if (key == RESET_MTP) // Received reset command. Flush everything and get ready for restart.
	{
		upperCase = true;
		prevKeyPress = NO_KEY;
		cyclicPtr = 0;
		return 0;
	}

	if (key != NO_KEY && key != 'D' && key != 'C' && key != 'B' && key != 'A') // A key is pressed at this iteration.
	{
		if (key == '*')
		{
			prevKeyPress = NO_KEY;
			prevKeyMillis = 0;
			cyclicPtr = 0;
			return 0;
		}
		if ((key > '9') || (key < '0')) // Function keys
		{
			if ((key == 1) || (key == '#')) // Up for case change
			{
				upperCase = !upperCase;
				return 0;
			}
			else // Other function keys. These keys produce characters so they need to terminate the last keypress.
			{
				if (prevKeyPress != NO_KEY)
				{
					char temp1 = multi_tap_mapping[prevKeyPress - '0'][cyclicPtr];
					if ((!upperCase) && (temp1 >= 'A') && (temp1 <= 'Z'))
						temp1 += 'a' - 'A';
					cyclicPtr = 0;
					prevKeyMillis = 0;
					switch (key)
					{
					case 2:
						// Call symbol list
						return 0; // Clear the buffer.
						break;
					case 3:
						prevKeyPress = '\b';
						break;
					case 5:
						prevKeyPress = '\n';
						break;
					case 6:
						prevKeyPress = NO_KEY; // Clear the buffer.
						break;
					}
					return (256 + (unsigned int)(temp1));
				}
				else
				{
					prevKeyPress = NO_KEY;
					cyclicPtr = 0;
					prevKeyMillis = 0;
					switch (key)
					{
					case 2:
						// Call symbol list
						return 0; // Clear the buffer.
						break;
					case 3:
						return (256 + (unsigned int)('\b'));
						break;
					case 4:
						return (256 + (unsigned int)(' '));
						break;
					case 5:
						return (256 + (unsigned int)('\n'));
						break;
					case 6:
						return 0; // Clear the buffer.
						break;
					}
				}
			}
		}
		if (prevKeyPress != NO_KEY)
		{
			if (prevKeyPress == key)
			{
				// Serial.print("cycling key: ");
				// Serial.println(char(key));
				char temp1;
				cyclicPtr++;
				if ((multi_tap_mapping[key - '0'][cyclicPtr] == 0) || (cyclicPtr == map_width))
					cyclicPtr = 0; //Cycle key
				prevKeyMillis = millis();
				temp1 = multi_tap_mapping[key - '0'][cyclicPtr];
				if ((!upperCase) && (temp1 >= 'A') && (temp1 <= 'Z'))
					temp1 += 'a' - 'A';
				return ((unsigned int)(temp1));
			}
			else
			{
				// Serial.println("skipping to next key");
				// Serial.print("previous: ");
				// Serial.print(char(prevKeyPress));
				// Serial.print("  current: ");
				// Serial.println(char(key));
				char temp1 = multi_tap_mapping[prevKeyPress - '0'][cyclicPtr];
				if ((!upperCase) && (temp1 >= 'A') && (temp1 <= 'Z'))
					temp1 += 'a' - 'A';
				prevKeyPress = key;
				cyclicPtr = 0;
				prevKeyMillis = millis();
				//Print key on cursor+1
				return (256 + (unsigned int)(temp1));
			}
		}
		else
		{
			char temp1 = multi_tap_mapping[key - '0'][cyclicPtr];
			if ((!upperCase) && (temp1 >= 'A') && (temp1 <= 'Z'))
				temp1 += 'a' - 'A';
			prevKeyPress = key;
			prevKeyMillis = millis();
			cyclicPtr = 0;
			return ((unsigned int)(temp1));
		}
	}
	else // No key is pressed at this iteration.
	{
		if (prevKeyPress == NO_KEY)
			return 0;											 // No key was previously pressed.
		else if (millis() - prevKeyMillis < multi_tap_threshold) // Key was pressed previously but within threshold
		{
			char temp1 = multi_tap_mapping[prevKeyPress - '0'][cyclicPtr];
			if ((!upperCase) && (temp1 >= 'A') && (temp1 <= 'Z'))
				temp1 += 'a' - 'A';
			return ((unsigned int)(temp1));
		}
		else // Key was pressed previously and threshold has passed
		{
			char temp1 = multi_tap_mapping[prevKeyPress - '0'][cyclicPtr];
			if ((!upperCase) && (temp1 >= 'A') && (temp1 <= 'Z'))
				temp1 += 'a' - 'A';
			prevKeyPress = NO_KEY;
			cyclicPtr = 0;
			return (256 + (unsigned int)(temp1));
		}
	}
	return 0;
}
String textInput(String buffer, int16_t length)
{
	int ret = 0;
	byte key = buttons.getKey(); // Get a key press from the keypad
	// if(key != NO_KEY)
	// {
	// 	Serial.println((char)key);
	// }
	if (buttons.released(BTN_FUN_4) && buffer != "")
	{
		if (textPointer == buffer.length()){
			textPointer--;
			btnHeld = false;
		}
		buffer.remove(buffer.length() - 1);
	}
	else if (key == '*')
	{
		if (textPointer != buffer.length())
			textPointer++;
		textPointer++;
		buffer += ' ';
	}
	
	if(!textLimitFlag && !(buffer.length() == length) && !btnHeld){
		for(int i = 0; i < 10; i++){
			uint8_t button = 0;
			switch (i)
			{
			case 0:
				button = BTN_0;
				break;
			case 1:
				button = BTN_1;
				break;
			case 2:
				button = BTN_2;
				break;
			case 3:
				button = BTN_3;
				break;
			case 4:
				button = BTN_4;
				break;
			case 5:
				button = BTN_5;
				break;
			case 6:
				button = BTN_6;
				break;
			case 7:
				button = BTN_7;
				break;
			case 8:
				button = BTN_8;
				break;
			case 9:
				button = BTN_9;
				break;
			default:
				break;
			}
			if(buttons.held(button, 120)){
				if(i == 0){
					buffer+='+';
				}
				else{
					buffer+=char(i+'0');
				}
				textPointer++;
				btnHeld = true;
				btnHeldField[i] = true;
				break;
			}
		}
		if(buffer.length() == length)
			textLimitFlag = 1;
	}

	if (textLimitFlag && buffer.length() == length)
		return buffer;
	else
		textLimitFlag = 0;

	if ((length == -1 || length >= buffer.length()) && !btnHeld)
	{
		ret = multi_tap(key); // Feed the key press to the multi_tap function.
		if ((ret & 256) != 0) // If this is non-zero, we got a key. Handle some special keys or just print the key on screen
		{
			// Serial.println("got key");

			if (buffer.length() == length)
			{
				textLimitFlag = 1;
				return buffer;
			}
			else{
				textPointer++;
			}
		}
		else if (ret) // We don't have a key but the user is still cycling through characters on one key so we need to update the screen
		{
			// Serial.print("text pointer: ");
			// Serial.println(textPointer);
			// Serial.println(buffer.length());
			if (textPointer >= buffer.length())
				buffer += char(lowByte(ret));
			else
				buffer[buffer.length() - 1] = char(lowByte(ret));
		}
	}
	
	if(btnHeld){
		if (btnHeldField[9] && buttons.released(10)) {
			btnHeldField[9] = false;
			btnHeld = false;
		}
		else{
			for(int i = 0; i < 9; i++){
				uint8_t button = 0;
				switch (i)
				{
				case 0:
					button = BTN_0;
					break;
				case 1:
					button = BTN_1;
					break;
				case 2:
					button = BTN_2;
					break;
				case 3:
					button = BTN_3;
					break;
				case 4:
					button = BTN_4;
					break;
				case 5:
					button = BTN_5;
					break;
				case 6:
					button = BTN_6;
					break;
				case 7:
					button = BTN_7;
					break;
				case 8:
					button = BTN_8;
					break;
				case 9:
					button = BTN_9;
					break;
				default:
					break;
				}
				if(btnHeldField[i] && buttons.released(button)){
						btnHeldField[i] = false;
						btnHeld = false;
						break;	
				}
			}
		}
	}
	if(buffer.length() > length)
	{
		buffer = buffer.substring(0, length);
		textPointer--;
	}
	return buffer;
}