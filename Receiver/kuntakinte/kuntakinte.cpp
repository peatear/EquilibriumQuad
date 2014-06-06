// kuntakinte.cpp
#include "pch.h"
#include "kuntakinte.h"

#include <string.h>

using namespace kuntakinte;
using namespace Platform;

//namespace required to support sensor and events
using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;

using namespace Windows::Foundation::Collections;

//namespace for UI control
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;


#define ROLL	0
#define PITCH	1
#define YAW		2




flightbox::flightbox()
{
	
	//init omega array
	omega = ref new Platform::Array<float>(3);
	//initialize rpy array
	rpy = ref new Platform::Array<float>(3);
	//init position array
	position = ref new Platform::Array<float>(3);
	//init motors array
	motors = ref new Platform::Array<float>(4);


	mthrottle = 1000;

	cmdRollRate = 0;
	cmdPitchRate = 0;
	
	rollRateE = 0;
	rollRateEint = 0;
	
	pitchRateE = 0;
	pitchRateEint = 0;

	fault = 0;

	attitude = new float[3];

	mroll = 0;

	m9pid = new PID(&mroll, &m9, &rollsetpoint, (float)60, (float)5 ,(float)40, REVERSE);
	m3pid = new PID(&mroll, &m3, &rollsetpoint, (float)60, (float)5, (float)40, DIRECT);

	m5pid = new PID(&mpitch, &m5, &pitchsetpoint, (float)60, (float)5, (float)40, REVERSE);
	m6pid = new PID(&mpitch, &m6, &pitchsetpoint, (float)60, (float)5, (float)40, DIRECT);

	m9pid->SetMode(AUTOMATIC);
	m3pid->SetMode(AUTOMATIC);
	m5pid->SetMode(AUTOMATIC);
	m6pid->SetMode(AUTOMATIC);
}




Platform::Array<float>^ flightbox::compensate(const Platform::Array<float>^ sensors){
	memcpy(attitude, sensors->Data, 3 * sizeof(float));
	mroll = attitude[ROLL];
	mpitch = attitude[PITCH];
	
	m9pid->Compute();
	m3pid->Compute();
	m5pid->Compute();
	m6pid->Compute();
	
	motors[0] = mthrottle + m9;
	motors[1] = mthrottle + m3;
	motors[2] = mthrottle + m5;
	motors[3] = mthrottle + m6;

	return motors;

}


void flightbox::throttle(float incr){
	mthrottle = incr;
}