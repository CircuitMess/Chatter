#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
Serial.begin(115200);
	while (!Serial);

	Serial.println("LoRa Sender non-blocking");
	LoRa.setTxPower(20);
	LoRa.setSpreadingFactor(7);
	LoRa.setSignalBandwidth(250E3);
	LoRa.setCodingRate4(5);
	// LoRa.enableCrc();
	if (!LoRa.begin(433E6)){
		Serial.println("Starting LoRa failed!");
		while (1);
	}
}

void loop() {
	// wait until the radio is ready to send a packet
	while (LoRa.beginPacket() == 0) {
		Serial.print("waiting for radio ... ");
		delay(100);
	}

	Serial.print("Sending packet non-blocking: ");
	Serial.println(counter);

	// send in async / non-blocking mode
	LoRa.beginPacket();
	LoRa.print("hello ");
	LoRa.print(counter);
	LoRa.endPacket(true); // true = async / non-blocking mode

	counter++;
}
