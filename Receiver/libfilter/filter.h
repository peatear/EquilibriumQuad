#pragma once
//#include "complex.h"

using namespace Platform;
using namespace Windows::Foundation::Metadata;

namespace libfilter
{
    public ref class Filter sealed
    {
    public:
		// Create a filter with a given impulseResponse
        Filter( const Platform::Array<float>^ impulseResponse );

		// Clean everything up, releasing all internally created buffers, etc...
		virtual ~Filter();

		// Feed one new sample into the filter, return the new sample of output
		float filter( float data );

		// Convolve every sample in the input array, returning the array of new output samples
		Platform::Array<float>^ filter( const Platform::Array<float>^ data );
	private:
		float *impulseResponse, *prevData;
		unsigned int idx;
		unsigned int idy;
		unsigned int N;

		float * zeropaddedh;
		float * zeropaddedd;
		float result;

	};
}
