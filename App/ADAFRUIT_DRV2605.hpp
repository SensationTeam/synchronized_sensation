/*
 * ADAFRUIT_DRV2605.h
 *
 *  Created on: Oct 17, 2024
 *      Author: tulip
 */

#ifndef ADAFRUIT_DRV2605_HPP_
#define ADAFRUIT_DRV2605_HPP_


#include <stdio.h>
#include "i2c.h"



#define DRV2605_ADDR 0x5A ///< Device I2C address

#define DRV2605_REG_STATUS 0x00       ///< Status register
#define DRV2605_REG_MODE 0x01         ///< Mode register
#define DRV2605_MODE_INTTRIG 0x00     ///< Internal trigger mode
#define DRV2605_MODE_REALTIME 0x05

#define DRV2605_REG_RTP_INPUT	 0x02    ///< Real-time playback input register
#define DRV2605_REG_LIBRARY 0x03  ///< Waveform library selection register
#define DRV2605_REG_WAVESEQ1 0x04 ///< Waveform sequence register 1
#define DRV2605_REG_WAVESEQ2 0x05 ///< Waveform sequence register 2
#define DRV2605_REG_WAVESEQ3 0x06 ///< Waveform sequence register 3
#define DRV2605_REG_WAVESEQ4 0x07 ///< Waveform sequence register 4
#define DRV2605_REG_WAVESEQ5 0x08 ///< Waveform sequence register 5
#define DRV2605_REG_WAVESEQ6 0x09 ///< Waveform sequence register 6
#define DRV2605_REG_WAVESEQ7 0x0A ///< Waveform sequence register 7
#define DRV2605_REG_WAVESEQ8 0x0B ///< Waveform sequence register 8

#define DRV2605_REG_GO 0x0C         ///< Go register
#define DRV2605_REG_OVERDRIVE 0x0D  ///< Overdrive time offset register
#define DRV2605_REG_SUSTAINPOS 0x0E ///< Sustain time offset, positive register
#define DRV2605_REG_SUSTAINNEG 0x0F ///< Sustain time offset, negative register
#define DRV2605_REG_BREAK 0x10      ///< Brake time offset register
#define DRV2605_REG_AUDIOCTRL 0x11  ///< Audio-to-vibe control register
#define DRV2605_REG_AUDIOLVL 0x12   ///< Audio-to-vibe minimum input level register
#define DRV2605_REG_AUDIOMAX 0x13   ///< Audio-to-vibe maximum input level register
#define DRV2605_REG_AUDIOOUTMIN 0x14 ///< Audio-to-vibe minimum output drive register
#define DRV2605_REG_AUDIOOUTMAX 0x15 ///< Audio-to-vibe maximum output drive register
#define DRV2605_REG_RATEDV 0x16     ///< Rated voltage register
#define DRV2605_REG_CLAMPV 0x17     ///< Overdrive clamp voltage register
#define DRV2605_REG_AUTOCALCOMP 0x18 ///< Auto-calibration compensation result register
#define DRV2605_REG_AUTOCALEMP 0x19  ///< Auto-calibration back-EMF result register
#define DRV2605_REG_FEEDBACK 0x1A   ///< Feedback control register
#define DRV2605_REG_CONTROL1 0x1B   ///< Control1 Register
#define DRV2605_REG_CONTROL2 0x1C   ///< Control2 Register
#define DRV2605_REG_CONTROL3 0x1D   ///< Control3 Register
#define DRV2605_REG_CONTROL4 0x1E   ///< Control4 Register
#define DRV2605_REG_VBAT 0x21       ///< Vbat voltage-monitor register
#define DRV2605_REG_LRARESON 0x22   ///< LRA resonance-period register
#define RTP_INPUT1 0x19
#define RTP_INPUT2 0x33
#define RTP_INPUT3 0x4C
#define RTP_INPUT4 0x66
#define RTP_INPUT5 0x7F
#define RTP_INPUT6 0x99
#define RTP_INPUT7 0xB2
#define RTP_INPUT8 0xCC
#define RTP_INPUT9 0xE5
#define RTP_INPUT10 0xFF


// Additional register definitions as needed...
class Adafruit_DRV2605 {
public:
	Adafruit_DRV2605();
	Adafruit_DRV2605(I2C_HandleTypeDef *i2c_handle1, TIM_HandleTypeDef *timer);
	I2C_HandleTypeDef* getWire();
		void init();                 // Başlatma
		void play();                 // GO register = 1
		void stop();                 // GO register = 0
		void selectEffect(uint8_t effect);  // Efekt seç
		uint8_t readStatus();        // Status oku
		void autoCalibrate();        // Opsiyonel: Auto cal
	    bool begin();

	    void writeRegister8(uint8_t reg, uint8_t val1);
	    uint8_t readRegister8(uint8_t reg);
	    void setWaveform(uint8_t slot, uint8_t w);
	    void getWaveforms(uint8_t* waveforms);
	    void setRealtimeValue(uint8_t rtp);
	    //void setRTP(uint8_t val1, uint8_t val2);
	    //void setDutyCycle(uint8_t dutyCycle1,uint8_t dutyCycle2);
	    void selectLibrary(uint8_t lib);

	    void setPWM(uint32_t value);
	    void go(void);
	    void goWait(void);

	    void setMode(uint8_t mode);

	      // Select ERM (Eccentric Rotating Mass) or LRA (Linear Resonant Actuator)
	      // vibration motor The default is ERM, which is more common
	    void useERM();
	    void useLRA();
	    void Setup_PWM_AnalogMode();

	    // Test print status
	    void ReadStatusRegister();

private:
	    I2C_HandleTypeDef *comport;// I2C handler
	    TIM_HandleTypeDef *timer;  // Timer handler

};

#endif // ADAFRUIT_DRV2605_H
