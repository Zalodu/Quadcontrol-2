/*
 * The primary functions for the project, including the primary loops called
 * by the main function.
 *
 * For http://github.com/Zalodu/Quadcontrol-2
 * Author: Peter Kjellén (Zalodu), Jesper Larsson (mrlarssonjr)
 * Date: 15/12/16
 */

#include <pic32mx.h>
#include "input.h"
#include "esc.h"
#include "vector.h"
#include "mpu9150ExtendedInterface.h"
#include "i2caddresses.h"
#include "controller.h"
#include "controller_constants.h"

#define START_MODE_SWITCH SW1

char textstring[] = "text, more text, and even more text!";

ControllerState inclinationController = {
		.proportionalGain = INCLINATION_PROPORTIONAL_GAIN,
		.integralGain = INCLINATION_INTEGRAL_GAIN,
		.derivativeGain = INCLINATION_DERIVATIVE_GAIN,
		.derivativeDerivativeGain = INCLINATION_DERIVATIVE_DERIVATIVE_GAIN,

		.integratorMax = INCLINATION_INTEGRATOR_MAX_CONST/INCLINATION_INTEGRAL_GAIN,
		.integratorMin = INCLINATION_INTEGRATOR_MIN_CONST/INCLINATION_INTEGRAL_GAIN,

		.integratorState = 0.0,

		.derivatorState = 0.0
	};

/* Interrupt Service Routine */
char increasing;
void user_isr( void )
{
	// Reset interrupt flag
	IFSCLR(0) = 0x100;
	time_tick();
	if(!mpu9150ExtendedInterface_notConnected()){
		mpu9150ExtendedInterface_tick();
	}

	if(time_getElapsedTicks() % 50 == 0) {
		/* Increment value for leds */
		if (PORTE == 0x0) {
			PORTE = 0x7;
			increasing = 1;
		}
		else if (PORTE == 0x7) {
			increasing = 1;
			PORTE = PORTE << 1;
		}
		else if (PORTE == 0xE0) {
			increasing = 0;
			PORTE = PORTE >> 1;
		}
		else if (increasing) {
			PORTE = PORTE << 1;
		}
		else if (!increasing) {
			PORTE = PORTE >> 1;
		}
	}
}

void quad_enableInterrupts(void) {
	asm volatile("ei");
}

void quad_init(void) {
	quad_enableInterrupts();

	// Init input
	display_string(0, "init input");
	display_update();
	input_init();

	//Init sensors
	display_string(0, "init sensors");
	display_update();
	if(!mpu9150ExtendedInterface_notConnected()) {
		mpu9150ExtendedInterface_init();
	}

	// Init ESCs depending on switch config
	display_string(0, "init ESCs");
	display_update();
	switch (input_getInput(START_MODE_SWITCH)) {
		case 0: {
			display_string(0, "ESC normal start");
			display_update();
			esc_init(NORMAL_START);
			break;
		}
		case 1: {
			display_string(0, "ESC tRange start");
			display_update();
			esc_init(SET_THROTTLE_RANGE_START);
			break;
		}
	}
	display_string(0, "init done");
	display_update();
}

/*
 * Volatile functions used for testing purposes
 */
double x = 0;
void quad_debug (void) {
	if(mpu9150ExtendedInterface_notConnected()) {
		display_string(0, "Not connected");
		mpu9150ExtendedInterface_init();
	} else {
		display_string(0, itoaconv((int) (mpu9150ExtendedInterface_getInclinationDerivative().x + 0.5)));
		display_string(1, itoaconv((int) (mpu9150ExtendedInterface_getInclinationDerivative().y + 0.5)));
		display_string(2, itoaconv((int) (mpu9150ExtendedInterface_getInclinationDerivative().z + 0.5)));
	}

	display_update();

	time_blockFor(100);

	x += 0.005;
	if(x > 1.0) {
		x = 0.0;
	}

	display_string(3, itoaconv((int) (x * 10000)));

	esc_setSpeed(MOTOR_FRONT, x);
	esc_setSpeed(MOTOR_REAR, x);
	esc_setSpeed(MOTOR_LEFT, x);
	esc_setSpeed(MOTOR_RIGHT, x);
}

unsigned int lastTime = 0;
void quad_loop(void) {
	// Read values from MPU, send it through the controller and send new PWM
	// values to ESCs

	/* Check if time has updated, skip an iteration/do nothing if it hasn't */
	unsigned int currentTime = time_getElapsedTicks();
	if (currentTime > lastTime) {
		lastTime = currentTime;

		Vector3 inclination = mpu9150ExtendedInterface_getInclination();
		Vector3 inclinationTarget = {.x = 0.0, .y = 0.0, .z = 0.0};

		Vector3 result = calculateStrategy(inclinationController, inclinationTarget, inclination);

		double throttle = 0.1;
/*
 		//Full version:
		double frontSpeed = throttle + result.x - result.z;
		double rearSpeed 	= throttle - result.x - result.z;

		double leftSpeed 	= throttle - result.y + result.z;
		double rightSpeed = throttle + result.y + result.z;
*/
		//Reduced version:
		double frontSpeed = throttle + result.x;
		double rearSpeed 	= throttle - result.x;

		display_string(0, itoaconv((int) (mpu9150ExtendedInterface_getInclination().x + 0.5)));
		display_string(1, itoaconv((int) ((result.x * 100) + 0.5)));
		display_update();

		esc_setSpeed(MOTOR_FRONT, frontSpeed);
		esc_setSpeed(MOTOR_REAR, rearSpeed);
/*
		esc_setSpeed(MOTOR_LEFT, leftSpeed);
		esc_setSpeed(MOTOR_RIGHT, rightSpeed);
*/
	}
}
