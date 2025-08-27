/*
 * ADAFRUIT_DRV2605.cpp
 *
 *  Created on: Oct 17, 2024
 *      Author: tulip
 */

#include <ADAFRUIT_DRV2605.hpp>


/*Type definitions*/

// DRV2605 Status Register Yapısı
typedef union {
    uint8_t byte;  // Tam status byte
    struct {
        uint8_t OC_DETECT   : 1;  // Bit 0 - Overcurrent detection
        uint8_t OVER_TEMP   : 1;  // Bit 1 - Overtemperature
        uint8_t FB_STS      : 1;  // Bit 2 - Feedback status
        uint8_t DIAG_RESULT : 1;  // Bit 3 - Diagnostic result
        uint8_t Reserved    : 1;  // Bit 4 - Kullanılmıyor
        uint8_t DEVICE_ID   : 3;  // Bit 5-7 - Device identifier
    } bits;
} DRV2605_StatusReg;

/**************************************************************************/
/*!
 @brief  Instantiates a new DRV2605 class.
 @param i2c_handle Pointer to the I2C handle
 */
/**************************************************************************/

Adafruit_DRV2605::Adafruit_DRV2605(I2C_HandleTypeDef *i2c_handle1,
		TIM_HandleTypeDef *tm) {

	comport = i2c_handle1; // Store the pointer to the I2C handle
	this->timer = tm;
}




/**************************************************************************/
/*!
 @brief  Setup the DRV2605 in RTP mode for LRA motors
 @return true if initialized correctly
 */
/**************************************************************************/
bool Adafruit_DRV2605::begin() {
    // Check if the device is connected
    if (HAL_I2C_IsDeviceReady(comport, DRV2605_ADDR << 1, 1, HAL_MAX_DELAY) != HAL_OK) {
        return false; // Device not found
    }

    // Initialize in RTP mode for LRA
    return true;

}

/*=====================
 * DRV init function for RTP Mode (LRA Motor)
 * ===================*/
void Adafruit_DRV2605::init() {
    writeRegister8(DRV2605_REG_MODE, DRV2605_MODE_INTTRIG);         // Çıkış modu
    writeRegister8(DRV2605_REG_RTP_INPUT, 0x00);
    writeRegister8(DRV2605_REG_LIBRARY, 0x06);      // LRA Library
    writeRegister8(DRV2605_REG_WAVESEQ1, 1);        // Effect 1
    writeRegister8(DRV2605_REG_GO, 0);              // GO reset
}

void Adafruit_DRV2605::play() {
    writeRegister8(DRV2605_REG_GO, 1);
}

void Adafruit_DRV2605::stop() {
    writeRegister8(DRV2605_REG_GO, 0);
}

void Adafruit_DRV2605::selectEffect(uint8_t effect) {
    writeRegister8(DRV2605_REG_WAVESEQ1, effect);
}

uint8_t Adafruit_DRV2605::readStatus() {
    return readRegister8(DRV2605_REG_STATUS);
}

void Adafruit_DRV2605::autoCalibrate() {
    writeRegister8(DRV2605_REG_MODE, 0x07); // Auto-calibration mode
    writeRegister8(DRV2605_REG_GO, 1);
    HAL_Delay(300);
    uint8_t status = readRegister8(DRV2605_REG_STATUS);
    if (status & 0x08)
        printf("✅ Auto-calibration success\r\n");
    else
        printf("❌ Auto-calibration failed\r\n");
}



void Adafruit_DRV2605::ReadStatusRegister() {
    DRV2605_StatusReg status;

    // Read status register using readRegister8 function
    status.byte = readRegister8(DRV2605_REG_STATUS);

    // Print the full status byte
    printf("Status Byte: 0x%02X\r\n", status.byte);
    printf("DEVICE_ID: %d\r\n", status.bits.DEVICE_ID);
    printf("DIAG_RESULT: %d\r\n", status.bits.DIAG_RESULT);
    printf("FB_STS: %d\r\n", status.bits.FB_STS);
    printf("OVER_TEMP: %d\r\n", status.bits.OVER_TEMP);
    printf("OC_DETECT: %d\r\n", status.bits.OC_DETECT);

    // Error handling based on status flags
    if (status.bits.OC_DETECT) {
        printf("⚠️ Overcurrent detected!\r\n");
    }
    if (status.bits.OVER_TEMP) {
        printf("🔥 Overtemperature detected!\r\n");
    }
}
/**************************************************************************/
/*!
 @brief Select the haptic waveform to use.
 @param slot The waveform slot to set, from 0 to 7
 @param w The waveform sequence value
 */
/**************************************************************************/
void Adafruit_DRV2605::setWaveform(uint8_t slot, uint8_t w) {
	writeRegister8(DRV2605_REG_WAVESEQ1 + slot, w);
}

/**
 *
 * */
void Adafruit_DRV2605::Setup_PWM_AnalogMode(void) {
	// mode register 0x03
	// control reg
}

/***/
void Adafruit_DRV2605::getWaveforms(uint8_t *waveforms) {
	// Loop through each waveform slot (0 to 7) and read the value
	for (int slot = 0; slot < 8; slot++) {
		waveforms[slot] = readRegister8(DRV2605_REG_WAVESEQ1 + slot);
	}
}
void Adafruit_DRV2605::setRealtimeValue(uint8_t rtp) {
	writeRegister8(DRV2605_REG_RTP_INPUT, rtp);
}

void Adafruit_DRV2605::setPWM(uint32_t value) {
	//__HAL_TIM_SET_COMPARE(this->timer, TIM_CHANNEL_1, value); // Set PWM duty cycle

}
void Adafruit_DRV2605::selectLibrary(uint8_t lib) {
	writeRegister8(DRV2605_REG_LIBRARY, lib);
}

/**************************************************************************/
/*!
 @brief Start playback of the waveforms (start moving!).
 */
/**************************************************************************/
void Adafruit_DRV2605::go() {
	writeRegister8(DRV2605_REG_GO, 1);
}

void Adafruit_DRV2605::goWait(void) {
	// Send the Go command
	go();

	// Poll the status register to wait for completion
	while (readRegister8(DRV2605_REG_GO) & 0x01) {
		HAL_Delay(10);  // Small delay to avoid excessive polling
	}
}

/**************************************************************************/
/*!


 @brief Set the device mode.
 @param mode Mode value
 */
/**************************************************************************/
void Adafruit_DRV2605::setMode(uint8_t mode) {
	writeRegister8(DRV2605_REG_MODE, mode);
}

/**************************************************************************/
/*!
 @brief Read an 8-bit register.
 @param reg The register to read.
 @return 8-bit value of the register.
 */
/**************************************************************************/
//uint8_t Adafruit_DRV2605::readRegister8(uint8_t reg) {
//	uint8_t val1;
//	HAL_I2C_Mem_Write(comport, DRV2605_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT,
//			&val1, 1, HAL_MAX_DELAY);
//
//	return val1;
//}

uint8_t Adafruit_DRV2605::readRegister8(uint8_t reg) {
	int devAddr = DRV2605_ADDR << 1;  // ✅ 0xB4 olacak şekilde düzelt
  // 8-bit I2C adresi
    uint8_t val = 0;  // Okunan veri buraya gelecek

    // Register adresini cihaza gönder (yazma işlemi)
    if (HAL_I2C_Master_Transmit(comport, devAddr, &reg, 1, HAL_MAX_DELAY) != HAL_OK) {
        printf("I2C Transmit Error\r\n");
        return 0xFF;  // Hata kodu
    }

    // Register değerini oku
    if (HAL_I2C_Master_Receive(comport, devAddr, &val, 1, HAL_MAX_DELAY) != HAL_OK) {
        printf("I2C Receive Error\r\n");
        return 0xFF;  // Hata kodu
    }

    return val;  // Okunan değeri döndür
}

/**************************************************************************/
/*!
 @brief Write an 8-bit register.
 @param reg The register to write.
 @param val The value to write.
 */
/**************************************************************************/
//void Adafruit_DRV2605::writeRegister8(uint8_t reg, uint8_t val1) {
//
//	HAL_I2C_Mem_Write(comport, DRV2605_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT,
//			&val1, 1, HAL_MAX_DELAY);
//}

void Adafruit_DRV2605::writeRegister8(uint8_t reg, uint8_t val1) {
	int devAddr = DRV2605_ADDR << 1;  // 7-bit adresi 8-bit formata çevir
    HAL_I2C_Mem_Write(comport, devAddr, reg, I2C_MEMADD_SIZE_8BIT, &val1, 1, HAL_MAX_DELAY);
}

/**************************************************************************/
/*!
 @brief Use ERM mode.
 */
/**************************************************************************/
void Adafruit_DRV2605::useERM() {
	writeRegister8(DRV2605_REG_FEEDBACK,
			readRegister8(DRV2605_REG_FEEDBACK) & 0x7F);
}

/**************************************************************************/
/*!
 @brief Use LRA mode.
 */
/**************************************************************************/
void Adafruit_DRV2605::useLRA() {
	writeRegister8(DRV2605_REG_FEEDBACK,
			readRegister8(DRV2605_REG_FEEDBACK) | 0x80);
}
I2C_HandleTypeDef* Adafruit_DRV2605::getWire() {
    return comport;
}
