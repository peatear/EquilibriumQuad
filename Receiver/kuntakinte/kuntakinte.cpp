// kuntakinte.cpp
#include "pch.h"
#include "kuntakinte.h"

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

	//get inclinometer
	inclinometer = Inclinometer::GetDefault();
	inclinometer->ReportInterval = inclinometer->MinimumReportInterval;
	inclinometer->ReadingChanged::add(ref new TypedEventHandler<Inclinometer ^, InclinometerReadingChangedEventArgs ^>(this, &flightbox::OnInclineReadingChanged));

	//get gyroscope
	gyrometer = Gyrometer::GetDefault();
	gyrometer->ReportInterval = gyrometer->MinimumReportInterval;
	//gyrometer->ReportInterval = 4;

	gyrometer->ReadingChanged::add(ref new TypedEventHandler<Gyrometer^, GyrometerReadingChangedEventArgs^>(this, &flightbox::OnGyroReadingChanged));

	//get accelerometer
	accelerometer = Accelerometer::GetDefault();
	accelerometer->ReportInterval = accelerometer->MinimumReportInterval;
	accelerometer->ReadingChanged::add(ref new TypedEventHandler<Accelerometer ^, AccelerometerReadingChangedEventArgs ^>(this, &flightbox::OnAccelReadingChanged));

	tickgyro = gyrometer->ReportInterval / 1000.0;
	tickincline = inclinometer->ReportInterval / 1000.0;
	tickaccel = accelerometer->ReportInterval / 1000.0;
	//set PID gain of roll loop

	rollGain[0] = 2;
	rollGain[1] = 0.01;
	rollGain[2] = 0.2;

	rollRateG[0] = 0.3;
	rollRateG[1] = 0.001;
	rollRateG[2] = 0.2;

	//set PID gain of pitch loop
	pitchGain[0] = 2;
	pitchGain[1] = 0.01;
	pitchGain[2] = 0.2;

	pitchRateG[0] = 0.3;
	pitchRateG[1] = 0.001;
	pitchRateG[2] = 0.2;

	//set PID gain of yaw loop
	yawGain[0] = 1;
	yawGain[1] = 1;
	yawGain[2] = 1;


	//init omega array
	omega = ref new Platform::Array<float>(3);
	//initialize rpy array
	rpy = ref new Platform::Array<float>(3);
	//init position array
	position = ref new Platform::Array<float>(3);
	//init motors array
	motors = ref new Platform::Array<int>(4);

	offset = 1000;
	cmdRollRate = 0;
	cmdPitchRate = 0;

	rollRateE = 0;
	rollRateEint = 0;

	pitchRateE = 0;
	pitchRateEint = 0;

	fault = 0;

	runningave[0] = 0;
	runningave[1] = 0;
	runningave[2] = 0;
}


void flightbox::initBt(){

	/*
	PeerFinder::FindAllPeersAsync();


	socket = ref new StreamSocket();
	socket->ConnectAsync(;
	dataWriter = ref new DataWriter(socket->OutputStream);
	*/
}

int flightbox::calibrate(float roll, float pitch, float yaw){

	//set calibration value if there is offset
	this->mroll = roll;
	this->mpitch = pitch;
	this->myaw = yaw;

	return 0;


}

int flightbox::rollPID(float rP, float rI, float rD){
	rollGain[0] = rP;
	rollGain[1] = rI;
	rollGain[2] = rD;
	return 0;
}

int flightbox::pitchPID(float pP, float pI, float pD){
	pitchGain[0] = pP;
	pitchGain[1] = pI;
	pitchGain[2] = pD;
	return 0;
}

int flightbox::yawPID(float yP, float yI, float yD){
	return 0;
}


void flightbox::OnInclineReadingChanged(Inclinometer ^sender, InclinometerReadingChangedEventArgs ^args)
{
	//roll and pitch goes from -180 to 180
	//integrate roll; running sum

	//integrate yaw
	//yaw goes from 0 to 360 NEED TO ACCOUNT FOR THIS //////////////////////



	rpy[ROLL] = args->Reading->RollDegrees * 10;
	rpy[PITCH] = args->Reading->PitchDegrees * 10;
	rpy[YAW] = args->Reading->YawDegrees * 10;

	//inclineEvent(rpy);
}

void flightbox::OnGyroReadingChanged(Gyrometer^sender, GyrometerReadingChangedEventArgs ^args){


	/*
	omega[ROLL] = args->Reading->AngularVelocityY*10;
	omega[PITCH] = args->Reading->AngularVelocityX*10;
	omega[YAW] = args->Reading->AngularVelocityZ;
	*/

	runningave[0] = runningave[1];
	runningave[1] = runningave[2];
	runningave[2] = args->Reading->AngularVelocityX;
	//motorEvent(omega);
	//omega[ROLL] = args->Reading->AngularVelocityX;
	omega[0] = (runningave[0] + runningave[1] + runningave[2])*0.33;
	//omega[0] = runningave[2];
	inclineEvent(omega);

}


void flightbox::OnAccelReadingChanged(Accelerometer ^sender, AccelerometerReadingChangedEventArgs ^args)
{

}

void flightbox::throttle(float incr){
	offset = 1000 + incr;
}

/*// kuntakinte.cpp
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
}*/