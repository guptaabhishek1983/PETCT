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
		void initFromImage(vtkSmartPointer<vtkImageData> image, double wl, double ww, double rs, double ri);
		image GetOutputImage(Axis axis);
		void Scroll(Axis axis, int delta);
		void Scroll2(Axis axis, float dx, float dy);
		int GetNumberOfImages(Axis axis);
		int GetCurrentImageIndex(Axis axis);
		double GetCurrentImagePosition(Axis axis);
		void GetCurrentSlicerPositionRelativeToIndex(Axis axis, double& xPos, double& yPos);
		void GetOutputImageDisplayDimensions(Axis axis, int& width, int& height);
		string GetOrientationMarkerLeft(Axis axis);
		string GetOrientationMarkerRight(Axis axis);
		string GetOrientationMarkerTop(Axis axis);
		string GetOrientationMarkerBottom(Axis axis);
		void GetXYZPixelSpacing(int axis, double* spacing);

		double GetPixelSpacing(int axis);
		long int GetPixelIntensity(Axis axis, int x_pos, int y_pos);

		void RotateAxesAlongPlane(int axis, int angle);
		vtkSmartPointer<vtkImageData> GetInput();
		void GetOrigin(double& x, double& y, double& z);

		// Method to translate slicer position.
		// Input to this method: axis & delta (spacing considered)
		void Scroll(Axis axis, double delta);
	private:
		MPRData* d;
	};
}