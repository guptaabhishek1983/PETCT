
#include "MPRSlicer.h"
// meddiff includes

#include "rad_logger.h"
#include "streaming-image.h"
// VTK includes
#include "vtkImageReslice.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkTransform.h"

using namespace radspeed;
using namespace std;
#define __FILENAME__ "MPRSlicer.cpp"
#undef __MODULENAME__
#define __MODULENAME__ "MPRSlicer"

MPRSlicer::MPRSlicer(Axis axis)
{
	this->m_axis = axis;
	this->m_position = 0;
	this->m_inputImage = NULL;
	this->displayData = NULL;
	this->displayImage = ::born_image();
}

MPRSlicer::~MPRSlicer(void)
{
	this->m_resliceMatrix->Delete();
	this->m_reslice->Delete();
}
void MPRSlicer::InitSlicer()
{
	// set up reslice.
	this->m_reslice = vtkSmartPointer<vtkImageReslice>::New();
	this->m_resliceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	this->m_resliceMatrix->Identity();

	switch (this->m_axis)
	{
		case AxialAxis:
		{
			this->m_resliceMatrix->DeepCopy(axialElements);
			vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
			transform->SetMatrix(this->m_resliceMatrix);
			transform->Update();
			this->m_resliceMatrix->DeepCopy(transform->GetMatrix());
		}
			break;
		case SagittalAxis:
		{
			this->m_resliceMatrix->DeepCopy(sagittalElements);
			// reorient reslice matrix to show image up-right and in correct orientation
			vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
			transform->SetMatrix(this->m_resliceMatrix);
			//transform->RotateZ(180);
			//transform->RotateY(180);
			transform->Update();
			// reorientation done. Now set new reslice matrix back.
			this->m_resliceMatrix->DeepCopy(transform->GetMatrix());
		}
			break;
		case CoronalAxis:
		{
			this->m_resliceMatrix->DeepCopy(coronalElements);
			// reorient reslice matrix to show image up-right and in correct orientation
			vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
			transform->SetMatrix(this->m_resliceMatrix);
			//transform->RotateX(180);
			transform->Update();
			// reorientation done. Now set new reslice matrix back.
			this->m_resliceMatrix->DeepCopy(transform->GetMatrix());
		}
			break;
	}


	this->m_reslice->SetResliceAxes(this->m_resliceMatrix);
	this->m_reslice->InterpolateOff();
	this->m_reslice->SetOutputDimensionality(2);
	this->m_reslice->SetOutputSpacing(this->m_spacing);
	
	this->m_inputImage->GetOrigin(m_origin);
	switch (this->m_axis)
	{
		case AxialAxis:
			this->m_position = m_origin[2];
			break;
		case CoronalAxis:
			this->m_position = m_origin[1];
			break;
		case SagittalAxis:
			this->m_position = m_origin[0];
			break;
		default:
			break;
	}
	this->SetReslicePosition(m_origin);
	this->ComputeOrientationMarkers();
}
void MPRSlicer::SetReslicePosition(double point[3])
{
	this->m_reslice->GetResliceAxes()->SetElement(0,3,point[0]);
	this->m_reslice->GetResliceAxes()->SetElement(1,3,point[1]);
	this->m_reslice->GetResliceAxes()->SetElement(2,3,point[2]);
	this->m_resliceMatrix->Modified();
}

image MPRSlicer::GetOutputImage()
{
	switch(this->m_axis)
	{
		case AxialAxis:
			{
				this->m_resliceMatrix->SetElement(0, 3, 0); 
				this->m_resliceMatrix->SetElement(1, 3, 0);
				this->m_resliceMatrix->SetElement(2, 3, m_position);
			}
			break;

		case CoronalAxis:
			{
			this->m_resliceMatrix->SetElement(0, 3, 0);
			this->m_resliceMatrix->SetElement(1, 3, m_position);
			this->m_resliceMatrix->SetElement(2, 3, 0);
			}
			break;

		case SagittalAxis:
			{
			this->m_resliceMatrix->SetElement(0, 3, m_position);
			this->m_resliceMatrix->SetElement(1, 3, 0);
			this->m_resliceMatrix->SetElement(2, 3, 0);
			}
			break;
	}

	this->m_reslice->SetResliceAxes(m_resliceMatrix);
	this->m_reslice->SetInputData(this->m_inputImage);
	this->m_reslice->SetOutputDimensionality(2); 
	this->m_reslice->SetInterpolationModeToCubic();
	this->m_reslice->Update();

	this->m_outputImage = this->m_reslice->GetOutput();

	if (this->m_outputImage != NULL)
	{
		int outDim[3] = { 0, 0, 0 };
		this->m_outputImage->GetDimensions(outDim);
		// Catch hold of arrays
		vtkDataArray* inScalars = this->m_outputImage->GetPointData()->GetScalars();

		image in_dcm = born_image();
		// Perform Window Level and Window width computations
		in_dcm.width = outDim[0];
		in_dcm.height = outDim[1];
		in_dcm.size = in_dcm.width*in_dcm.height;
		switch (inScalars->GetDataType())
		{
			case VTK_UNSIGNED_INT:
				in_dcm.type = TYPE_U32Data;
				break;
			case VTK_UNSIGNED_CHAR:
				in_dcm.type = TYPE_U8Data;
				break;
			case VTK_SHORT:
				in_dcm.type = TYPE_S16Data;
				break;
			case VTK_UNSIGNED_SHORT:
				in_dcm.type = TYPE_U16Data;
				break;
		}
		in_dcm.data = inScalars->GetVoidPointer(0);

		if (this->displayData != NULL)
		{
			rad_free_memory(this->displayData);
			this->displayData = NULL;
		}
		
		
		displayImage.width = in_dcm.width;
		displayImage.height = in_dcm.height;
		displayImage.size = in_dcm.size;
		displayImage.type = TYPE_U8Data;
		
		this->displayData = rad_get_memory(displayImage.height*displayImage.width*rad_sizeof(displayImage.type));
		
		displayImage.data = this->displayData;

		voi_lut_transform_image_fast(displayImage, in_dcm, m_ww, m_wl,
			0, 255,
			m_rs,
			m_ri);

		// convert 8 bit display data to 32 bit RGBA.
		/*displayImage.width = in_dcm.width;
		displayImage.height = in_dcm.height;
		displayImage.size = in_dcm.size;
		displayImage.type = TYPE_U32Data;
		
		U32Data _32BitDisplayData = (U32Data)rad_get_memory(displayImage.height*displayImage.width*rad_sizeof(displayImage.type));


		decode_rgb_to_argb((U8Data)_8BitDisplayData, (U8Data)_8BitDisplayData, (U8Data)_8BitDisplayData, _32BitDisplayData, displayImage.size);*/
		

		//rad_free_memory(_8BitDisplayData);
		
	}
	
	return displayImage;
}

void MPRSlicer::Scroll(int delta)
{
	RAD_LOG_INFO("Scrolling delta:" << delta);
	RAD_LOG_INFO("Old position:" << this->m_position);
	switch(this->m_axis)
	{
	case AxialAxis:
		this->m_position += delta*m_spacing[2];
		break;
	case CoronalAxis:
		this->m_position += delta*m_spacing[1];
		break;
	case SagittalAxis:
		this->m_position += delta*m_spacing[0];
	}
	RAD_LOG_INFO("New position:" << this->m_position);
	return;
}

int MPRSlicer::GetNumberOfImages()
{
	int num = 0;
	switch (this->m_axis)
	{
		case AxialAxis:
			num = this->m_dimension[2];
			break;
		case CoronalAxis:
			num = this->m_dimension[1];
			break;
		case SagittalAxis:
			num = this->m_dimension[0];
			break;
		default:
			break;
	}
	return num;
}

int MPRSlicer::GetSlicerPositionAsIndex()
{
	int idx = 0;
	switch (this->m_axis)
	{
		case AxialAxis:
			idx = this->m_spacing[2]==0 ? 0 : (this->m_position - this->m_origin[2])/this->m_spacing[2];
			break;
		case CoronalAxis:
			idx = this->m_spacing[0] == 0 ? 0 : (this->m_position - this->m_origin[1]) / this->m_spacing[1];
			break;
		case SagittalAxis:
			idx = this->m_spacing[1] == 0 ? 0 : (this->m_position - this->m_origin[0]) / this->m_spacing[0];
			break;
		default:
			break;
	}
	return abs(idx);
}

double MPRSlicer::GetSlicerPosition()
{
	return this->m_position;
}

//char *
//DerivedImagePlane::getOrientation(Vector3D vector)
//{
//	char *orientation = new char[4];
//	char *optr = orientation;
//	*optr = '\0';
//
//	char orientationX = vector.getX() < 0 ? 'R' : 'L';
//	char orientationY = vector.getY() < 0 ? 'A' : 'P';
//	char orientationZ = vector.getZ() < 0 ? 'F' : 'H';
//
//	double absX = fabs(vector.getX());
//	double absY = fabs(vector.getY());
//	double absZ = fabs(vector.getZ());
//
//	int i;
//	for (i = 0; i<3; ++i) {
//		if (absX>.0001 && absX>absY && absX>absZ) {
//			*optr++ = orientationX;
//			absX = 0;
//		}
//		else if (absY>.0001 && absY>absX && absY>absZ) {
//			*optr++ = orientationY;
//			absY = 0;
//		}
//		else if (absZ>.0001 && absZ>absX && absZ>absY) {
//			*optr++ = orientationZ;
//			absZ = 0;
//		}
//		else break;
//		*optr = '\0';
//	}
//	return orientation;
//}

// helper methods.
string CalucateOrientation(double vectorX, double vectorY, double vectorZ)
{
	string orientation = "";
	string orientationX = vectorX <= (double)0 ? "R" : "L";
	string orientationY = vectorY <= (double)0 ? "A" : "P";
	string orientationZ = vectorZ <= (double)0 ? "I" : "S";


	double absX = abs(vectorX);
	double absY = abs(vectorY);
	double absZ = abs(vectorZ);

	double obliquity = 0.50;
	for (int i = 0; i<3; ++i) {
		if (absX>obliquity && absX >= absY && absX >= absZ) {
			orientation.append(orientationX);
			absX = 0;
		}
		else if (absY>obliquity && absY >= absX && absY >= absZ) {
			orientation.append(orientationY);
			absY = 0;
		}
		else if (absZ>obliquity && absZ >= absX && absZ >= absY) {
			orientation.append(orientationZ);
			absZ = 0;
		}
		else
			break;
	}
	return(orientation);
}
string GetOtherOrientation(string oneOrientation)
{
	string otherOrientation;
	if (strcmp(oneOrientation.c_str(), "A") == 0)
	{
		otherOrientation = "P";
	}
	if (strcmp(oneOrientation.c_str(), "P") == 0)
	{
		otherOrientation = "A";
	}

	if (strcmp(oneOrientation.c_str(), "L") == 0)
	{
		otherOrientation = "R";
	}
	if (strcmp(oneOrientation.c_str(), "R") == 0)
	{
		otherOrientation = "L";
	}

	if (strcmp(oneOrientation.c_str(), "I") == 0)
	{
		otherOrientation = "S";
	}
	if (strcmp(oneOrientation.c_str(), "S") == 0)
	{
		otherOrientation = "I";
	}
	return otherOrientation;

}
void MPRSlicer::ComputeOrientationMarkers()
{
	double x[3] = { 0, 0, 0 };
	double y[3] = { 0, 0, 0 };
	double z[3] = { 0, 0, 0 };
	this->m_reslice->GetResliceAxesDirectionCosines(x, y, z);

	this->m_orientatationMarkers_L = CalucateOrientation(x[0], x[1], x[2]);
	this->m_orientatationMarkers_R = GetOtherOrientation(this->m_orientatationMarkers_L);
	this->m_orientatationMarkers_B = CalucateOrientation(y[0], y[1], y[2]);
	this->m_orientatationMarkers_T = GetOtherOrientation(this->m_orientatationMarkers_B);
}