//#include "pch.h"
#include "filter.h"
#include <string.h>

using namespace libfilter;

Filter::Filter( const Platform::Array<float>^ impulseResponse ) {
	// Initialize everything here
	N = impulseResponse->Length;
	this->impulseResponse = new float[this->N];



	//float * impulseResponseData = impulseResponse->Data;
	memcpy(this->impulseResponse, impulseResponse->Data, N*sizeof(float));

	prevData = new float[N];
	for (int i = 0; i < N; i++){
		prevData[i] = 0;
	}
}

Filter::~Filter() {
	// Clean everything up here
}

float Filter::filter( float data ) {
	// Filter a single sample
	// Return the output sample
	result=0;

	for (idx = N-1; idx >0; idx--){
		this->prevData[idx] = prevData[idx - 1];
	}

	this->prevData[0] = data;

	for (idx = 0; idx < N; idx++){
		result += this->prevData[idx] * this->impulseResponse[idx];
	}

	
	return result;
}

Platform::Array<float>^ Filter::filter( const Platform::Array<float>^ data ) {
	// Filter each sample in the input array `data`
	// Return the array of output

	//output length of convolve sequence N+M-1
	
	Platform::Array<float>^ output = ref new Platform::Array<float>(N + data->Length - 1);	
	/*
	
	zeropaddedh = new float[N + data->Length - 1];
	zeropaddedd = new float[N + data->Length - 1];
	memset(zeropaddedh, (float)0.0, (N+data->Length-1)*sizeof(float));
	memcpy(zeropaddedh, impulseResponse, N*sizeof(float));
	memset(zeropaddedd, (float)0.0, (N + data->Length - 1)*sizeof(float));
	memcpy(zeropaddedd, data->Data, data->Length*sizeof(float));

	//convolve impulse response with data
	for (idx = 0; idx < N+data->Length-1; idx++){
		
		output[idx] = 0;
		for (idy = 0; idy <= idx; idy++){
			output[idx] =output[idx]+zeropaddedh[idy] * zeropaddedd[idx - idy];
		}
	}
	
	delete zeropaddedd;
	delete zeropaddedh;
	*/

	return output;
}
