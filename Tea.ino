
#include <LiquidCrystal.h>

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int sensorPin = A1;
int motorPin = 9;
int sensorValue[10] = {0};
int sensor_value_index = 0;
int state = 0;
long sleep_ms = 0;
long tea_finished_counter = 0;

int SLEEP_CYCLE = 500;
int MOTOR_ON_TIME = 6000;

enum states {
	MEASURE_STATE,
	BREW_STATE,
	MOTOR_ON_STATE,
	MOTOR_OFF_STATE,
	FINISHED_STATE
};

void print_time(long total_seconds) {
	int minutes =  total_seconds / 60;
	int secs = total_seconds % 60;

	lcd.setCursor(0, 1);
	lcd.print(minutes, DEC);
	lcd.print("min ");
	lcd.print(secs, DEC);
	lcd.print("sec ");
	lcd.setCursor(0, 0);
}

int minimum(int *value, int length) {
	int min = value[0];
	for (int i = 0; i < length; i++) {
		if (value[i] < min) {
			min = value[i];
		}
	}
	return min;
}

int maximum(int *value, int length) {
	int max = value[0];
	for (int i = 0; i < length; i++) {
		if (value[i] > max) {
			max = value[i];
		}
	}
	return max;
}

void setup() {
	Serial.begin(9600);
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	// Print a message to the LCD.
	lcd.print("Tea time!");
	pinMode(motorPin, OUTPUT);
	digitalWrite(motorPin, LOW);
}

void loop() {
	if (state == MEASURE_STATE) {
		lcd.setCursor(0, 0);
		lcd.clear();
		lcd.print("Selected time:");

		sensorValue[sensor_value_index % 10] = analogRead(sensorPin);
		int select_value = sensorValue[sensor_value_index % 10];
		sensor_value_index++;

		if (sensor_value_index > 10) {
			sensor_value_index = 0;
			int min = minimum(sensorValue, 10);
			int max = maximum(sensorValue, 10);

			float max_min = (float) ((float)max - (float)min)/((float)max);
			float max_min_seconds = max - min;

			if (max_min < 0.1f || max_min_seconds < 2) {
				// 0 - 1023 = 0 - 6 min == 0 - 6*60*1000 ms = 360000 => sensor_value * 351 (517 since connected to 3v3)
				sleep_ms = 517 * (long)select_value;
				Serial.println(sleep_ms);
				state = BREW_STATE;
			}
		} else {
			print_time(517 * (long) select_value / 1000);
			delay(SLEEP_CYCLE/2);
		}

	} else if (state == BREW_STATE) {
		lcd.setCursor(0, 0);
		lcd.clear();
		lcd.print("Brewing: ");
		print_time(sleep_ms/1000);

		sleep_ms = sleep_ms - SLEEP_CYCLE;
		if (sleep_ms > 0) {
			delay(SLEEP_CYCLE);
		} else {
			state = MOTOR_ON_STATE;
		}
	} else if (state == MOTOR_ON_STATE) {
		lcd.setCursor(0, 0);
		lcd.clear();
		lcd.print("Motor on");

		// Turn on motor
		digitalWrite(motorPin, HIGH);
		delay(MOTOR_ON_TIME);
		state = MOTOR_OFF_STATE;
	} else if (state == MOTOR_OFF_STATE) {
		lcd.setCursor(0, 0);
		lcd.clear();
		lcd.print("Motor off");

		// Turn off motor
		digitalWrite(motorPin, LOW);
		lcd.setCursor(0, 0);
		lcd.clear();
		lcd.print("Tea finished!");
		state = FINISHED_STATE;
	} else if (state == FINISHED_STATE) {
		// Tea finished for seconds:
		lcd.setCursor(0, 0);
		lcd.clear();
		lcd.print("Finished for: ");
		print_time(tea_finished_counter/1000);

		tea_finished_counter += SLEEP_CYCLE;
		delay(SLEEP_CYCLE);
	}
}
