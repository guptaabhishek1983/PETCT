
#include "MPR.h"
#include "MPRSlicer.h"

// meddiff includes
#include "rad_util.h"
#include "rad_logger.h"
#include "RTDcmtkDicomInterface.h"
#include "rad_template_helper.h"

//dcmtk includes

// VTK includes
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include <vtkUnsignedIntArray.h>
#include <vtkSignedCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>

using namespace radspeed;

#define __FILENAME__ "MPR.cpp"
#undef __MODULENAME__
#define __MODULENAME__ "MPR"

#include "MPRTransform.h"
namespace radspeed
{
	struct MPRData
	{
	private:
		vtkSmartPointer<vtkImageData> m_inputImage;
		
	public:
		MPRSlicer* m_slicers[3];
		double m_lastImagePosition[3];
		double m_imageOrientation[6];
		double m_normal[3];
		vtkSmartPointer<vtkMatrix4x4> m_initOrient;

		MPRTransform* m_cursorTransform;
	private:
		
	public:
		MPRSlicer* GetSlicer(Axis axis)
		{
			return m_slicers[axis];
		}
		void SetInput(vtkSmartPointer<vtkImageData> image) { this->m_inputImage = image; }
		vtkSmartPointer<vtkImageData> GetInput() { return this->m_inputImage; }
		
	};
}
MPR::MPR(void)
{
	
	d = new MPRData();
	d->m_initOrient = vtkSmartPointer<vtkMatrix4x4>::New();
	d->m_initOrient->Identity();
	d->m_cursorTransform = MPRTransform::New();
}

MPR::~MPR(void)
{
	for (int i = 0; i<3; i++)
	{
		delete d->m_slicers[i];
	}
	delete d;
}

void MPR::initFromDir1(vector<string> dicomFiles)
{
	// load data from scratch.
	map<float, string> sortedDicomFiles;
	map<float, string>::iterator rit;
	
	for (int i = 0; i < dicomFiles.size(); i++)
	{
		RadRTDicomInterface* pDicom = new RTDcmtkDicomInterface(dicomFiles.at(i).c_str());
		if (!pDicom->IsFileLoadedSuccessfully())
		{
			delete pDicom;
			continue;
		}
		if (!((RTDcmtkDicomInterface*)pDicom)->checkIfRTObject())
		{
			string imagePosition = "";
			vector<string> temp;
			imagePosition = pDicom->Get_IMAGE_POSITION();
			tokenize(imagePosition, temp, "\\");
			sortedDicomFiles.insert(std::pair<float, std::string>(convert_to_double(temp[2].c_str()), dicomFiles.at(i)));
		}
		

		delete pDicom;
		//if (i == 100) break;
	}

	
	RadDataType dicomDataType = TYPE_NOT_SET;
	void* dicomData;
	double firstImagePosition[3];
	int dimensions[3];
	double spacing[3];
	int dicomDataIdx;
	int dicomSliceSize;
	int dicomDataSize;
	int j = 0;

	double wl;
	double ww;
	double rs;
	double ri;
	
	
	for (rit = sortedDicomFiles.begin(); rit != sortedDicomFiles.end(); ++rit)
	{
		
		const char* dicomFile = rit->second.c_str();
		RadRTDicomInterface* pDicom = new RTDcmtkDicomInterface(dicomFile);
		if (j == sortedDicomFiles.size()-1)
		{
			string imagePosition = string(pDicom->Get_IMAGE_POSITION());
			vector<string> _imgPosition;
			tokenize(imagePosition, _imgPosition, "\\", true);
			for (int i = 0; i<_imgPosition.size(); i++)
			{
				firstImagePosition[i] = convert_to_double(_imgPosition.at(i).c_str());
			}

		}
		if (j == 0)
		{
			string imagePosition = string(pDicom->Get_IMAGE_POSITION());
			vector<string> _imgPosition;
			tokenize(imagePosition, _imgPosition, "\\", true);
			for (int i = 0; i<_imgPosition.size(); i++)
			{
				d->m_lastImagePosition[i] = convert_to_double(_imgPosition.at(i).c_str());
				firstImagePosition[i] = convert_to_double(_imgPosition.at(i).c_str());
			}
			
			string imageOrientation = string(pDicom->Get_IMAGE_ORIENTATION());
			/*m_normal[3] = {0,0,0};
			m_imageOrientation[6] = {0,0,0,0,0,0};*/

			vector<string> _imgOrient;
			tokenize(imageOrientation, _imgOrient, "\\", true);
			for (int i = 0; i<_imgOrient.size(); i++)
			{
				d->m_imageOrientation[i] = convert_to_double(_imgOrient.at(i).c_str());
			}
			d->m_normal[0] = (d->m_imageOrientation[1] * d->m_imageOrientation[5]) - (d->m_imageOrientation[2] * d->m_imageOrientation[4]);
			d->m_normal[1] = (d->m_imageOrientation[0] * d->m_imageOrientation[5]) - (d->m_imageOrientation[2] * d->m_imageOrientation[3]);
			d->m_normal[2] = (d->m_imageOrientation[0] * d->m_imageOrientation[4]) - (d->m_imageOrientation[1] * d->m_imageOrientation[3]);

			double matrx[16] = { (d->m_imageOrientation[0]), (d->m_imageOrientation[1]), (d->m_imageOrientation[2]), 0,
				(d->m_imageOrientation[3]), (d->m_imageOrientation[4]), (d->m_imageOrientation[5]), 0,
				(d->m_normal[0]), (d->m_normal[1]), (d->m_normal[2]), 0,
				0, 0, 0, 1 };

			d->m_initOrient->Identity();
			for (int i = 0; i<4; ++i)
			{
				for (int j = 0; j<4; ++j)
				{
					d->m_initOrient->SetElement(i, j, matrx[4 * i + j]);
				}
			}

			d->m_initOrient->Transpose();
			
			if (pDicom->Get_BITS_ALLOCATED() / 8 == 1)
			{
				if (!strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(), "RGB") ||
					!strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(), "PALETTE COLOR"))
					dicomDataType = TYPE_U32Data;
				else
					dicomDataType = TYPE_U8Data;
			}
			else
			{
				if (!pDicom->Get_PIXEL_REPRESENTATION())
					dicomDataType = TYPE_U16Data;
				else
					dicomDataType = TYPE_S16Data;
			}
			dimensions[0] = pDicom->Get_COLOUMN();
			dimensions[1] = pDicom->Get_ROW();
			dimensions[2] = sortedDicomFiles.size();
			dicomDataSize = dimensions[0] * dimensions[1] * dimensions[2];
			dicomSliceSize = dimensions[0] * dimensions[1];

			string pixelSpacing = string(pDicom->Get_PIXEL_SPACING());
			vector<string> _pixelSpacing;
			tokenize(pixelSpacing, _pixelSpacing, "\\", true);

			spacing[0] = convert_to_double(_pixelSpacing.at(0).c_str());
			spacing[1] = convert_to_double(_pixelSpacing.at(1).c_str());
			spacing[2] = firstImagePosition[2];

			ww = convert_to_double(pDicom->Get_WINDOW_WIDTH());
			wl = convert_to_double(pDicom->Get_WINDOW_CENTER());
			rs = convert_to_double(pDicom->Get_RESCALE_SLOPE());
			ri = convert_to_double(pDicom->Get_RESCALE_INTERCEPT());

			RAD_LOG_CRITICAL(dicomDataType);
			switch (dicomDataType)
			{
				case TYPE_U32Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_U32Data));
					break;
				case TYPE_U8Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_U8Data));
					break;
				case TYPE_U16Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_U16Data));
					break;
				case TYPE_S16Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_S16Data));
					break;
			}

			dicomDataIdx = dicomSliceSize;
		}
		if (j == 1)
		{
			string imagePosition = string(pDicom->Get_IMAGE_POSITION());
			vector<string> _imgPosition;
			tokenize(imagePosition, _imgPosition, "\\", true);
			spacing[2] = spacing[2] - convert_to_double(_imgPosition.at(2).c_str());
		}

		image pixelData = born_image();
		image overlayPixelData = born_image();
		pDicom->InflateSingleFrameDicomPixelData(&pixelData, &overlayPixelData);
		void* raw_dcm_data = pixelData.data;

		switch (dicomDataType)
		{
			case TYPE_U32Data: {
				U32DataType* dicomData2 = static_cast<U32DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(U32DataType));
			} break;
			case TYPE_U8Data: {
				U8DataType* dicomData2 = static_cast<U8DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(U8DataType));
			} break;
			case TYPE_U16Data: {
				U16DataType* dicomData2 = static_cast<U16DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(U16DataType));
			} break;
			case TYPE_S16Data: {
				S16DataType* dicomData2 = static_cast<S16DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(S16DataType));
			} break;
		}
		dicomDataIdx += dicomSliceSize;

		if (pixelData.data)
		{
			rad_free_memory(pixelData.data);
		}
		if (overlayPixelData.data)
		{
			rad_free_memory(overlayPixelData.data);
		}

		delete pDicom;
		j++;
	}
	dicomDataIdx -= dicomSliceSize;
	vtkDataArray* scalars = 0;

	switch (dicomDataType)
	{
		case TYPE_U32Data:
			scalars = vtkUnsignedIntArray::New();
			((vtkUnsignedIntArray*)(scalars))->SetArray((unsigned int*)dicomData, dicomDataSize, 1);
			break;
		case TYPE_U8Data:
			scalars = vtkUnsignedCharArray::New();
			((vtkUnsignedCharArray*)(scalars))->SetArray((unsigned char*)dicomData, dicomDataSize, 1);
			break;
		case TYPE_U16Data:
			scalars = vtkUnsignedShortArray::New();
			((vtkUnsignedShortArray*)(scalars))->SetArray((unsigned short*)dicomData, dicomDataSize, 1);
			break;
		case TYPE_S16Data:
			scalars = vtkShortArray::New();
			((vtkShortArray*)(scalars))->SetArray((short*)dicomData, dicomDataSize, 1);
			break;
	}
	scalars->SetNumberOfComponents(1);

	vtkSmartPointer<vtkImageData> CTMPRCuboid = vtkSmartPointer<vtkImageData>::New();
	//vtkImageData* CTMPRCuboid = vtkImageData::New();
	CTMPRCuboid->SetDimensions(dimensions);
	CTMPRCuboid->SetSpacing(spacing);
	CTMPRCuboid->SetOrigin(firstImagePosition);
	CTMPRCuboid->GetPointData()->SetScalars(scalars);
	CTMPRCuboid->GetPointData()->GetScalars()->SetName("CT Cuboid");
	double CTBounds[6];
	CTMPRCuboid->GetBounds(CTBounds);
	//CTMPRCuboid->SetOrigin(CTBounds[0],CTBounds[2],CTBounds);
	RAD_LOG_CRITICAL("<abhishek> bounds:" << CTBounds[0] << ":" << CTBounds[1] << ":" << CTBounds[2] << ":" << CTBounds[3] << ":" << CTBounds[4] << ":" << CTBounds[5]);
	this->initFromImage(CTMPRCuboid, wl,ww,rs,ri);

}
void MPR::initFromDir(string dirPath)
{
	/*vtkSmartPointer<vtkDICOMImageReader> dicomReader  = vtkSmartPointer<vtkDICOMImageReader>::New();
	dicomReader->SetDirectoryName(dirPath.c_str());
	dicomReader->FileLowerLeftOn();
	dicomReader->Update();
	this->initFromImage(dicomReader->GetOutput());*/
}

void MPR::initFromImage(vtkSmartPointer<vtkImageData> image, double wl, double ww, double rs, double ri)
{
	if( d->GetInput()!=NULL )
	{
		d->GetInput()->Delete();
		d->SetInput(NULL);
	}
	d->SetInput(image);
	for(int i=0;i<3;i++)
	{
		MPRSlicer* slicer = new MPRSlicer((Axis)i);
		slicer->SetVOI_LUTParameters(ww, wl, rs, ri);
		slicer->SetInput(image);
		slicer->InitSlicer(d->m_initOrient, d->m_cursorTransform);

		d->m_slicers[i] = slicer;
	}

	// scroll slicers to get middle image
	d->m_cursorTransform->Identity();
	d->m_cursorTransform->Translate(image->GetCenter());
	d->m_cursorTransform->ResetRotations();
}

image MPR::GetOutputImage(Axis axis)
{
	for(int i=0;i<3;i++)
	{
		if(i==(int)axis)
		{
			RAD_LOG_INFO("i:" << i);
			return d->m_slicers[i]->GetOutputImage();
		}
	}
	return ::born_image();
}

void MPR::Scroll(Axis axis, double delta)
{
	double pos[] = { 0, 0, 0 };
	d->m_slicers[(int)axis]->GetTransform()->TransformPoint(pos, pos);

	double t[3] = { 0, 0, -delta };
	d->m_slicers[(int)axis]->GetTransform()->TransformPoint(t, t);
	t[0] -= pos[0]; t[1] -= pos[1]; t[2] -= pos[2];

	double bounds[] = { 0, 0, 0, 0, 0, 0 };
	d->GetInput()->GetBounds(bounds);

	for (int i = 0; i<3; i++)
	{
		double v = pos[i] + t[i];
		if (v < bounds[i * 2] || v > bounds[i * 2 + 1])
		{
			RAD_LOG_CRITICAL("Going out of bounds. Returning")
				return;
		}
	}

	d->m_cursorTransform->Translate(t);
}

void MPR::Scroll(Axis axis, int delta)
{
	double spacing[3];
	d->GetInput()->GetSpacing(spacing);
	double _spacing = spacing[2];

	switch (axis)
	{
		case radspeed::AxialAxis:

			break;
		case radspeed::CoronalAxis:
			_spacing = spacing[1];
			break;
		case radspeed::SagittalAxis:
			_spacing = spacing[0];
			break;
		default:
			break;
	}
	this->Scroll(axis, delta*_spacing);
	
}

void MPR::Scroll2(Axis axis, float dx, float dy)
{
	
	// Find out the current postion
	double pos[] = { 0, 0, 0 };
	d->m_slicers[(int)axis]->GetTransform()->TransformPoint(pos, pos);

	// Translate the current slice matrix
	double t[] = { dx, dy, 0 };
	d->m_slicers[(int)axis]->GetTransform()->Translate(t[0], t[1], t[2]);

	// Find out the translated position
	double tpos[] = { 0, 0, 0 };
	d->m_slicers[(int)axis]->GetTransform()->TransformPoint(tpos, tpos);

	// Translate the slice matrix back to its previous position
	d->m_slicers[(int)axis]->GetTransform()->Translate(-t[0], -t[1], -t[2]);

	// Evaluate the actual translation
	for (int i = 0; i<3; i++)
		t[i] = tpos[i] - pos[i];

	// Ensure that tpos is not outside bounds
	double bounds[] = { 0, 0, 0, 0, 0, 0 };
	d->GetInput()->GetBounds(bounds);
	for (int i = 0; i<3; i++)
	{
		double& v = tpos[i];
		if (v < bounds[i * 2] || v > bounds[i * 2 + 1])
			return;
	}

	// Apply the translation on the cursor and update.
	d->m_cursorTransform->Translate(t);
	return;
}
int MPR::GetNumberOfImages(Axis axis)
{
	int numberOfImages = 0;
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			numberOfImages = d->m_slicers[i]->GetNumberOfImages();
		}
	}
	return numberOfImages;
}

int MPR::GetCurrentImageIndex(Axis axis)
{
	int idx = 0;
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			idx = d->m_slicers[i]->GetSlicerPositionAsIndex();
		}
	}
	return idx;
}

double MPR::GetCurrentImagePosition(Axis axis)
{
	double pos = 0;
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			pos = d->m_slicers[i]->GetSlicerPosition();
		}
	}
	return pos;
}

void MPR::GetCurrentSlicerPositionRelativeToIndex(Axis axis, double& xPos, double& yPos)
{
	//double pos[] = { 0, 0, 0 };
	//d->m_slicers[(int)axis]->GetTransform()->TransformPoint(pos);// GetPosition(pos);


	double pos[] = { 0, 0, 0 };
	d->m_slicers[(int)axis]->GetTransform()->TransformPoint(pos, pos);

	double origin1[3];
	d->m_slicers[(int)axis]->GetRawOutputImage()->GetOrigin(origin1);

	double origin[3];
	d->GetInput()->GetOrigin(origin);

	double spacing[3] = { 0, 0, 0 };
	d->GetInput()->GetSpacing(spacing);

	//// convert to physical coordinates as per David's advise.
	double xt = origin1[0] + spacing[0] * pos[0];
	double yt = origin1[1] + spacing[1] * pos[1];
	double zt = origin1[2] + spacing[2] * pos[2];

	// Apply the transform....
	double * tp = d->m_slicers[(int)axis]->GetTransform()->GetLinearInverse()->TransformPoint(pos);

	double out[4] = { 0, 0, 0, 0 };
	// Convert back...
	out[0] = (tp[0] - origin1[0]) / spacing[0];
	out[1] = (tp[1] - origin1[1]) / spacing[1];
	out[2] = (tp[2] - origin1[2]) / spacing[2];
	out[3] = 0.0;

	xPos = fabs(out[0]);
	yPos = fabs(out[1]);
	double zPos = fabs(out[2]);
	/*RAD_LOG_CRITICAL("************************");
	RAD_LOG_CRITICAL("Axis:" << axis);
	RAD_LOG_CRITICAL("(VTK) Transform Pos:" << pos[0] << ":" << pos[1] << ":" << pos[2]);
	RAD_LOG_CRITICAL("(VTK) Output origin:" << origin1[0] << ":" << origin1[1] << ":" << origin1[2]);
	RAD_LOG_CRITICAL("(VTK Math) Position:" << out[0] << ":" << out[1] << ":" << out[2]);
	RAD_LOG_CRITICAL("Pos:" << xPos << ":" << yPos << ":" << zPos);
	RAD_LOG_CRITICAL("************************");*/
}

void MPR::GetOutputImageDisplayDimensions(Axis axis, int& width, int& height)
{
	
	int dim[3] = { 0, 0, 0 };
	d->m_slicers[(int)axis]->GetRawOutputImage()->GetDimensions(dim);
	width = dim[0];
	height = dim[1];

	/*d->GetInput()->GetDimensions(dim);
	switch (axis)
	{
		case radspeed::AxialAxis:
		{
			width = dim[0] ;
			height = dim[1];
		}
			break;
		case radspeed::CoronalAxis:
		{
			width = dim[0] ;
			height = dim[2];
		}
			break;
		case radspeed::SagittalAxis:
		{
			width = dim[1] ;
			height = dim[2];
		}
			break;
		default:
			break;
	}*/
	/*double spacing[3] = { 0, 0, 0 };
	d->GetInput()->GetSpacing(spacing);
	spacing[0] = fabs(spacing[0]);
	spacing[1] = fabs(spacing[1]);
	spacing[2] = fabs(spacing[2]);
	*/
}


string MPR::GetOrientationMarkerLeft(Axis axis)
{
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			return d->m_slicers[i]->GetOrientationMarkers_L();
		}
	}
	return "";
}

string MPR::GetOrientationMarkerRight(Axis axis)
{
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			return d->m_slicers[i]->GetOrientationMarkers_R();
		}
	}
	return "";
}
string MPR::GetOrientationMarkerTop(Axis axis)
{
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			return d->m_slicers[i]->GetOrientationMarkers_T();
		}
	}
	return "";
}
string MPR::GetOrientationMarkerBottom(Axis axis)
{
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			return d->m_slicers[i]->GetOrientationMarkers_B();
		}
	}
	return "";
}

void MPR::GetOrigin(double& x, double& y, double& z)
{
	d->GetInput()->GetOrigin(x, y, z);
}

vtkSmartPointer<vtkImageData> MPR::GetInput()
{
	return d->GetInput();
}

long int MPR::GetPixelIntensity(Axis axis, int x_pos, int y_pos)
{
	for (int i = 0; i < 3; i++)
	{
		if (i == (int)axis)
		{
			return d->m_slicers[i]->GetPixelIntensity(x_pos, y_pos);
		}
	}
	return 0;
}

void MPR::RotateAxesAlongPlane(int axis, int angle)
{
	switch (axis)
	{
		case AxialAxis:
		{
			// rotate sagittal
			d->m_cursorTransform->RotateX(angle);
			//rotate coronal
			d->m_cursorTransform->RotateY(angle);

			//d->m_slicers[SagittalAxis]->GetTransform()->RotateX(angle);
			//d->m_slicers[CoronalAxis]->GetTransform()->RotateY(angle);
			//d->m_cursorTransform->RotateX(angle);
			//d->m_cursorTransform->RotateZ(angle);
		}
			break;
		case CoronalAxis:
		{
			// rotate sagittal
			d->m_cursorTransform->RotateX(angle);
			// rotate axial
			d->m_cursorTransform->RotateZ(angle);

			//d->m_slicers[SagittalAxis]->GetTransform()->RotateX(angle);
			//d->m_slicers[AxialAxis]->GetTransform()->RotateZ(angle*);
		}
			break;
		case SagittalAxis:
		{
			// rotate axial
			d->m_cursorTransform->RotateZ(angle);
			// rotate coronal
			d->m_cursorTransform->RotateY(angle);

			//d->m_slicers[AxialAxis]->GetTransform()->RotateY(angle);
			//d->m_slicers[CoronalAxis]->GetTransform()->RotateZ(angle);
		}
			break;
		default:
			break;
	}
}