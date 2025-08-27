#include "pattern_player.hpp"
#include "ADAFRUIT_DRV2605.hpp"
#include "tim.h"


#define MAX_DUTY_PERIOD_MS	(100U)

/**
 * @brief Pattern dizisi indeksleri
 * Pattern dizisi, her satırda {motor komutu, süre} şeklinde veri içeriyor.
 * Örneğin, {0xAA, 20} -> 0xAA komutu 20ms boyunca çalıştırılır.
 */
enum {
	PatternValueIndex = 0,   // Komut değeri (motorun çalıştırılma seviyesi)
	PatternDurationIndex = 1 // Bu komutun ne kadar süreceği (ms cinsinden)
};

typedef enum {
	Duty_Mode = 0, Frequency_Mode,
} PWM_Mode_t;

// **Harici Değişkenler** (Başka bir dosyada tanımlanan değişkenler)
extern TIM_HandleTypeDef htim2;   // 1ms zamanlayıcı (Timer)
extern Adafruit_DRV2605 haptic1; // DRV2605 motor sürücü 1
extern Adafruit_DRV2605 haptic2; // DRV2605 motor sürücü 2

// **İki farklı motor için kontrol yapıları**
MotorControl hMotor1; // Motor 1 kontrolcüsü
MotorControl hMotor2; // Motor 2 kontrolcüsü

// **Titreşim (haptic) pattern dizileri** (Motorlara uygulanacak titreşim komutları)
// İlk eleman motorun çalıştırılacağı komut değeri, ikinci eleman ise süresi (ms)

//TODO: Double check the pattern code
int patternA[][2] = { { 0x10, 20 }, { 0x00, 200 } }; // Motor 1 için titreşim deseni
int patternB[][2] = { { 0x10, 20 }, { 0x00, 200 } }; // Motor 2 için titreşim deseni
// Geçici olarak JSON'dan gelen değerler burada tutulur
volatile bool pendingUpdate = false;
volatile int next_duty_motor1 = -1;
volatile int next_duty_motor2 = -1;
volatile int next_period = 100;
volatile bool isMotor1Connected = false;
volatile bool isMotor2Connected = false;
// Motor modunu belirle
PWM_Mode_t PWM_Mode = Duty_Mode;

/***
 * @brief Yardımcı fonksiyonlar (Bu fonksiyonlar sadece bu dosyada kullanılıyor)
 */
static void play_pattern(MotorControl *motor);
void play_pattern_synchronized(MotorControl* motor1, MotorControl* motor2);
/* unused */
__weak void ToggleMode() {
	PWM_Mode = (PWM_Mode == Duty_Mode) ? (Frequency_Mode) : (Duty_Mode);
}

/**
 * @brief Pattern oynatıcıyı başlatmak için zayıf fonksiyon
 * Kullanıcı bu fonksiyonu kendi tanımlayabilir.
 */
void DRV2605_SoftwareReset(MotorControl* motor) {
    printf("[RESET] Putting device to standby...\r\n");
    motor->driver->writeRegister8(0x01, 0x00);  // Standby
    HAL_Delay(10);

    printf("[RESET] Exiting standby...\r\n");
    motor->driver->writeRegister8(0x01, 0x01);  // Out of standby
    HAL_Delay(10);

    printf("[RESET] Setting feedback config...\r\n");
    motor->driver->writeRegister8(0x1D, 0xB6);  // Feedback config (typical value)

    printf("[RESET] Setting mode to RTP...\r\n");
    motor->driver->setMode(DRV2605_MODE_REALTIME); // RTP mode

    HAL_Delay(5);  // Make sure DRV2605 is ready

    // ❗️ ÖNEMLİ: RTP değeri burada gönderilmeyecek.
    // motor->driver->setRealtimeValue(0x00); // Bunu buradan kaldır
}


bool DRV2605_RunAutoCalibration(MotorControl* motor) {
    printf("[%s] Starting auto-calibration...\r\n", motor == &hMotor1 ? "Motor1" : "Motor2");

    motor->driver->writeRegister8(0x01, 0x07); // Mode: Auto-Calibration mode
    HAL_Delay(2);

    motor->driver->writeRegister8(0x0C, 0x01); // GO = 1

    // Wait until GO bit clears
    while (motor->driver->readRegister8(0x0C) & 0x01) {
        HAL_Delay(5);
    }

    uint8_t status = motor->driver->readRegister8(0x0B);
    if (status & 0x08) {
        printf("[%s] Calibration failed! Status=0x%02X\r\n", motor == &hMotor1 ? "Motor1" : "Motor2", status);
        return false;
    }

    uint8_t ratedV = motor->driver->readRegister8(0x16);
    uint8_t overdrive = motor->driver->readRegister8(0x17);
    uint8_t gain = motor->driver->readRegister8(0x18);

    printf("[%s] Calibration successful.\r\n", motor == &hMotor1 ? "Motor1" : "Motor2");
    printf("Rated Voltage: %u, Overdrive Clamp: %u, Feedback Gain: %u\r\n", ratedV, overdrive, gain);

    return true;
}


void InitMotor(MotorControl *motor, Adafruit_DRV2605 *driver, int (*pattern)[2],
               int patternLength, const char *motorName) {
    printf("Initializing %s...\r\n", motorName);

    motor->driver = driver;
    motor->pattern = pattern;
    motor->patternLength = patternLength;
    motor->pair_index = 0;
    motor->millis_counter = 0;
    motor->isDisabled = false;

    bool connected = motor->driver->begin();

    if (motor == &hMotor1) isMotor1Connected = connected;
    if (motor == &hMotor2) isMotor2Connected = connected;

    if (connected) {
    	DRV2605_SoftwareReset(motor);      // <- ekle
    	    motor->driver->setRealtimeValue(0x00);
    	    motor->isDisabled = true;
        printf("%s: DRV2605L initialized successfully.\r\n", motorName);

        motor->driver->setMode(DRV2605_MODE_REALTIME);
        motor->driver->selectLibrary(1);
        motor->driver->useLRA();

        uint8_t fault = motor->driver->readRegister8(0x0B);
        if (fault) {
            printf("%s: Fault Register = 0x%X\r\n", motorName, fault);
        } else {
            printf("%s: No faults detected.\r\n", motorName);
        }

        motor->driver->setRealtimeValue(0x00);
        motor->isDisabled = true;
        printf("%s: Motor set to off.\r\n", motorName);
    } else {
        printf("%s: DRV2605L initialization failed.\r\n", motorName);
    }
}




void PatternPlayer_Init(void) {
	//DRV2605_SoftwareReset(&hMotor1);
	 //   DRV2605_SoftwareReset(&hMotor2);

	printf("Starting PatternPlayer_Init...\r\n");
	InitMotor(&hMotor1, &haptic1, patternA,
			sizeof(patternA) / sizeof(patternA[0]), "Motor 1");
	InitMotor(&hMotor2, &haptic2, patternB,
			sizeof(patternB) / sizeof(patternB[0]), "Motor 2");
	printf("PatternPlayer_Init completed.\r\n");

}

void apply_new_duty(MotorControl *motor, int duty,int period) {
	printf("[DEBUG] Applying new duty cycle: %d%%\r\n", duty);

	int on_time = period * duty / 100;
	int off_time = period - on_time;

	printf("[DEBUG] Calculated on_time: %d ms, off_time: %d ms\r\n", on_time,
			off_time);

	// To avoid motor heat up and uncertainty
	if (off_time < 5) {
		off_time = 5;
		printf("[WARNING] off_time too low, adjusted to: %d ms\r\n", off_time);
	}

	// Ensure motor is enabled if duty is more than 0
	if (on_time > 0) {
		motor->isDisabled = false;
		printf("[INFO] Motor enabled\r\n");
	} else {
		motor->isDisabled = true;
		printf("[INFO] Motor disabled\r\n");
	}

	// Set motor on-time and off-time values
	motor->pattern[0][PatternDurationIndex] = on_time;
	motor->pattern[1][PatternDurationIndex] = off_time;

	printf("[DEBUG] Pattern updated: [0] = %d ms, [1] = %d ms\r\n",
			motor->pattern[0][PatternDurationIndex],
			motor->pattern[1][PatternDurationIndex]);

}

/************************************************************
 * @brief Motor için görev döngüsünü başlatan fonksiyon
 *
 * @param is_motor MOTOR_1 veya MOTOR_2
 * @param duty Motorun çalışma yüzdesi (0-100)
 ************************************************************/
void SetDuty(Motor_t is_motor, int duty, int period) {
    MotorControl *motor = (is_motor == MOTOR_1) ? (&hMotor1) : (&hMotor2);

    if (duty == 0) {
        motor->isDisabled = true;
        motor->driver->setRealtimeValue(0x00);
        return;
    }

    // Duty & Pattern ayarla
    apply_new_duty(motor, duty, period);

            // Pattern baştan başlasın
    motor->pair_index = 0;
    motor->millis_counter = 0;
    motor->isDisabled = false;

    // İlk fazı anında uygula (gecikmesiz başlasın)
       int pwm = motor->pattern[motor->pair_index][PatternValueIndex];
       motor->driver->setRealtimeValue(pwm);

}




/************************************************************
 * @brief Her 1ms'de çağrılan timer geri çağrım fonksiyonu
 *
 * İki motor için ayrı ayrı play_pattern() fonksiyonu çağrılır.
 ************************************************************/
void PatternTimerCallback() {
    // JSON'dan gelen duty update'leri varsa, önce bunları uygula
    if (pendingUpdate) {
        if (next_duty_motor1 != -1) {
            SetDuty(MOTOR_1, next_duty_motor1, next_period);
        }
        if (next_duty_motor2 != -1) {
            SetDuty(MOTOR_2, next_duty_motor2, next_period);
        }

        // Bayrakları sıfırla
        pendingUpdate = false;
        next_duty_motor1 = -1;
        next_duty_motor2 = -1;
    }

    // Her motorun durumu kontrol edilir
    bool motor1_active = !hMotor1.isDisabled;
    bool motor2_active = !hMotor2.isDisabled;
    if (motor1_active==false || motor2_active==false ) return;

    if (motor1_active && motor2_active) {
        play_pattern_synchronized(&hMotor1, &hMotor2);return;
    }
    if (motor1_active) play_pattern(&hMotor1);
    if (motor2_active) play_pattern(&hMotor2);

}


void play_pattern_synchronized(MotorControl* motor1, MotorControl* motor2) {
    // 1. Sayaçları artır
    motor1->millis_counter++;
    motor2->millis_counter++;

    // 2. Ne zaman pattern değiştirilecek?
    bool update1 = (motor1->millis_counter >= motor1->pattern[motor1->pair_index][PatternDurationIndex]);
    bool update2 = (motor2->millis_counter >= motor2->pattern[motor2->pair_index][PatternDurationIndex]);

    // 3. Yeni pattern indexini hesapla (ama henüz uygulama)
    if (update1) {
        motor1->millis_counter = 0;
        motor1->pair_index = (motor1->pair_index + 1) % motor1->patternLength;
    }

    if (update2) {
        motor2->millis_counter = 0;
        motor2->pair_index = (motor2->pair_index + 1) % motor2->patternLength;
    }

    // 4. setRealtimeValue() çağrılarını **eş zamanlı** yap
    int pwm_value1 = motor1->pattern[motor1->pair_index][PatternValueIndex];
    int pwm_value2 = motor2->pattern[motor2->pair_index][PatternValueIndex];

    motor1->driver->setRealtimeValue(pwm_value1);


    //while (HAL_I2C_GetState(motor2->driver->getWire()) != HAL_I2C_STATE_READY);


    motor2->driver->setRealtimeValue(pwm_value2);

}


/**************************************************************
 * @brief Motor için pattern oynatma fonksiyonu
 *
 * Motorun aktif olup olmadığı ve hangi pattern'in çalıştırılacağı kontrol edilir.
 *
 * @param motor Motor kontrol yapısı (hMotor1 veya hMotor2 olabilir)
 *************************************************************/
static void play_pattern(MotorControl *motor) {
    if (motor->millis_counter == 0 && motor->pair_index == 0) {
        // İlk başlangıçta reset uygula (opsiyonel)
        DRV2605_SoftwareReset(motor);
    }

    motor->millis_counter++;
    if (motor->millis_counter >= motor->pattern[motor->pair_index][PatternDurationIndex]) {
        motor->millis_counter = 0;
        motor->pair_index = (motor->pair_index + 1) % motor->patternLength;

        int motor_pwm_value = motor->pattern[motor->pair_index][PatternValueIndex];
        motor->driver->setRealtimeValue(motor_pwm_value);
    }
}
