#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <TFT_eSPI.h>
#include "textInput.h"
#include <SPIFFS.h>
#include <messages.h>
#include <RTClib.h>
#include <sprites.h>
#define NUM_MESSAGES_ONSCREEN 6
#define MAX_MESSAGES 15
PCF8563 RTC;
int counter = 0;
TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
TFT_eSprite display = TFT_eSprite(&tft);
TFT_eSprite _displayHelper = TFT_eSprite(&tft);
uint8_t cursor = 0;
int32_t cameraY = 0;
int32_t cameraYactual = 1;
int16_t messagesSize = 1;
uint8_t messageSelect = 1;
const uint16_t *memesArray[12] = {
	MemeAliens, MemeBrian, MemeCartman, MemeDistracted, MemeFWP, MemeJon,
	MemeNickYoung, MemeOver9000, MemePsycho, MemeRickRoll, MemeSaltBae, MemeSteve};
const uint8_t emojiCount = 20;
const uint16_t *emojiArray[20] = {
	EmojiSmile1, EmojiSmile2, EmojiSmile3, EmojiSmile4, EmojiSmile5, EmojiSmile7, EmojiLMAO,
	EmojiBlink, EmojiHeart, EmojiDollar, EmojiFire, EmojiPhone, EmojiPhone2, EmojiPoop, EmojiSad,
	Emoji100, EmojiCool, EmojiCry, EmojiCryCry, EmojiKiss};

String content, prevContent = "";
bool blinkState = 0;
int32_t y = 16;
uint32_t elapsedMillis = millis();
uint32_t refreshMillis = millis();
uint32_t milliserino = millis();
String received = "";
bool isRefreshed = 0;

void printEmoji(String emojiCode)
{
	int numCode = emojiCode.toInt();
	switch (numCode)
	{
	case 0:
		display.drawIcon(EmojiSmile1, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 1:
		display.drawIcon(EmojiSmile2, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 2:
		display.drawIcon(EmojiSmile3, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 3:
		display.drawIcon(EmojiSmile4, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 4:
		display.drawIcon(EmojiSmile5, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 5:
		display.drawIcon(EmojiSmile7, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 6:
		display.drawIcon(EmojiLMAO, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 7:
		display.drawIcon(EmojiBlink, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 8:
		display.drawIcon(EmojiHeart, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 9:
		display.drawIcon(EmojiDollar, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 10:
		display.drawIcon(EmojiFire, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 11:
		display.drawIcon(EmojiPhone, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 12:
		display.drawIcon(EmojiPhone2, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 13:
		display.drawIcon(EmojiPoop, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 14:
		display.drawIcon(EmojiSad, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 15:
		display.drawIcon(Emoji100, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 16:
		display.drawIcon(EmojiCool, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 17:
		display.drawIcon(EmojiCry, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 18:
		display.drawIcon(EmojiCryCry, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	case 19:
		display.drawIcon(EmojiKiss, display.cursor_x, display.cursor_y, 16, 16, 1, 0x07E0);
		break;
	}
}

void smileyCheck(String textCheck)
{
	int index = textCheck.indexOf(":");
	//bool emBegin = false;
	if (index == -1)
		display.print(textCheck);
	else
	{
		for (int i = 0; i < textCheck.length(); i++)
		{
			if (textCheck[i] == ':' && textCheck[i + 3] == ':')
			{
				//emBegin = true;
				printEmoji(textCheck.substring(i + 1, i + 3));
				display.print("   ");
				i += 3;
			}
			else
				display.print(textCheck[i]);
		}
	}
}

int8_t smileyPopup()
{
	uint8_t emojiSelect = 0;
	uint8_t startingHeight = 32;
	while (!buttons.released(BTN_FUN_3))
	{
		if (buttons.repeat(BTN_FUN_1, 10))
		{
			if(emojiSelect > 0)
				emojiSelect--;
			else
				emojiSelect = emojiCount - 1;
		}
		else if (buttons.repeat(BTN_FUN_2, 10))
		{
			if(emojiSelect < emojiCount - 1)
				emojiSelect++;
			else
				emojiSelect = 0;
		}
		// display.drawRoundRect(8, 44, 112, 76, 1, TFT_DARKGREY);
		// display.fillRoundRect(8, 43, 112, 76, 1, display.color565(0, 130, 255));
		display.fillRoundRect(8, startingHeight, 112, 76, 1, TFT_DARKGREY);
		uint8_t x = emojiSelect % 6;
		uint8_t y = emojiSelect / 6;
		display.fillRoundRect(9 + x * 18, startingHeight + 3 + y * 18, 18, 18, 2, TFT_RED);
		for(uint8_t i = 0; i < emojiCount; i++)
		{
			display.drawIcon(emojiArray[i], 10 + 18*(i%6), startingHeight + 4 + 18*(int(i/6)), 16, 16, 1,0x07E0);
		}
		display.pushSprite(0, 0);
		buttons.update();

		if (buttons.released(BTN_FUN_4)) return -1;
	}
	return emojiSelect;
}
int8_t memesPopup()
{
	uint8_t memeSelect = 0;
	uint32_t arrowMillis = millis();
	bool arrowState = 0;
	uint8_t startingHeight = 22;
	while (!buttons.pressed(BTN_FUN_3))
	{
		// display.fillRect(0, 0, 128, 118, display.color565(0, 0, 170));
		display.fillRect(0, 0, 128, 140, TFT_BLACK);

		// display.drawRoundRect(16, 34, 96, 96, 3, TFT_DARKGREY);
		// display.fillRoundRect(16, 33, 96, 96, 3, display.color565(0, 130, 255));
		display.fillRoundRect(16, startingHeight, 96, 96, 3, TFT_DARKGREY);
		//display.drawRoundRect(41, 31, 82, 76, 2, TFT_WHITE);
		if (buttons.pressed(BTN_FUN_2))
		{
			if (memeSelect < 11)
				memeSelect++;
		}
		else if (buttons.pressed(BTN_FUN_1))
		{
			if (memeSelect > 0)
				memeSelect--;
		}
		display.drawRect(31, startingHeight + 14, 66, 66, TFT_WHITE);

		display.drawIcon(memeLeftIcon, memeSelect > 0 ? 5 - arrowState*3 : 5, startingHeight + 41, 8, 13, 1, 0x57EA);
		display.drawIcon(memeRightIcon, memeSelect < 11 ? 115 + arrowState*3 : 115, startingHeight + 41, 8, 13, 1, 0x57EA);
		display.drawIcon(memesArray[memeSelect], 32, startingHeight + 15, 64, 64, 1, 0x07E0);
		display.pushSprite(0, 0);
		buttons.update();
		if(millis() - arrowMillis >= 350)
		{
			arrowMillis = millis();
			arrowState = !arrowState;
		}
		if (buttons.released(BTN_FUN_4))
			return -1;
	}
	return memeSelect;
}

int8_t optionsPopup()
{
	char* elementsArray[5] = {"Memes", "Emojis", "Details", "Delete", "Help"};
	uint8_t optionsSelect = 0;
	uint8_t menuYoffset = 49;
	// display.fillRoundRect(60, 44, 69, 88, 1, display.color565(0, 130, 130));
	// display.fillRect(62, 44, 66, 86, display.color565(0, 130, 255));
	display.fillRoundRect(60, menuYoffset, 69, 88, 1, TFT_DARKGREY);
	display.fillRect(62, menuYoffset, 66, 86, TFT_DARKGREY);
	display.setTextColor(TFT_WHITE);
	for(uint8_t i = 0; i < 5; i++)
	{
		display.setCursor(66, menuYoffset + i*17);
		display.print(elementsArray[i]);
	}
	// display.drawFastHLine(65, menuYoffset + optionsSelect*17 + 15, 40, display.color565(170, 0, 0));
	display.drawFastHLine(65, menuYoffset + optionsSelect*17 + 15, 40, TFT_WHITE);
	display.drawIcon(optionsSelectIcon, 116,  menuYoffset + optionsSelect*17 + 4, 8, 8, 1, 0x57EA);
	display.setCursor(66, menuYoffset + optionsSelect*17);
	display.print(elementsArray[optionsSelect]);
	display.pushSprite(0, 0);
	while(!buttons.released(BTN_FUN_4))
	{
		buttons.update();
		vTaskDelay(1);
	}
	while (!buttons.released(BTN_FUN_3))
	{
		// display.fillRoundRect(60, 44, 69, 88, 1, display.color565(0, 130, 130));
		// display.fillRect(62, 44, 66, 86, display.color565(0, 130, 255));
		display.fillRoundRect(60, menuYoffset, 69, 88, 1, TFT_DARKGREY);
		display.fillRect(62, menuYoffset, 66, 86, TFT_DARKGREY);
		display.setTextColor(TFT_WHITE);
		for(uint8_t i = 0; i < 5; i++)
		{
			display.setCursor(66, menuYoffset + i*17);
			display.print(elementsArray[i]);
		}
		// display.drawFastHLine(65, menuYoffset + optionsSelect*17 + 15, 40, display.color565(170, 0, 0));
		display.drawFastHLine(65, menuYoffset + optionsSelect*17 + 15, 40, TFT_WHITE);
		display.drawIcon(optionsSelectIcon, 116,  menuYoffset + optionsSelect*17 + 4, 8, 8, 1, 0x57EA);
		display.setCursor(66, menuYoffset + optionsSelect*17);
		display.print(elementsArray[optionsSelect]);
		display.pushSprite(0, 0);
		buttons.update();
		if (buttons.pressed(BTN_FUN_2))
		{
			if (optionsSelect < 4)
				optionsSelect++;
			else
				optionsSelect = 0;
		}
		else if (buttons.pressed(BTN_FUN_1))
		{
			if (optionsSelect > 0)
				optionsSelect--;
			else
				optionsSelect = 4;
		}
		if (buttons.released(BTN_FUN_4))
			return -1;
	}
	return optionsSelect;
}

int8_t selectPopup(uint8_t messageSelect)
{

	uint8_t selectSelect = 0;
	uint8_t windowLocation;
	if (messages[messageSelect]["sendOrReceiveFlag"].as<bool>())
		windowLocation = 2;
	else
		windowLocation = 60;
	while (!buttons.released(BTN_FUN_3))
	{
		display.fillRoundRect(windowLocation, 37, 96, 67, 2, TFT_BLACK);
		//display.fillRect(140, 102, 16, 6, TFT_BLACK);
		display.setTextColor(TFT_WHITE);
		display.setCursor(windowLocation + 6, 44);
		display.print(F("Details"));
		display.setCursor(windowLocation + 6, 64);
		display.print(F("Delete"));
		// display.setCursor(windowLocation + 6, 84);
		// display.print(F("Raw text"));
		if (buttons.pressed(BTN_FUN_2))
		{
			if (selectSelect < 2)
				selectSelect++;
			else
				selectSelect = 0;
		}
		else if (buttons.pressed(BTN_FUN_1))
		{
			if (selectSelect > 0)
				selectSelect--;
			else
				selectSelect = 2;
		}
		switch (selectSelect)
		{
		case 0:
			display.fillRoundRect(windowLocation + 4, 44, 46, 16, 2, TFT_LIGHTGREY);
			display.setTextColor(TFT_BLACK);
			display.setCursor(windowLocation + 6, 44);
			display.print(F("Details"));
			break;
		case 1:
			display.fillRoundRect(windowLocation + 4, 64, 44, 16, 2, TFT_LIGHTGREY);
			display.setTextColor(TFT_BLACK);
			display.setCursor(windowLocation + 6, 64);
			display.print(F("Delete"));
			break;
		// case 2:
		// 	display.fillRoundRect(windowLocation + 4, 84, 58, 16, 2, TFT_LIGHTGREY);
		// 	display.setTextColor(TFT_BLACK);
		// 	display.setCursor(windowLocation + 6, 84);
		// 	display.print(F("Raw text"));
		// 	break;
		}
		display.pushSprite(0, 0);
		buttons.update();
		if (buttons.pressed(BTN_FUN_4))
			return -1;
	}
	return selectSelect;
}

void detailsPopup(uint8_t messageSelect)
{
	uint8_t windowLocation = 2;
	while (!buttons.released(BTN_FUN_4))
	{
		display.fillRoundRect(windowLocation, 37, 86, 67, 2, display.color565(0, 130, 255));
		//display.fillRect(140, 102, 16, 6, TFT_BLACK);
		display.setTextColor(TFT_WHITE);
		display.setTextFont(2);
		display.setCursor(windowLocation + 4, 38);
		if (messages[messageSelect]["sendOrReceiveFlag"].as<bool>())
			display.print(F("Received at:"));
		else
			display.print(F("Sent at:"));
		display.setCursor(windowLocation + 4, 56);
		char format[23];
		strncpy(format, "hh:mm \n DD/MM/YYYY\0", 23);
		DateTime time = DateTime(messages[messageSelect]["time"].as<uint32_t>());
		display.print(time.format(format));
		display.pushSprite(0, 0);
		buttons.update();
	}
	Serial.println("EXITED");
}

void memoryReset()
{
	//SPIFFS.end();
	/*display.fillScreen(TFT_DARKGREY);
	display.fillRoundRect(26, 36, 108, 38, 4, TFT_RED);
	display.fillRoundRect(30, 40, 100, 30, 4, TFT_WHITE);
	display.setCursor(32, 42);
	display.setTextColor(TFT_BLACK);
	display.print("Formatting and");
	display.setCursor(32, 52);
	display.print("resetting...");
	display.pushSprite(0, 0);*/
	clearAllMessages();
	/*SPIFFS.format();
	ESP.restart();*/
}
void drawMainScreen()
{
	display.setTextColor(TFT_WHITE);
	display.setTextWrap(0);
	display.setTextSize(1);
	display.setTextFont(2);
	display.fillScreen(TFT_BLACK);
	// display.fillScreen(display.color565(0, 0, 170));

	// Serial.printf("cameraY: %d, cursor: %d, messageSelect: %d\n", cameraY, cursor, messageSelect);
	if (isRefreshed)
	{
		cameraYactual = (cameraYactual + cameraY) / 2;
		if (abs(cameraYactual - cameraY) == 1)
		{
			cameraYactual = cameraY;
		}
		isRefreshed = 0;
	}
	for (uint8_t j = 0; j < messages.size(); j++)
	{
		int8_t messagesYoffset = -20;
		uint8_t i = messages.size() - j;
		if (i * 23 + messagesYoffset - cameraYactual <= 140)//&& i * 23 + messagesYoffset - cameraYactual >= 10)
		{
			display.setCursor(-50, -50);
			uint16_t textLength = display.cursor_x;
			display.print(messages[i - 1]["text"].as<String>());
			textLength = display.cursor_x - textLength;
			if (messages[i - 1]["sendOrReceiveFlag"].as<bool>())
			{
				if ((i - 1) == messageSelect)
				{
					display.fillRoundRect(128 - textLength - 5, i * 23 + messagesYoffset - cameraYactual, textLength + 4, 18, 2, 0x7BDF);
					display.drawRoundRect(128 - textLength - 6, i * 23 + messagesYoffset - 1 - cameraYactual, textLength + 6, 20, 2, TFT_WHITE);
				}
				else{
					display.fillRoundRect(128 - textLength - 5, i * 23 + messagesYoffset - cameraYactual, textLength + 4, 18, 2, TFT_BLUE);
				}
				display.setCursor(128 - textLength - 3, i * 23 + messagesYoffset + 1 - cameraYactual);
				//display.print(messages[i - 1]["text"].as<String>());
				String textCheck = messages[i - 1]["text"].as<String>();
				smileyCheck(textCheck);
			}
			else
			{
				if ((i - 1) == messageSelect)
				{
					display.fillRoundRect(1, i * 23 + messagesYoffset - cameraYactual, textLength + 4, 18, 2, 0xFBCF);
					display.drawRoundRect(0, i * 23 + messagesYoffset - 1 - cameraYactual, textLength + 6, 20, 2, TFT_WHITE);
				}
				else{
					display.fillRoundRect(1, i * 23 + messagesYoffset - cameraYactual, textLength + 4, 18, 2, TFT_RED);
				}
				display.setCursor(3, i * 23 + messagesYoffset + 1 - cameraYactual);
				String textCheck = messages[i - 1]["text"].as<String>();
				/*if (textCheck == "format")
					memoryReset();
				else if (textCheck == "/Brian")
				{

					display.fillRoundRect(1, i * 23 + 14 - cameraYactual, 68, 68, 2, TFT_RED);
					display.drawIcon(Brian, display.cursor_x, display.cursor_y, 64, 64, 1, 0x07E0);
					break;
				}
				else if (textCheck == "/FWP")
				{

					display.fillRoundRect(1, i * 23 + 14 - cameraYactual, 84, 84, 2, TFT_RED);
					display.drawIcon(FWP, display.cursor_x, display.cursor_y, 80, 80, 1, 0x07E0);
					break;
				}
				else if (textCheck == "/Cartman")
				{

					display.fillRoundRect(1, i * 23 + 14 - cameraYactual, 84, 59, 2, TFT_RED);
					display.drawIcon(Cartman, display.cursor_x, display.cursor_y, 80, 55, 1, 0x07E0);
					break;
				}
				else if (textCheck == "/NickYoung")
				{

					display.fillRoundRect(1, i * 23 + 14 - cameraYactual, 84, 60, 2, TFT_RED);
					display.drawIcon(NickYoung, display.cursor_x, display.cursor_y, 80, 56, 1, 0x07E0);
					break;
				}
				else if (textCheck == "/Aliens")
				{

					display.fillRoundRect(1, i * 23 + 14 - cameraYactual, 84, 62, 2, TFT_RED);
					display.drawIcon(Aliens, display.cursor_x, display.cursor_y, 80, 58, 1, 0x07E0);
					break;
				}*/

				smileyCheck(textCheck);
				//display.print(textCheck[0]);
				//display.print(textCheck[1]);
				//display.print(messages[i - 1]["text"].as<String>());
				//display.drawIcon(SpriteUno, display.cursor_x, display.cursor_y, 24, 24, 1, 0x47E0);
				//display.drawIcon(newMessageIndicator, display.cursor_x, display.cursor_y, 20, 20, 1, 0x47E0);
				//display.drawIcon(newMessageIndicator, display.cursor_x + 30, display.cursor_y + 30, 20, 20, 1, 0xbbbb);
				//display.drawIcon(SpriteDue, display.cursor_x + 60, display.cursor_y + 60, 24, 24, 1, 0x0000);
				/*display.print("  ");
				display.setTextSize(1);
				display.setTextFont(1);
				display.setTextColor(TFT_LIGHTGREY);
				display.print(messages[i - 1]["time"].as<String>());
				DateTime now( = RTC.now();)
				display.print(now.month());
				display.setTextColor(TFT_WHITE);
				display.setTextSize(1);
				display.setTextFont(2);*/
			}
		}
	}
	
	//top banner
	// display.drawRoundRect(0, -2, 128, 21, 3, display.color565(0, 200, 255));
	// display.fillRoundRect(0, -2, 128, 20, 3, TFT_WHITE);

	//text box
	// display.drawRoundRect(0, 140, 128, 22, 3, display.color565(0, 200, 255));
	display.fillRoundRect(0, 140, 128, 21, 3, TFT_WHITE);

	//bottom banner
	// display.fillRoundRect(0, 140, 128, 23, 3, display.color565(0, 200, 255));
	// display.fillRect(0, 154, 128, 5, display.color565(0, 130, 255));
	// display.fillRect(0, 159, 128, 1, TFT_BLUE);
	// display.drawIcon(arrowUpIcon, 16, 146, 12, 9, 1, 0x57EA);
	// display.drawIcon(arrowDownIcon, 44, 146, 12, 9, 1, 0x57EA);
	// display.drawIcon(sendIcon, 71, 144, 11, 12, 1, 0x57EA);
	// display.drawIcon(IconOptions, 97, 144, 15, 12, 1, 0x57EA);

	prevContent = content;
	content = textInput(content, 12);
	if (prevContent != content)
	{
		blinkState = 1;
		elapsedMillis = millis();
	}
	display.setTextWrap(0);
	display.setCursor(1, 142);
	display.setTextColor(TFT_BLACK);

	// display.fillRect(0, 0, 128, 18, TFT_WHITE);
	for (int i = 0; i < content.length(); i++)
	{
		display.print(content[i]);
		if (display.getCursorX() > 150)
			display.print("\n");
	}
	if (blinkState == 1)
		display.drawFastVLine(display.getCursorX(), display.getCursorY() + 3, 10, TFT_BLACK);
}
void loraTask()
{
	while(1)
	{
		//check if received
		int packetSize = LoRa.parsePacket();
		// Serial.println("check if received?");
		if (packetSize)
		{
			Serial.print("Size: ");
			Serial.println(packetSize);

			// read packet
			bool findInString = 0;
			uint8_t counter = 0;
			for (uint32_t i = 0; i < received.length(); i++)
			{
				if (received[i] == '\n')
				{
					counter++;
					if (counter > 3)
					{
						findInString = 1;
						break;
					}
				}
			}
			char buffer[20];
			memset(buffer, 0, sizeof(buffer));
			if (findInString)
				received = "";
			while (LoRa.available())
			{
				char temp = (char)LoRa.read();
				strncat(buffer, &temp, 1);
			}
			// received+=" ";
			Serial.print(LoRa.packetRssi());
			Serial.println("dBm");
			// received+=LoRa.packetRssi();
			// received+="dBm";
			received += buffer;

			received += (char)'\n';
			// wait until the radio is ready to send a packet
			while (LoRa.beginPacket() == 0)
			{
				Serial.print("waiting for radio ... ");
				delay(100);
			}

			Serial.print("Sending ack: ");
			Serial.println(counter);

			// send in async / non-blocking mode
			LoRa.beginPacket();
			LoRa.print(char(6));
			LoRa.endPacket();
			
			addMessage(buffer, 1, RTC.now());
			if (messages.size() > 15)
			{
				messages.remove(0);
				saveMessages();
			}
			if (messages.size() > 5)
			{
				cursor = messages.size() - 5;
				cameraY = (cursor * 23);
			}
			messageSelect = messages.size() - 1;
		}
		vTaskDelay(1);
	}
}
void setup()
{
	SPIFFS.begin();
	Serial.begin(115200);

	// while (!Serial);
	// pinMode(25, OUTPUT); //off pin
	// digitalWrite(25, 0);
	pinMode(21, OUTPUT);
	digitalWrite(21, 0);
	Serial.println("start");
	delay(5);
	tft.init();
	tft.invertDisplay(0);
	tft.setRotation(0);
	display.setColorDepth(8); // Set colour depth of Sprite to 8 (or 16) bits
	display.createSprite(128, 160);
	display.setRotation(0);
	display.fillScreen(TFT_WHITE);
	display.pushSprite(0, 0);

	Serial.println("display");
	delay(5);
	buttons.begin();
	Serial.println("buttons");
	delay(5);
	LoRa.setPins(22, 17, 26);
	RTC.begin();
	RTC.adjust(DateTime(__DATE__, __TIME__));
	loadMessages();
	Serial.println("load messages");
	delay(5);
	/*
	case 0: return 7.8E3;
    case 1: return 10.4E3;
    case 2: return 15.6E3;
    case 3: return 20.8E3;
    case 4: return 31.25E3;
    case 5: return 41.7E3;
    case 6: return 62.5E3;
    case 7: return 125E3;
    case 8: return 250E3;
    case 9: return 500E3;
	*/

	// LoRa.setPins(5, -1, 4);
	// LoRa.setTxPower(20);
	// LoRa.setSpreadingFactor(12);
	// LoRa.setSignalBandwidth(7.8E3);
	// LoRa.setCodingRate4(8);
	// LoRa.setTxPower(20);
	// Serial.print("here");
	// delay(5);
	// LoRa.setSpreadingFactor(7);
	// Serial.print("here");
	// delay(5);
	// LoRa.setSignalBandwidth(250E3);
	// Serial.print("here");
	// delay(5);
	// LoRa.setCodingRate4(5);
	// Serial.print("here");
	// delay(5);
	// Serial.print("here");
	// delay(5);
	// LoRa.enableCrc();
	if (!LoRa.begin(433E6))
	{
		Serial.println("Starting LoRa failed!");
		// while (1)
		// 	yield();
	}
	Serial.print("lora OK");
	delay(5);
	if (messages.size() > NUM_MESSAGES_ONSCREEN)
	{
		cursor = messages.size() - NUM_MESSAGES_ONSCREEN;
		cameraYactual = (cursor * 23);
		cameraY = cameraYactual;
	}
	messageSelect = messages.size() > 0 ? messages.size() - 1 : 0;
}

void loop()
{
	if (millis() - elapsedMillis > 350)
	{
		elapsedMillis = millis();
		blinkState = !blinkState;
	}
	
	drawMainScreen();
	if (buttons.repeat(BTN_FUN_1, 50))
	{
		if (messageSelect <= cursor && cursor > 0)
		{
			cameraY -= 23;
			cursor--;
		}
		if (messageSelect > 0)
			messageSelect--;
	}
	if (buttons.repeat(BTN_FUN_2, 50))
	{
		if (messageSelect >= (NUM_MESSAGES_ONSCREEN - 1 + cursor)
		&& cursor < messages.size() - NUM_MESSAGES_ONSCREEN)
		{
			cameraY += 23;
			cursor++;
		}
		if (messageSelect < (messages.size() - 1))
			messageSelect++;
	}
if (buttons.pressed(BTN_FUN_3) && content != "")
	{
		bool ackReceived = 0;
		while (!ackReceived)
		{
			while (LoRa.beginPacket() == 0)
			{
				Serial.print("waiting for radio ... ");
				delay(100);
			}

			// Serial.print("Sending packet non-blocking: ");
			// Serial.println(counter);

			// // send in async / non-blocking mode
			LoRa.beginPacket();
			LoRa.print(content);
			LoRa.endPacket(1); // true = async / non-blocking mode
			Serial.println("sent, waiting for ack");
			uint32_t tempMillis = millis();
			while(millis() - tempMillis <= 1000)
			{
				int packetSize = LoRa.parsePacket();
				
				if (packetSize)
				{
					// received a packet
					// read packet
					while (LoRa.available())
					{
						Serial.println("waiting ack");
						if ((char)LoRa.read() == 6)
						{
							Serial.println("ACK RECEIVED!");
							ackReceived = 1;
							break;
						}
					}
				}
			}
			// addMessage((char *)content.c_str(), 0, RTC.now());
		}
		addMessage((char *)content.c_str(), 0, DateTime((uint32_t)0));
		if (messages.size() > MAX_MESSAGES)
		{
			messages.remove(0);
			saveMessages();
		}
		if (messages.size() > NUM_MESSAGES_ONSCREEN)
		{
			cursor = messages.size() - NUM_MESSAGES_ONSCREEN;
			cameraY = (cursor * 23);
		}
		messageSelect = messages.size() - 1;
		content = "";
		prevContent = "";
		Serial.println("SENT");
		textInput("", -1);
		textPointer = 0;
		buttons.update();
	}
	if (buttons.held(BTN_FUN_4, 120))
	{
		bool optionsMenuExit = 0;
		while(!optionsMenuExit)
		{
			int8_t optionsCode = optionsPopup();
			buttons.update();
			if (optionsCode > -1)
			{
				switch (optionsCode)
				{
				case 0:
				{
					int8_t selection = memesPopup();
					if(selection > 0){
						optionsMenuExit = 1;
					}
					break;
				}
				case 1:
				{
					drawMainScreen();
					display.pushSprite(0,0);
					int8_t smileyCode = smileyPopup();
					Serial.println(smileyCode);
					delay(5);
					if(smileyCode > 0)
					{
						content.concat(":");
						//content += ':';
						if (smileyCode < 10)
							//content += '0';
							content.concat("0");
						//content += smileyCode;
						content.concat(smileyCode);
						//content += ':';
						content.concat(":");
						textPointer += 4;
						optionsMenuExit = 1;
					}
					else
					{
						drawMainScreen();
						display.pushSprite(0,0);
					}
					break;
				}
				case 2:
				{
					drawMainScreen();
					display.pushSprite(0,0);
					detailsPopup(messageSelect);
					drawMainScreen();
					display.pushSprite(0,0);
					break;
				}
				case 3:
				{
					messages.remove(messageSelect);
					saveMessages();
					messageSelect--;
					optionsMenuExit = 1;
					break;
				}
				}
			}
			else
			{
				optionsMenuExit = 1;
			}
			
		}
	}
	//check if received
	int packetSize = LoRa.parsePacket();
	// Serial.println("check if received?");
	if (packetSize)
	{
		Serial.print("Size: ");
		Serial.println(packetSize);

		// read packet
		bool findInString = 0;
		uint8_t counter = 0;
		for (uint32_t i = 0; i < received.length(); i++)
		{
			if (received[i] == '\n')
			{
				counter++;
				if (counter > 3)
				{
					findInString = 1;
					break;
				}
			}
		}
		char buffer[20];
		memset(buffer, 0, sizeof(buffer));
		if (findInString)
			received = "";
		while (LoRa.available())
		{
			char temp = (char)LoRa.read();
			strncat(buffer, &temp, 1);
		}
		// received+=" ";
		Serial.print(LoRa.packetRssi());
		Serial.println("dBm");
		// received+=LoRa.packetRssi();
		// received+="dBm";
		received += buffer;

		received += (char)'\n';
		// wait until the radio is ready to send a packet
		while (LoRa.beginPacket() == 0)
		{
			Serial.print("waiting for radio ... ");
			delay(100);
		}

		Serial.print("Sending ack: ");
		Serial.println(counter);

		// send in async / non-blocking mode
		LoRa.beginPacket();
		LoRa.print(char(6));
		LoRa.endPacket();
		
		addMessage(buffer, 1, RTC.now());
		if (messages.size() > MAX_MESSAGES)
		{
			messages.remove(0);
			saveMessages();
		}
		if (messages.size() > NUM_MESSAGES_ONSCREEN)
		{
			cursor = messages.size() - NUM_MESSAGES_ONSCREEN;
			cameraY = (cursor * 23);
		}
		messageSelect = messages.size() - 1;
	}

	buttons.update();
	if (millis() - refreshMillis > 40)
	{
		refreshMillis = millis();
		display.pushSprite(0, 0);
		isRefreshed = 1;
	}

}