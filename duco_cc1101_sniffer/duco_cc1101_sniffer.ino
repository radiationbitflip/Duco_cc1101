#include <Arduino.h>
#include <SPI.h>

// ---------------- PINS ----------------
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CS    5
#define PIN_GDO0  4

SPIClass spi(VSPI);

// ---------------- CC1101 ----------------
#define WRITE_BURST 0x40
#define READ_SINGLE 0x80
#define READ_BURST  0xC0

#define CC1101_IOCFG2    0x00
#define CC1101_IOCFG0    0x02
#define CC1101_FIFOTHR   0x03
#define CC1101_SYNC1     0x04
#define CC1101_SYNC0     0x05
#define CC1101_PKTLEN    0x06
#define CC1101_PKTCTRL1  0x07
#define CC1101_PKTCTRL0  0x08
#define CC1101_FSCTRL1   0x0B
#define CC1101_FREQ2     0x0D
#define CC1101_FREQ1     0x0E
#define CC1101_FREQ0     0x0F
#define CC1101_MDMCFG4   0x10
#define CC1101_MDMCFG3   0x11
#define CC1101_MDMCFG2   0x12
#define CC1101_DEVIATN   0x15
#define CC1101_MCSM0     0x18
#define CC1101_FOCCFG    0x19
#define CC1101_BSCFG     0x1A
#define CC1101_AGCCTRL2  0x1B
#define CC1101_AGCCTRL1  0x1C
#define CC1101_AGCCTRL0  0x1D
#define CC1101_FREND1    0x21
#define CC1101_FREND0    0x22
#define CC1101_FSCAL3    0x23
#define CC1101_FSCAL2    0x24
#define CC1101_FSCAL1    0x25
#define CC1101_FSCAL0    0x26
#define CC1101_TEST2     0x2C
#define CC1101_TEST1     0x2D
#define CC1101_TEST0     0x2E

#define CC1101_RXBYTES   0x3B
#define CC1101_RXFIFO    0x3F
#define CC1101_VERSION   0x31

#define CC1101_SRES      0x30
#define CC1101_SRX       0x34
#define CC1101_SIDLE     0x36
#define CC1101_SFRX      0x3A

// ---------------- SPI Helpers ----------------

void selectChip() {
  digitalWrite(PIN_CS, LOW);
}

void deselectChip() {
  digitalWrite(PIN_CS, HIGH);
}

void writeReg(uint8_t addr, uint8_t value) {
  selectChip();
  spi.transfer(addr);
  spi.transfer(value);
  deselectChip();
}

uint8_t readReg(uint8_t addr) {
  selectChip();
  spi.transfer(addr | READ_SINGLE);
  uint8_t val = spi.transfer(0);
  deselectChip();
  return val;
}

uint8_t readStatus(uint8_t addr) {
  selectChip();
  spi.transfer(addr | READ_BURST);
  uint8_t val = spi.transfer(0);
  deselectChip();
  return val;
}

void strobe(uint8_t cmd) {
  selectChip();
  spi.transfer(cmd);
  deselectChip();
}

void burstRead(uint8_t addr, uint8_t* buf, uint8_t len) {
  selectChip();
  spi.transfer(addr | READ_BURST);

  for (int i = 0; i < len; i++) {
    buf[i] = spi.transfer(0);
  }

  deselectChip();
}

// ---------------- RESET ----------------

void resetCC1101() {

  deselectChip();
  delay(5);

  selectChip();
  delay(5);

  deselectChip();
  delay(5);

  selectChip();

  while (digitalRead(PIN_MISO));

  spi.transfer(CC1101_SRES);

  while (digitalRead(PIN_MISO));

  deselectChip();
}

// ---------------- CONFIG ----------------

void configureCC1101() {

  writeReg(CC1101_IOCFG2, 0x07);

  writeReg(CC1101_IOCFG0, 0x2E);

  writeReg(CC1101_FIFOTHR, 0x47);

  writeReg(CC1101_SYNC1, 0xD3);
  writeReg(CC1101_SYNC0, 0x91);

  writeReg(CC1101_PKTLEN, 0x20);

  writeReg(CC1101_PKTCTRL1, 0x04);
  writeReg(CC1101_PKTCTRL0, 0x05);

  writeReg(CC1101_FSCTRL1, 0x06);

  // 868.326 MHz
  writeReg(CC1101_FREQ2, 0x21);
  writeReg(CC1101_FREQ1, 0x65);
  writeReg(CC1101_FREQ0, 0xAD);

  // 38.4 kbaud GFSK
  writeReg(CC1101_MDMCFG4, 0xCA);
  writeReg(CC1101_MDMCFG3, 0x83);
  writeReg(CC1101_MDMCFG2, 0x13);

  writeReg(CC1101_DEVIATN, 0x35);

  writeReg(CC1101_MCSM0, 0x18);

  writeReg(CC1101_FOCCFG, 0x16);
  writeReg(CC1101_BSCFG, 0x6C);

  writeReg(CC1101_AGCCTRL2, 0x43);
  writeReg(CC1101_AGCCTRL1, 0x40);
  writeReg(CC1101_AGCCTRL0, 0x91);

  writeReg(CC1101_FREND1, 0x56);
  writeReg(CC1101_FREND0, 0x10);

  writeReg(CC1101_FSCAL3, 0xE9);
  writeReg(CC1101_FSCAL2, 0x2A);
  writeReg(CC1101_FSCAL1, 0x00);
  writeReg(CC1101_FSCAL0, 0x1F);

  writeReg(CC1101_TEST2, 0x81);
  writeReg(CC1101_TEST1, 0x35);
  writeReg(CC1101_TEST0, 0x09);

  strobe(CC1101_SIDLE);
  strobe(CC1101_SFRX);
  strobe(CC1101_SRX);
}

// ---------------- SETUP ----------------

void setup() {

  Serial.begin(115200);

  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);

  pinMode(PIN_GDO0, INPUT);

  spi.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  delay(100);

  Serial.println("DUCO SIMPLE RX");

  resetCC1101();

  uint8_t version = readStatus(CC1101_VERSION);

  Serial.print("VERSION: 0x");
  Serial.println(version, HEX);

  configureCC1101();

  Serial.println("Listening...");
}

void printPacket(uint8_t *buf, int len) {

  Serial.print(millis());
  Serial.print(" ms  ");

  Serial.print("LEN=");
  Serial.print(len);
  Serial.print(" : ");

  for (int i = 0; i < len; i++) {

    if (buf[i] < 16)
      Serial.print("0");

    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }

  Serial.println();
}

void loop() {

  uint8_t bytes = readStatus(CC1101_RXBYTES) & 0x7F;

  // overflow check
  if (bytes & 0x80) {

    Serial.println("FIFO OVERFLOW");

    strobe(CC1101_SIDLE);
    strobe(CC1101_SFRX);
    strobe(CC1101_SRX);

    delay(10);
    return;
  }

  if (bytes > 0 && bytes < 64) {

    uint8_t buf[64];

    // read packet
    burstRead(CC1101_RXFIFO, buf, bytes);

    printPacket(buf, bytes);

    // IMPORTANT:
    // fully reset RX state machine

    strobe(CC1101_SIDLE);
    strobe(CC1101_SFRX);
    strobe(CC1101_SRX);
  }

  delay(20);
}