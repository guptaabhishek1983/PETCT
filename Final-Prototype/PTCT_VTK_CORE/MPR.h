#pragma once
#include <iostream>
#include <vector>
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "enums.h"
#include "rad_util.h"
using namespace std;
namespace radspeed
{
	struct MPRData;
	class MPR
	{
	public:
		MPR(void);
		~MPR(void);

		void initFromDir(string dirPath);
		void initFromDir1(vector<string> dicomFiles);
		void initFromImage(vtkSmartPointer<vtkImageData> image, double wl, double ww, double rs,double ri);
		image GetOutputImage(Axis axis);
		void Scroll(Axis axis, int delta);
		void Scroll2(Axis axis, float newPosition);
		int GetNumberOfImages(Axis axis);
		int GetCurrentImageIndex(Axis axis);
		double GetCurrentImagePosition(Axis axis);
		double GetCurrentImagePositionRelativeToOrigin(Axis axis);
		void GetOutputImageDisplayDimensions(Axis axis, int& width, int& height);
		string GetOrientationMarkerLeft(Axis axis);
		string GetOrientationMarkerRight(Axis axis);
		string GetOrientationMarkerTop(Axis axis);
		string GetOrientationMarkerBottom(Axis axis);

		void GetOrigin(double& x, double& y, double& z);

	private:
		MPRData* d;
	};
}