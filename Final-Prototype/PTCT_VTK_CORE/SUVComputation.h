#pragma once
#include "RTDcmtkDicomInterface.h"
namespace radspeed
{
	struct SUVParameters; 
	class SUVComputation
	{
	public:
		SUVComputation();
		~SUVComputation();

		void SetParams(RTDcmtkDicomInterface* pDicom);
		double GetSUV(long int pixelVal);
	private:

		SUVParameters* d;
	};

}