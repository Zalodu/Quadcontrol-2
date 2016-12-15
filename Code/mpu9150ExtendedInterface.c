/*
 * An extended interface for more specific use of the MPU9150.
 *
 * For http://github.com/Zalodu/Quadcontrol-2
 * Author: Jesper Larsson (mrlarssonjr)
 * Date: 15/12/16
 */

#include "vector.h"
#include "time.h"
#include "mpu9150interface.h"

typedef struct {
	double time;
	Vector3 value;
} Measurement;

Measurement latestGyro;
Measurement latestAccel;

Measurement secondLatestGyro;
Measurement secondLatestAccel;

unsigned int numberOfMeasurmentsDone = 0;

Vector3 inclination;
Vector3 speed;

Vector3 angularRotationOffset;
Vector3 accelerationOffset;

void mpu9150ExtendedInterface_setOffset() {
	if(numberOfMeasurmentsDone >= 2) {
		angularRotationOffset = vector_scalarProduct(1.0/2.0, vector_add(latestGyro.value, secondLatestGyro.value));
	}
}

void mpu9150ExtendedInterface_resetInclination() {
	inclination.x = 0.0;
	inclination.y = 0.0;
	inclination.z = 0.0;
}

void mpu9150ExtendedInterface_reset() {
	mpu9150ExtendedInterface_setOffset();
	mpu9150ExtendedInterface_resetInclination();
}

/*
 * Checks if the MPU9150 is properly connected and is able to communicate
 *
 * Returns 0 if it works as expected, -1 if not
 */
 int mpu9150ExtendedInterface_notConnected (void) {
	 return mpu9150interface_notConnected();
 }

/*
 * Initialize the MPU interface.
 */
void mpu9150ExtendedInterface_init(void) {
	mpu9150interface_setup();
	mpu9150interface_awaken();
}

/*
 * Perform a measurment of the MPU.
 */
void mpu9150ExtendedInterface_doMeasurement(void) {
	//Declaration
	double time;

	Vector3 gyroValues;
	Vector3 accelValues;

	double gyroArray[3];
	double accelArray[3];

	//Measuring
	mpu9150interface_getGyroValues(gyroArray);
	mpu9150interface_getAccelValues(accelArray);
	time = time_getElapsedTicks() * time_getLengthOfTick();

	//Ordering the data neatly
	accelValues.x = accelArray[0];
	accelValues.y = accelArray[1];
	accelValues.z = accelArray[2];

	gyroValues.x = gyroArray[0];
	gyroValues.y = gyroArray[1];
	gyroValues.z = gyroArray[2];

	Measurement gyroMeasurement;
	Measurement accelMeasurement;

	gyroMeasurement.time = time;
	accelMeasurement.time = time;

	gyroMeasurement.value = gyroValues;
	accelMeasurement.value = accelValues;

	//Store measurment
	secondLatestGyro = latestGyro;
	secondLatestAccel = latestAccel;

	latestGyro = gyroMeasurement;
	latestAccel = accelMeasurement;

	++numberOfMeasurmentsDone;
}

/*
 * Analyze the latest measurment;
 */
void mpu9150ExtendedInterface_analysis(void) {
	if(numberOfMeasurmentsDone > 1) {
		double k = (latestAccel.time - secondLatestAccel.time) / 2.0;

		Vector3 deltaSpeed = vector_scalarProduct(k, vector_add(latestAccel.value, secondLatestAccel.value));

		Vector3 offsetGyroLatest = vector_subtract(latestGyro.value, angularRotationOffset);
		Vector3 offsetGyroSecondLatest = vector_subtract(secondLatestGyro.value, angularRotationOffset);
		Vector3 deltaInclination = vector_scalarProduct(k, vector_add(offsetGyroLatest, offsetGyroSecondLatest));

		speed = vector_add(speed, deltaSpeed);
		inclination = vector_add(inclination, deltaInclination);
	} else {
		speed.x = 0;
		speed.y = 0;
		speed.z = 0;

		inclination.x = 0;
		inclination.y = 0;
		inclination.z = 0;
	}
}


/*
 * Interface method to perform a measurment and analysis.
 */
void mpu9150ExtendedInterface_tick(void) {
	mpu9150ExtendedInterface_doMeasurement();
	mpu9150ExtendedInterface_analysis();
}

/*
 * Returns the latest gyro reading;
 */
Vector3 mpu9150ExtendedInterface_getInclinationDerivative(void) {
	return vector_subtract(latestGyro.value, angularRotationOffset);;
}

/*
 * Returns the latest accel reading;
 */
Vector3 mpu9150ExtendedInterface_getSpeedDerivative(void) {
	return latestAccel.value;
}

/*
 * Returns the latest calculated inclination;
 */
Vector3 mpu9150ExtendedInterface_getInclination(void) {
	return inclination;
}

/*
 * Returns the latest calculated speed;
 */
Vector3 mpu9150ExtendedInterface_getSpeed(void) {
	return speed;
}
