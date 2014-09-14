// Fusion-1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

#include "dirent.h"
#include "Reslice_Matrix.h"

#include "rad_util.h"
#include "rad_template_helper.h"
#include "rad_logger.h"
#include "RTDcmtkDicomInterface.h"
#define __FILENAME__ "Fusion-1.cpp"
#undef  __MODULENAME__
#define __MODULENAME__ "Fusion-1"

#include "dcmtk\dcmjpeg\djdecode.h"  /* for dcmjpeg decoders */
#include "dcmtk\dcmjpeg\djencode.h"  /* for dcmjpeg encoders */
#include "dcmtk\dcmdata\dcrledrg.h"  /* for rle decoders */	
#include "dcmtk\dcmjpeg\djrplol.h"   /* for DJ_RPLossless */
#include "dcmtk\dcmjpeg\djrploss.h"  /* for DJ_RPLossy */

#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkImageActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkShortArray.h>
#include <vtkPointData.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleImage.h>
using namespace std;

class vtkImageInteractionCallback : public vtkCommand
{
public:

	static vtkImageInteractionCallback *New() {
		return new vtkImageInteractionCallback;
	};

	vtkImageInteractionCallback() {
		this->Slicing = 0;
		this->ImageReslice = 0;
		this->Interactor = 0;
	};

	void SetImageReslice(vtkImageReslice *reslice) {
		this->ImageReslice = reslice;
	};

	vtkImageReslice *GetImageReslice() {
		return this->ImageReslice;
	};

	void SetInteractor(vtkRenderWindowInteractor *interactor) {
		this->Interactor = interactor;
	};

	vtkRenderWindowInteractor *GetInteractor() {
		return this->Interactor;
	};

	virtual void Execute(vtkObject *, unsigned long event, void *)
	{
		vtkRenderWindowInteractor *interactor = this->GetInteractor();

		int lastPos[2];
		interactor->GetLastEventPosition(lastPos);
		int currPos[2];
		interactor->GetEventPosition(currPos);

		if (event == vtkCommand::LeftButtonPressEvent)
		{
			this->Slicing = 1;
		}
		else if (event == vtkCommand::LeftButtonReleaseEvent)
		{
			this->Slicing = 0;
		}
		else if (event == vtkCommand::MouseMoveEvent)
		{
			if (this->Slicing)
			{
				vtkImageReslice *reslice = this->ImageReslice;

				// Increment slice position by deltaY of mouse
				int deltaY = lastPos[1] - currPos[1];

				//reslice->GetOutput();
				
				double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
				vtkMatrix4x4 *matrix = reslice->GetResliceAxes();
				// move the center point that we are slicing through
				double point[4];
				double center[4];
				point[0] = 0.0;
				point[1] = 0.0;
				point[2] = sliceSpacing * deltaY;
				point[3] = 1.0;
				matrix->MultiplyPoint(point, center);
				matrix->SetElement(0, 3, center[0]);
				matrix->SetElement(1, 3, center[1]);
				matrix->SetElement(2, 3, center[2]);
				cout << center[0] << ":" << center[1] << ":" << center[2] << endl;
				this->ImageReslice->SetResliceAxes(matrix);
				this->ImageReslice->UpdateInformation();
				this->ImageReslice->Update();
				interactor->Render();
			}
			else
			{
				vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(
					interactor->GetInteractorStyle());
				if (style)
				{
					style->OnMouseMove();
				}
			}
		}
	};

private:

	// Actions (slicing only, for now)
	int Slicing;

	// Pointer to vtkImageReslice
	vtkImageReslice *ImageReslice;

	// Pointer to the interactor
	vtkRenderWindowInteractor *Interactor;
};

int _tmain(int argc, _TCHAR* argv[])
{
	rad_setLogLevel(7);
	rad_setLogFileName("Fusion-1.log");
	string CT_Series_Folder = "D:\\DicomDataSet\\PTCT\\Dataset1\\1.3.12.2.1107.5.1.4.1007.30000012032001413290600020180";
	map<double, string> sortedImage;
	map<double, string>::iterator it;
	
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(CT_Series_Folder.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
				continue;
			string dicomFileLocation = CT_Series_Folder + "\\" + ent->d_name;
			RadRTDicomInterface* pDicom = new RTDcmtkDicomInterface(dicomFileLocation.c_str());
			if (pDicom->IsFileLoadedSuccessfully())
			{
				string ipp = pDicom->Get_IMAGE_POSITION();
				vector<string> _ipp;
				tokenize(ipp, _ipp, "\\", true);
				double z = convert_to_double(_ipp[2].c_str());
				sortedImage.insert(pair<double, string>(z, dicomFileLocation));
			}
			delete pDicom;
		}
	}

	// Create CT cuboid
	int CTDimension[3] = { 0,0,0 };
	long CTDicomSliceSize = 0;// CTDimension[0] * CTDimension[1];
	long CTDicomDataSize = 0;// CTDimension[0] * CTDimension[1] * CTDimension[2];
	long CTDicomDataIdx = 0;// CTDicomSliceSize;
	double CTSpacing[3] = { 0,0,0 };
	double CTImagePosition[3] = { 0, 0, 0 };
	RadDataType CTDicomDataType = TYPE_NOT_SET;
	void* CTDicomData;

	int count = 0;
	for (it = sortedImage.begin(); it != sortedImage.end(); ++it)
	{

		cout << "Loading image at position:" << it->first << endl;
		RadRTDicomInterface* pDicom = new RTDcmtkDicomInterface(it->second.c_str());
		if (count == 0)
		{
			CTDimension[0] = pDicom->Get_ROW();
			CTDimension[1] = pDicom->Get_COLOUMN();
			CTDimension[2] = sortedImage.size();

			CTDicomSliceSize = CTDimension[0] * CTDimension[1];
			CTDicomDataSize = CTDimension[0] * CTDimension[1] * CTDimension[2];

			CTDicomDataIdx = CTDicomSliceSize;

			string position = pDicom->Get_IMAGE_POSITION();
			vector<string> _pos;
			tokenize(position, _pos, "\\", true);
			CTImagePosition[0] = convert_to_double(_pos[0].c_str());
			CTImagePosition[1] = convert_to_double(_pos[1].c_str());
			CTImagePosition[2] = convert_to_double(_pos[2].c_str());

			string pixelSpacing = pDicom->Get_PIXEL_SPACING();
			vector<string> _ps;
			tokenize(pixelSpacing, _ps, "\\", true);
			CTSpacing[0] = convert_to_double(_ps[0].c_str());
			CTSpacing[1] = convert_to_double(_ps[1].c_str());
			CTSpacing[2] = convert_to_double(_pos[2].c_str());

			if (pDicom->Get_BITS_ALLOCATED() / 8 == 1)
			{
				if (!strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(), "RGB") ||
					!strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(), "PALETTE COLOR"))
					CTDicomDataType = TYPE_U32Data;
				else
					CTDicomDataType = TYPE_U8Data;
			}
			else
			{
				if (!pDicom->Get_PIXEL_REPRESENTATION())
					CTDicomDataType = TYPE_U16Data;
				else
					CTDicomDataType = TYPE_S16Data;
			}

			switch (CTDicomDataType)
			{
				case TYPE_U32Data:
					CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_U32Data));
					break;
				case TYPE_U8Data:
					CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_U8Data));
					break;
				case TYPE_U16Data:
					CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_U16Data));
					break;
				case TYPE_S16Data:
					CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_S16Data));
					break;
			}

			CTDicomDataIdx = CTDicomSliceSize;
		}
		if (count == 1)
		{
			string position = pDicom->Get_IMAGE_POSITION();
			vector<string> _pos;
			tokenize(position, _pos, "\\", true);
			CTSpacing[2] = convert_to_double(_pos[2].c_str()) - CTSpacing[2];
		}
		
		image pixelData = born_image();
		image overlayPixelData = born_image();
		pDicom->InflateSingleFrameDicomPixelData(&pixelData, &overlayPixelData);
		void* raw_dcm_data = pixelData.data;

		switch (CTDicomDataType)
		{
			case TYPE_U32Data: {
				U32DataType* dicomData2 = static_cast<U32DataType*>(CTDicomData);
				dicomData2 += CTDicomDataSize;
				memcpy(dicomData2 - CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(U32DataType));
			} break;
			case TYPE_U8Data: {
				U8DataType* dicomData2 = static_cast<U8DataType*>(CTDicomData);
				dicomData2 += CTDicomDataSize;
				memcpy(dicomData2 - CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(U8DataType));
			} break;
			case TYPE_U16Data: {
				U16DataType* dicomData2 = static_cast<U16DataType*>(CTDicomData);
				dicomData2 += CTDicomDataSize;
				memcpy(dicomData2 - CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(U16DataType));
			} break;
			case TYPE_S16Data: {
				S16DataType* dicomData2 = static_cast<S16DataType*>(CTDicomData);
				dicomData2 += CTDicomDataSize;
				memcpy(dicomData2 - CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(S16DataType));
			} break;
		}
		CTDicomDataIdx += CTDicomSliceSize;

		if (pixelData.data)
		{
			rad_free_memory(pixelData.data);
		}
		if (overlayPixelData.data)
		{
			rad_free_memory(overlayPixelData.data);
		}

		count+=1;
		delete pDicom;
	}
	CTDicomDataIdx -= CTDicomSliceSize;
	vtkDataArray* CTScalars = 0;

	switch (CTDicomDataType)
	{
		case TYPE_U32Data:
			CTScalars = vtkUnsignedIntArray::New();
			((vtkUnsignedIntArray*)(CTScalars))->SetArray((unsigned int*)CTDicomData, CTDicomDataSize, 1);
			break;
		case TYPE_U8Data:
			CTScalars = vtkUnsignedCharArray::New();
			((vtkUnsignedCharArray*)(CTScalars))->SetArray((unsigned char*)CTDicomData, CTDicomDataSize, 1);
			break;
		case TYPE_U16Data:
			CTScalars = vtkUnsignedShortArray::New();
			((vtkUnsignedShortArray*)(CTScalars))->SetArray((unsigned short*)CTDicomData, CTDicomDataSize, 1);
			break;
		case TYPE_S16Data:
			CTScalars = vtkShortArray::New();
			((vtkShortArray*)(CTScalars))->SetArray((short*)CTDicomData, CTDicomDataSize, 1);
			break;
	}
	CTScalars->SetNumberOfComponents(1);


	vtkSmartPointer<vtkImageData> CTMPRCuboid = vtkSmartPointer<vtkImageData>::New();
	CTMPRCuboid->SetDimensions(CTDimension);
	CTMPRCuboid->SetSpacing(CTSpacing);
	//CTMPRCuboid->SetOrigin(CTImagePosition);
	CTMPRCuboid->GetPointData()->SetScalars(CTScalars);
	CTMPRCuboid->GetPointData()->GetScalars()->SetName("CT Cuboid");
	double CTBounds[6];
	CTMPRCuboid->GetBounds(CTBounds);

	int extent[6];
	CTMPRCuboid->GetExtent(extent);
	int origin[3] = { 0, 0, 0 };
	double center[3];
	center[0] = origin[0] + CTSpacing[0] * 0.5 * (extent[0] + extent[1]);
	center[1] = origin[1] + CTSpacing[1] * 0.5 * (extent[2] + extent[3]);
	center[2] = origin[2] + CTSpacing[2] * 0.5 * (extent[4] + extent[5]);


	// setup reslicer

	vtkSmartPointer<vtkMatrix4x4> resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
	resliceAxes->DeepCopy(axialElements);
	// Set the point through which to slice
	resliceAxes->SetElement(0, 3, center[0]);
	resliceAxes->SetElement(1, 3, center[1]);
	resliceAxes->SetElement(2, 3, center[2]);
	
	vtkSmartPointer<vtkImageReslice> CT_Reslice = vtkImageReslice::New();
	CT_Reslice->SetInputData(CTMPRCuboid);
	CT_Reslice->SetOutputDimensionality(2);
	CT_Reslice->SetOutputSpacing(CTSpacing);
	CT_Reslice->SetResliceAxes(resliceAxes);
	CT_Reslice->InterpolateOff();
	CT_Reslice->Update();


	//// Display the image
	////// Map the image through the lookup table
	vtkSmartPointer<vtkLookupTable> CT_Table = vtkSmartPointer<vtkLookupTable>::New();
	CT_Table->SetRange(0, 2000); // image intensity range
	CT_Table->SetValueRange(0.0, 1.0); // from black to white
	CT_Table->SetSaturationRange(0.0, 0.0); // no color saturation
	CT_Table->SetRampToLinear();
	CT_Table->Build();
	
	vtkSmartPointer<vtkImageMapToColors> CT_Color = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	CT_Color->SetInputData(CT_Reslice->GetOutput());
	//CT_Color->SetOutputFormatToLuminance();
	CT_Color->SetOutputFormatToLuminance();
	CT_Color->SetLookupTable(CT_Table);
	CT_Color->Update();

	vtkSmartPointer<vtkImageActor> CT_Actor = vtkSmartPointer<vtkImageActor>::New();
	CT_Actor->SetInputData(CT_Color->GetOutput());
	CT_Actor->Update();

	vtkSmartPointer<vtkOutlineFilter> CTOutline = vtkSmartPointer<vtkOutlineFilter>::New();
	CTOutline->SetInputData(CT_Color->GetOutput());
	CTOutline->Update();

	vtkSmartPointer<vtkPolyDataMapper> CTOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	CTOutlineMapper->SetInputData(CTOutline->GetOutput());
	CTOutlineMapper->Update();

	vtkSmartPointer<vtkActor> CTOutlineActor = vtkSmartPointer<vtkActor>::New();
	CTOutlineActor->SetMapper(CTOutlineMapper);
	CTOutlineActor->GetProperty()->SetEdgeVisibility(1);
	CTOutlineActor->GetProperty()->SetEdgeColor(255, 0, 0);

	//// Visualize
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

	//
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	//
	renderer->AddActor(CT_Actor);
	renderer->AddActor(CTOutlineActor);

	renderWindow->Render();

	// Set up the interaction
	vtkSmartPointer<vtkInteractorStyleImage> imageStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
	renderWindowInteractor->SetInteractorStyle(imageStyle);
	renderWindow->SetInteractor(renderWindowInteractor);
	renderWindow->Render();

	vtkSmartPointer<vtkImageInteractionCallback> callback = vtkSmartPointer<vtkImageInteractionCallback>::New();
	callback->SetImageReslice(CT_Reslice);
	callback->SetInteractor(renderWindowInteractor);

	imageStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);
	imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
	imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback);
	
	renderWindowInteractor->Start();


	return 0;
}

