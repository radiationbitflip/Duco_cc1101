#include <Arduino.h>
#include <SPI.h>

#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CS    5

SPIClass spi(VSPI);

// ---------------- CC1101 STROBES ----------------
#define CC1101_SRES   0x30
#define CC1101_SFSTXON 0x31
#define CC1101_SXOFF   0x32
#define CC1101_SCAL    0x33
#define CC1101_SRX     0x34
#define CC1101_STX     0x35
#define CC1101_SIDLE   0x36
#define CC1101_SWOR    0x38
#define CC1101_SPWD    0x39
#define CC1101_SFRX    0x3A
#define CC1101_SFTX    0x3B

#define CC1101_TXFIFO  0x3F
#define CC1101_PATABLE 0x3E

#define WRITE_BURST    0x40
#define READ_SINGLE    0x80
#define READ_BURST     0xC0

// ---------------- CC1101 REGISTERS ----------------
#define CC1101_IOCFG2    0x00
#define CC1101_IOCFG1    0x01
#define CC1101_IOCFG0    0x02
#define CC1101_FIFOTHR   0x03
#define CC1101_SYNC1     0x04
#define CC1101_SYNC0     0x05
#define CC1101_PKTLEN    0x06
#define CC1101_PKTCTRL1  0x07
#define CC1101_PKTCTRL0  0x08
#define CC1101_ADDR      0x09
#define CC1101_CHANNR    0x0A
#define CC1101_FSCTRL1   0x0B
#define CC1101_FSCTRL0   0x0C
#define CC1101_FREQ2     0x0D
#define CC1101_FREQ1     0x0E
#define CC1101_FREQ0     0x0F
#define CC1101_MDMCFG4   0x10
#define CC1101_MDMCFG3   0x11
#define CC1101_MDMCFG2   0x12
#define CC1101_MDMCFG1   0x13
#define CC1101_MDMCFG0   0x14
#define CC1101_DEVIATN   0x15
#define CC1101_MCSM2     0x16
#define CC1101_MCSM1     0x17
#define CC1101_MCSM0     0x18
#define CC1101_FOCCFG    0x19
#define CC1101_BSCFG     0x1A
#define CC1101_AGCCTRL2  0x1B
#define CC1101_AGCCTRL1  0x1C
#define CC1101_AGCCTRL0  0x1D
#define CC1101_WORCTRL   0x20
#define CC1101_FREND1    0x21
#define CC1101_FREND0    0x22
#define CC1101_FSCAL3    0x23
#define CC1101_FSCAL2    0x24
#define CC1101_FSCAL1    0x25
#define CC1101_FSCAL0    0x26
#define CC1101_FSTEST    0x29
#define CC1101_AGCTEST   0x2B
#define CC1101_TEST2     0x2C
#define CC1101_TEST1     0x2D
#define CC1101_TEST0     0x2E

#define CC1101_VERSION   0x31
#define CC1101_MARCSTATE 0x35
#define CC1101_TXBYTES   0x3A

// ---------------- SPI HELPERS ----------------

void csLow() {
  digitalWrite(PIN_CS, LOW);
}

void csHigh() {
  digitalWrite(PIN_CS, HIGH);
}

void waitMisoLow() {
  uint32_t start = millis();
  while (digitalRead(PIN_MISO)) {
    if (millis() - start > 1000) {
      Serial.println("MISO wait timeout");
      break;
    }
  }
}

void strobe(uint8_t cmd) {
  csLow();
  waitMisoLow();
  spi.transfer(cmd);
  csHigh();
}

void writeReg(uint8_t addr, uint8_t value) {
  csLow();
  waitMisoLow();
  spi.transfer(addr);
  spi.transfer(value);
  csHigh();
}

uint8_t readStatus(uint8_t addr) {
  csLow();
  waitMisoLow();
  spi.transfer(addr | READ_BURST);
  uint8_t val = spi.transfer(0x00);
  csHigh();
  return val;
}

void writeBurst(uint8_t addr, const uint8_t *data, uint8_t len) {
  csLow();
  waitMisoLow();
  spi.transfer(addr | WRITE_BURST);
  for (uint8_t i = 0; i < len; i++) {
    spi.transfer(data[i]);
  }
  csHigh();
}

// ---------------- RESET ----------------

void cc1101Reset() {
  csHigh();
  delay(5);

  csLow();
  delay(1);

  csHigh();
  delay(1);

  csLow();
  waitMisoLow();

  spi.transfer(CC1101_SRES);

  waitMisoLow();
  csHigh();

  delay(10);
}

// ---------------- DUCO RF CONFIG ----------------

void configureDucoTx() {
  strobe(CC1101_SIDLE);

  // GDO pins not important for this simple TX test
  writeReg(CC1101_IOCFG2, 0x2E);
  writeReg(CC1101_IOCFG1, 0x2E);
  writeReg(CC1101_IOCFG0, 0x2E);

  writeReg(CC1101_FIFOTHR, 0x47);

  // DUCO sync word
  writeReg(CC1101_SYNC1, 0xD3);
  writeReg(CC1101_SYNC0, 0x91);

  // Max packet length
  writeReg(CC1101_PKTLEN, 0x20);

  // Variable length, CRC enabled
  writeReg(CC1101_PKTCTRL1, 0x04);
  writeReg(CC1101_PKTCTRL0, 0x05);

  writeReg(CC1101_ADDR, 0x00);
  writeReg(CC1101_CHANNR, 0x00);

  writeReg(CC1101_FSCTRL1, 0x06);
  writeReg(CC1101_FSCTRL0, 0x00);

  // 868.326 MHz
  writeReg(CC1101_FREQ2, 0x21);
  writeReg(CC1101_FREQ1, 0x65);
  writeReg(CC1101_FREQ0, 0xAD);

  // 38.3835 kbaud, 101 kHz RX BW, GFSK
  writeReg(CC1101_MDMCFG4, 0xCA);
  writeReg(CC1101_MDMCFG3, 0x83);
  writeReg(CC1101_MDMCFG2, 0x13);
  writeReg(CC1101_MDMCFG1, 0x22);
  writeReg(CC1101_MDMCFG0, 0xF8);

  // Deviation
  writeReg(CC1101_DEVIATN, 0x35);

  writeReg(CC1101_MCSM2, 0x07);
  writeReg(CC1101_MCSM1, 0x2F);
  writeReg(CC1101_MCSM0, 0x08);

  writeReg(CC1101_FOCCFG, 0x16);
  writeReg(CC1101_BSCFG, 0x6C);

  writeReg(CC1101_AGCCTRL2, 0x43);
  writeReg(CC1101_AGCCTRL1, 0x40);
  writeReg(CC1101_AGCCTRL0, 0x91);

  writeReg(CC1101_WORCTRL, 0xFB);

  writeReg(CC1101_FREND1, 0x56);
  writeReg(CC1101_FREND0, 0x10);

  writeReg(CC1101_FSCAL3, 0xE9);
  writeReg(CC1101_FSCAL2, 0x2A);
  writeReg(CC1101_FSCAL1, 0x00);
  writeReg(CC1101_FSCAL0, 0x1F);

  writeReg(CC1101_FSTEST, 0x59);
  writeReg(CC1101_AGCTEST, 0x3F);

  writeReg(CC1101_TEST2, 0x81);
  writeReg(CC1101_TEST1, 0x35);
  writeReg(CC1101_TEST0, 0x09);

  // PA table. 0xC1 is used in the DUCO file as default TX power.
  const uint8_t paTable[8] = {
    0xC1, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  writeBurst(CC1101_PATABLE, paTable, 8);

  // Calibrate
  strobe(CC1101_SIDLE);
  strobe(CC1101_SCAL);
  delay(10);

  strobe(CC1101_SIDLE);
  strobe(CC1101_SFTX);
}

// ---------------- CAPTURED PACKETS ----------------
// Last 2 RX status bytes are removed.
// First byte remains because CC1101 variable packet mode needs it.

// Replace with your captured DUCO device bytes


uint8_t HIGH_CMD[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t LOW_CMD[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t AUTO_CMD[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ---------------- TX ----------------

void sendPacket(const uint8_t *packet, uint8_t len) {
  Serial.print("TX: ");

  for (uint8_t i = 0; i < len; i++) {
    if (packet[i] < 16) Serial.print("0");
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }

  Serial.println();

  strobe(CC1101_SIDLE);
  strobe(CC1101_SFTX);

  writeBurst(CC1101_TXFIFO, packet, len);

  strobe(CC1101_STX);

  delay(80);

  strobe(CC1101_SIDLE);
  strobe(CC1101_SFTX);
}

void sendRepeated(const uint8_t *packet, uint8_t len) {
  for (int i = 0; i < 3; i++) {
    sendPacket(packet, len);
    delay(120);
  }
}

// ---------------- SETUP / LOOP ----------------

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);

  pinMode(PIN_MISO, INPUT);

  spi.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  Serial.println();
  Serial.println("DUCO TX START");

  cc1101Reset();

  uint8_t version = readStatus(CC1101_VERSION);
  Serial.print("VERSION: 0x");
  Serial.println(version, HEX);

  configureDucoTx();

  Serial.println("Commands:");
  Serial.println("h = HIGH");
  Serial.println("l = LOW");
}

void loop() {
  if (!Serial.available()) return;

  char c = Serial.read();

  if (c == 'h' || c == 'H') {
    Serial.println("Sending HIGH");
    sendRepeated(HIGH_CMD, sizeof(HIGH_CMD));
  }

  if (c == 'l' || c == 'L') {
    Serial.println("Sending LOW");
    sendRepeated(LOW_CMD, sizeof(LOW_CMD));
  }

  if (c == 'a' || c == 'A') {
    Serial.println("Sending AUTO");
    sendRepeated(AUTO_CMD, sizeof(AUTO_CMD));
  }
}