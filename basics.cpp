#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

void setup() {
	Serial.begin(115200);
	LoRa.setPins(22, 16, 17);
	
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
	if (!LoRa.begin(433E6)) {
		Serial.println("Starting LoRa failed!");
		while (1)
			yield();
	}
	// LoRa.setTxPower(20);
	// LoRa.setSpreadingFactor(7);
	// LoRa.setSignalBandwidth(7.8E3);
	// LoRa.setCodingRate4(8);
	LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
	LoRa.setSpreadingFactor(12);
	LoRa.setSignalBandwidth(7.8E3);
	LoRa.setCodingRate4(8);
	// LoRa.setTxPower(20);
	// LoRa.setSpreadingFactor(7);
	// LoRa.setSignalBandwidth(250E3);
	// LoRa.setCodingRate4(5);
	LoRa.enableCrc();

	if (!LoRa.begin(433E6)) {
		Serial.println("Starting LoRa failed!");
		while (1)
			yield();
	}
}

void loop() {
	// try to parse packet
	int packetSize = LoRa.parsePacket();
	if (packetSize) {
		// received a packet
		Serial.print("Received packet '");

		// read packet
		while (LoRa.available()) {
		Serial.print((char)LoRa.read());
		}

		// print RSSI of packet
		Serial.print("' with RSSI ");
		Serial.println(LoRa.packetRssi());
	}
}
// #include <Arduino.h>
// #include <SPI.h>
// #include <LoRa.h>

// void setup() {
// 	Serial.begin(115200);
// 	LoRa.setPins(22, 16, 17);
	
// 	/*
// 	case 0: return 7.8E3;
//     case 1: return 10.4E3;
//     case 2: return 15.6E3;
//     case 3: return 20.8E3;
//     case 4: return 31.25E3;
//     case 5: return 41.7E3;
//     case 6: return 62.5E3;
//     case 7: return 125E3;
//     case 8: return 250E3;
//     case 9: return 500E3;
// 	*/

// 	// LoRa.setPins(5, -1, 4);
// 	if (!LoRa.begin(433E6)) {
// 		Serial.println("Starting LoRa failed!");
// 		while (1)
// 			yield();
// 	}
// 	// LoRa.setTxPower(20);
// 	// LoRa.setSpreadingFactor(7);
// 	// LoRa.setSignalBandwidth(7.8E3);
// 	// LoRa.setCodingRate4(8);
// 	LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
// 	LoRa.setSpreadingFactor(12);
// 	LoRa.setSignalBandwidth(7.8E3);
// 	LoRa.setCodingRate4(8);
// 	// LoRa.setTxPower(20);
// 	// LoRa.setSpreadingFactor(7);
// 	// LoRa.setSignalBandwidth(250E3);
// 	// LoRa.setCodingRate4(5);
// 	LoRa.enableCrc();

// 	if (!LoRa.begin(433E6)) {
// 		Serial.println("Starting LoRa failed!");
// 		while (1)
// 			yield();
// 	}
// }
// uint16_t counter = 0;
// void loop() {
// 	// wait until the radio is ready to send a packet
// 	while (LoRa.beginPacket() == 0) {
// 		Serial.print("waiting for radio ... ");
// 		delay(100);
// 	}

// 	Serial.print("Sending packet non-blocking: ");
// 	Serial.println(counter);

// 	// send in async / non-blocking mode
// 	LoRa.beginPacket();
// 	LoRa.print(char(6));
// 	LoRa.print(counter);
// 	LoRa.endPacket(true); // true = async / non-blocking mode

// 	counter++;
// 	delay(2000);
// }

