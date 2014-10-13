#include "Stdafx.h"
#include "MPR_UI_Interface.h"
#include "Utility_Functions.h"

// meddiff includes
#include "rad_util.h"
#include "rad_logger.h"
#include "streaming-image.h"

// VTK icludes
#include "vtkImageData.h"
#include "vtkSmartPointer.h"

using namespace System;
using namespace System::IO;
using namespace System::Xml;

using namespace PTCT_VTK_BRIDGE;
using namespace ImageUtils;
using namespace EventDelegate_BRIDGE;

#define __FILENAME__ "MPR_UI_Interface.cpp"
#undef __MODULENAME__
#define __MODULENAME__ "MPR_UI_Interface"

MPR_UI_Interface::MPR_UI_Interface()
{

	if (File::Exists("C:\\Temp\\MPR_View.log"))
	{
		File::Delete("C:\\Temp\\MPR_View.log");
	}
	rad_setLogFileName("C:\\Temp\\MPR_View.log");
	rad_setLogLevel(7);
}

MPR_UI_Interface^ MPR_UI_Interface::GetHandle()
{
	if (m_handle == nullptr)
	{
		m_handle = gcnew MPR_UI_Interface();
	}
	return m_handle;
}
MPR_UI_Interface::~MPR_UI_Interface(void)
{
}

void MPR_UI_Interface::InitMPR(String^ path)
{
	this->m_mpr = new MPR();

	// find all dcm files.
	array<String^>^ dirs = Directory::GetFiles(path, "*.dcm");
	vector<string> fileNames;
	for (int i = 0; i < dirs->Length; i++)
	{
		String^ _strFile = safe_cast<String^>(dirs[i]);
		const char* dicomFile = convert_to_const_charPtr(_strFile);
		fileNames.push_back(dicomFile);
	}
	this->m_mpr->initFromDir1(fileNames);
}

void MPR_UI_Interface::Init_PT_MPR(String^ path)
{
	this->m_pet = gcnew PET();
	this->m_pet->SetupLookupTable("D:\\GIT-HUB\\PETCT\\Final-Prototype\\LUT\\HotIron.xml");

	this->m_pt_mpr = new MPR();

	// find all dcm files.
	array<String^>^ dirs = Directory::GetFiles(path, "*.dcm");
	vector<string> fileNames;
	for (int i = 0; i < dirs->Length; i++)
	{
		String^ _strFile = safe_cast<String^>(dirs[i]);
		const char* dicomFile = convert_to_const_charPtr(_strFile);
		fileNames.push_back(dicomFile);
	}
	this->m_pt_mpr->initFromDir1(fileNames);
	
}

BitmapWrapper^ MPR_UI_Interface::GetDisplayImage(int axis)
{
	image displayImage = this->m_mpr->GetOutputImage((Axis)axis);

	BitmapWrapper^ bmp = gcnew BitmapWrapper(displayImage.data, displayImage.width, displayImage.height, "MONOCHROME");
	return bmp;

}
BitmapWrapper^ MPR_UI_Interface::GetPTDisplayImage(int axis, bool applyLut)
{

	image displayImage = this->m_pt_mpr->GetOutputImage((Axis)axis);
	
	BitmapWrapper^ bmp = nullptr;
	// Apply LUT
	if (applyLut)
	{
		U8Data r1 = (U8Data)rad_get_memory(displayImage.size);
		U8Data g1 = (U8Data)rad_get_memory(displayImage.size);
		U8Data b1 = (U8Data)rad_get_memory(displayImage.size);

		for (int i = 0; i < displayImage.size; i++)
		{
			int r, g, b;
			this->m_pet->GetLookupValue(((U8Data)displayImage.data)[i], r, g, b);
			r1[i] = r;
			g1[i] = g;
			b1[i] = b;
		}

		image _rgb_display_image = ::born_image();
		_rgb_display_image.height = displayImage.height;
		_rgb_display_image.width = displayImage.width;
		_rgb_display_image.size = displayImage.size;
		_rgb_display_image.data = rad_get_memory(displayImage.size*rad_sizeof(TYPE_U32Data));

		unsigned char*p = NULL;

		for (u_int i = 0; i<displayImage.size; i++)
		{
			p = (unsigned char*)((U32Data)_rgb_display_image.data + i);
			*p = b1[i];
			p++;
			*p = g1[i];
			p++;
			*p = r1[i];
			p++;
			*p = 125;
		}

		//decode_rgb_to_argb(r1, g1, b1, (U32Data)_rgb_display_image.data, displayImage.size);

		bmp = gcnew BitmapWrapper(_rgb_display_image.data, _rgb_display_image.width, _rgb_display_image.height, "RGB");
		bmp->ChangeImageOpacity(0.7);
		rad_free_memory(r1);
		rad_free_memory(g1);
		rad_free_memory(b1);
		die_image(_rgb_display_image);
	}
	else
	{
		bmp = gcnew BitmapWrapper(displayImage.data, displayImage.width, displayImage.height, "MONOCHROME");
	}
	return bmp;
}

int MPR_UI_Interface::GetNumberOfImages(int axis)
{
	return this->m_mpr->GetNumberOfImages((Axis)axis);
}

void MPR_UI_Interface::RaiseSlicerPositionUpdate(int axis)
{
	double pos[2] = { 0, 0 };
	GetCurrentSlicerPositionRelativeToIndex(axis, pos);
	EventDelegate_BRIDGE::EDB::Instance->RaiseUpdateCursorPos(axis, pos[0], pos[1]);
}
void MPR_UI_Interface::Scroll(int axis, int delta)
{
	this->m_mpr->Scroll((Axis)axis, delta);

	double ct_spacing[3] = { 0, 0, 0 };
	this->m_mpr->GetInput()->GetSpacing(ct_spacing);
	double ct_pos = this->m_mpr->GetCurrentImagePosition((Axis)axis);
	double pt_translate = delta*ct_spacing[2];
	if (axis == SagittalAxis)
	{
		pt_translate = delta*ct_spacing[0];
	}
	if (axis == CoronalAxis)
	{
		pt_translate = delta*ct_spacing[1];

	}
	this->m_pt_mpr->Scroll((Axis)axis, pt_translate);
	/*double ct_pos = this->m_mpr->GetCurrentImagePosition((Axis)axis);
	double pt_pos = this->m_pt_mpr->GetCurrentImagePosition((Axis)axis);
	double pt_spacing[3] = { 0, 0, 0 };
	this->m_pt_mpr->GetInput()->GetSpacing(pt_spacing);
	switch (axis)
	{
		case Axis::AxialAxis:
		{
			if ((fabs(ct_pos) - fabs(pt_pos)) > fabs(pt_spacing[2]))
			{
				this->m_pt_mpr->Scroll((Axis)axis, delta);
			}
		}
			break;
		case SagittalAxis:
		{
			if ((fabs(ct_pos) - fabs(pt_pos)) > fabs(pt_spacing[0]))
			{
				this->m_pt_mpr->Scroll((Axis)axis, delta);
			}
		}
		case CoronalAxis:
		{
			if ((fabs(ct_pos) - fabs(pt_pos)) > fabs(pt_spacing[1]))
			{
				this->m_pt_mpr->Scroll((Axis)axis, delta);
			}
		}
		default:
			break;
	}*/
	switch (axis)
	{
		case Axis::AxialAxis:
		{
			UpdateDisplay((int)AxialAxis, true);
			RaiseSlicerPositionUpdate(SagittalAxis);
			RaiseSlicerPositionUpdate(CoronalAxis);
		}
			break;
		case SagittalAxis:
		{
			UpdateDisplay((int)SagittalAxis, true);
			RaiseSlicerPositionUpdate(AxialAxis);
			RaiseSlicerPositionUpdate(CoronalAxis);
		}
			break;

		case CoronalAxis:
		{
			UpdateDisplay((int)CoronalAxis, true);
			RaiseSlicerPositionUpdate(AxialAxis);
			RaiseSlicerPositionUpdate(SagittalAxis);
		}
			break;
		default:
			break;
	}
	
}

int MPR_UI_Interface::GetCurrentImageIndex(int axis)
{
	return this->m_mpr->GetCurrentImageIndex((Axis)axis);
}

double MPR_UI_Interface::GetCurrentImagePosition(int axis)
{
	return this->m_mpr->GetCurrentImagePosition((Axis)axis);
}

void MPR_UI_Interface::GetCurrentSlicerPositionRelativeToIndex(int axis, double* pos)
{
	double xPos = 0, yPos = 0;
	this->m_mpr->GetCurrentSlicerPositionRelativeToIndex((Axis)axis, xPos, yPos);
	pos[0] = xPos;
	pos[1] = yPos;
}

//TODO: Fix me
void MPR_UI_Interface::UpdateSlicerPosition(int axis, float x, float y)
{
	/*switch ((Axis)axis)
	{
		case AxialAxis:
		{
			this->m_mpr->Scroll2(Axis::SagittalAxis, x);
			this->m_mpr->Scroll2(Axis::CoronalAxis, y);

			BitmapWrapper^ sagittal_bmp = GetDisplayImage((int)Axis::SagittalAxis);
			BitmapWrapper^ coronal_bmp = GetDisplayImage((int)Axis::CoronalAxis);
			if (m_updateImage != nullptr)
			{
				EVT_UpdateImage(sagittal_bmp, nullptr, Axis::SagittalAxis, 0, 0);
				EVT_UpdateImage(coronal_bmp, nullptr, Axis::CoronalAxis, 0, 0);
			}
		}
			break;
		case SagittalAxis:
		{
			this->m_mpr->Scroll2(Axis::CoronalAxis, x);
			this->m_mpr->Scroll2(Axis::AxialAxis, y);

			BitmapWrapper^ axial_bmp = GetDisplayImage((int)Axis::AxialAxis);
			BitmapWrapper^ coronal_bmp = GetDisplayImage((int)Axis::CoronalAxis);
			if (m_updateImage != nullptr)
			{
				EVT_UpdateImage(axial_bmp, nullptr, Axis::AxialAxis, 0, 0);
				EVT_UpdateImage(coronal_bmp, nullptr, Axis::CoronalAxis, 0, 0);
			}
		}
			break;
		case CoronalAxis:
		{
			this->m_mpr->Scroll2(Axis::SagittalAxis , x);
			this->m_mpr->Scroll2(Axis::AxialAxis, y);

			BitmapWrapper^ sagittal_bmp = GetDisplayImage((int)Axis::SagittalAxis);
			BitmapWrapper^ axial_bmp = GetDisplayImage((int)Axis::AxialAxis);
			if (m_updateImage != nullptr)
			{
				EVT_UpdateImage(sagittal_bmp, nullptr, Axis::SagittalAxis, 0, 0);
				EVT_UpdateImage(axial_bmp, nullptr, Axis::AxialAxis, 0, 0);
			}
		}
			break;
		default:
			break;
	}*/
}

String^ MPR_UI_Interface::GetOrientationMarkerLeft(int axis)
{
	return convert_to_managed_string(this->m_mpr->GetOrientationMarkerLeft((Axis)axis));
}
String^ MPR_UI_Interface::GetOrientationMarkerRight(int axis)
{
	return convert_to_managed_string(this->m_mpr->GetOrientationMarkerRight((Axis)axis));
}
String^ MPR_UI_Interface::GetOrientationMarkerTop(int axis)
{
	return convert_to_managed_string(this->m_mpr->GetOrientationMarkerTop((Axis)axis));
}
String^ MPR_UI_Interface::GetOrientationMarkerBottom(int axis)
{
	return convert_to_managed_string(this->m_mpr->GetOrientationMarkerBottom((Axis)axis));
}

void MPR_UI_Interface::UpdateDisplay(int axis, bool applyLut)
{
	image ct_displayImage = this->m_mpr->GetOutputImage((Axis)axis);

	BitmapWrapper^ ct_bmp = gcnew BitmapWrapper(ct_displayImage.data, ct_displayImage.width, ct_displayImage.height, "MONOCHROME");


	image pt_displayImage = this->m_pt_mpr->GetOutputImage((Axis)axis);

	BitmapWrapper^ pt_bmp = nullptr;
	// Apply LUT
	if (applyLut)
	{
		U8Data r1 = (U8Data)rad_get_memory(pt_displayImage.size);
		U8Data g1 = (U8Data)rad_get_memory(pt_displayImage.size);
		U8Data b1 = (U8Data)rad_get_memory(pt_displayImage.size);

		for (int i = 0; i < pt_displayImage.size; i++)
		{
			int r, g, b;
			this->m_pet->GetLookupValue(((U8Data)pt_displayImage.data)[i], r, g, b);
			r1[i] = r;
			g1[i] = g;
			b1[i] = b;
		}

		image _rgb_display_image = ::born_image();
		_rgb_display_image.height = pt_displayImage.height;
		_rgb_display_image.width = pt_displayImage.width;
		_rgb_display_image.size = pt_displayImage.size;
		_rgb_display_image.data = rad_get_memory(pt_displayImage.size*rad_sizeof(TYPE_U32Data));

		unsigned char*p = NULL;

		for (u_int i = 0; i<pt_displayImage.size; i++)
		{
			p = (unsigned char*)((U32Data)_rgb_display_image.data + i);
			*p = b1[i];
			p++;
			*p = g1[i];
			p++;
			*p = r1[i];
			p++;
			*p = 125;
		}

		//decode_rgb_to_argb(r1, g1, b1, (U32Data)_rgb_display_image.data, displayImage.size);

		pt_bmp = gcnew BitmapWrapper(_rgb_display_image.data, _rgb_display_image.width, _rgb_display_image.height, "RGB");
		pt_bmp->ChangeImageOpacity(0.7);
		rad_free_memory(r1);
		rad_free_memory(g1);
		rad_free_memory(b1);
		die_image(_rgb_display_image);
	}
	else
	{
		pt_bmp = gcnew BitmapWrapper(pt_displayImage.data, pt_displayImage.width, pt_displayImage.height, "MONOCHROME");
	}

	// resize
	vtkSmartPointer<vtkImageData> ct_cuboid = this->m_mpr->GetInput();
	double ct_spacing[3] = { 0, 0, 0 };
	ct_cuboid->GetSpacing(ct_spacing);

	double ct_origin[3] = { 0, 0, 0 };
	ct_cuboid->GetOrigin(ct_origin);

	vtkSmartPointer<vtkImageData> pt_cuboid = this->m_pt_mpr->GetInput();
	double pt_spacing[3] = { 0, 0, 0 };
	pt_cuboid->GetSpacing(pt_spacing);

	double pt_origin[3] = { 0, 0, 0 };
	pt_cuboid->GetOrigin(pt_origin);

	pt_bmp->Resize(pt_displayImage .width *(pt_spacing[0] / ct_spacing[0]),
		pt_displayImage.height * (pt_spacing[1] / ct_spacing[1]));


	double translateX = (ct_origin[0] - pt_origin[0]) / ct_spacing[0];
	double translateY = (ct_origin[1] - pt_origin[1]) / ct_spacing[1];

	double ct_pos = this->m_mpr->GetCurrentImagePosition((Axis)axis);
	double pt_pos = this->m_pt_mpr->GetCurrentImagePosition((Axis)axis);

	EventDelegate_BRIDGE::EDB::Instance->RaiseUpdatePTCTImage(axis, ct_bmp, pt_bmp, translateX, translateY, ct_pos, pt_pos);

	return;

}
void MPR_UI_Interface::InitDisplay(int axis)
{
	UpdateDisplay(axis, true);
}
// meddiff includes
#include "rad_util.h"
#include "rad_logger.h"
#include "RTDcmtkDicomInterface.h"
#include "rad_template_helper.h"

void GetDisplayImage(RadRTDicomInterface* pDicom, image& displayImage)
{
	
	RadDataType dicomDataType = RadDataType::TYPE_NOT_SET;

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

	image pixelData = ::born_image();
	pixelData.height = pDicom->Get_ROW();
	pixelData.width = pDicom->Get_COLOUMN();
	pixelData.type = dicomDataType;

	image overlayPixelData = ::born_image();
	pDicom->InflateSingleFrameDicomPixelData(&pixelData, &overlayPixelData);

	voi_lut_transform_image_fast(displayImage, pixelData,
		convert_to_int(pDicom->Get_WINDOW_WIDTH()),
		convert_to_int(pDicom->Get_WINDOW_CENTER()),
		0, 255,
		convert_to_int(pDicom->Get_RESCALE_SLOPE()),
		convert_to_int(pDicom->Get_RESCALE_INTERCEPT()));
	die_image(pixelData);

	return;
}

void MPR_UI_Interface::LoadImages(String^ ctImage, String^ ptImage)
{
	const char* ct_file = convert_to_const_charPtr(ctImage);
	const char* pt_file = convert_to_const_charPtr(ptImage);

	RadRTDicomInterface* ctDicom = new RTDcmtkDicomInterface(ct_file);
	double ct_spacing[2] = { 0, 0 };
	vector<string> ctPixelSpacing;
	tokenize(ctDicom->Get_PIXEL_SPACING(), ctPixelSpacing, "\\");
	ct_spacing[0] = convert_to_double(ctPixelSpacing[0].c_str());
	ct_spacing[1] = convert_to_double(ctPixelSpacing[1].c_str());
	double ctImagePosition[6] = { 0, 0, 0, 0, 0, 0 };

	{
		string imagePosition = string(ctDicom->Get_IMAGE_POSITION());
		vector<string> _imgPosition;
		tokenize(imagePosition, _imgPosition, "\\", true);

		for (int i = 0; i < _imgPosition.size(); i++)
		{
			ctImagePosition[i] = convert_to_double(_imgPosition.at(i).c_str());
		}
	}

	image ct_display = ::born_image();
	ct_display.height = ctDicom->Get_ROW();
	ct_display.width = ctDicom->Get_COLOUMN();
	ct_display.size = ct_display.height* ct_display.width;
	ct_display.type = TYPE_U8Data;
	ct_display.data = rad_get_memory(ct_display.height * ct_display.width *rad_sizeof(ct_display.type));

	::GetDisplayImage(ctDicom, ct_display);

	this->ct_bmp = gcnew BitmapWrapper(ct_display.data, ct_display.width, ct_display.height, "MONOCHROME");

	//this->ct_bmp->Resize(ct_display.width* 0.9765625, ct_display.height* 0.9765625);
	
	RadRTDicomInterface* ptDicom = new RTDcmtkDicomInterface(pt_file);
	double pt_spacing[2] = { 0, 0 };
	vector<string> ptPixelSpacing;
	tokenize(ptDicom->Get_PIXEL_SPACING(), ptPixelSpacing, "\\");
	pt_spacing[0] = convert_to_double(ptPixelSpacing[0].c_str());
	pt_spacing[1] = convert_to_double(ptPixelSpacing[1].c_str());

	double ptImagePosition[6] = { 0, 0, 0, 0, 0, 0 };

	{
		string imagePosition = string(ptDicom->Get_IMAGE_POSITION());
		vector<string> _imgPosition;
		tokenize(imagePosition, _imgPosition, "\\", true);

		for (int i = 0; i < _imgPosition.size(); i++)
		{
			ptImagePosition[i] = convert_to_double(_imgPosition.at(i).c_str());
		}
	}

	image pt_display = ::born_image();
	pt_display.height = ptDicom->Get_ROW();
	pt_display.width = ptDicom->Get_COLOUMN();
	pt_display.size = pt_display.height* pt_display.width;
	pt_display.type = TYPE_U8Data;
	pt_display.data = rad_get_memory(pt_display.height * pt_display.width *rad_sizeof(pt_display.type));

	::GetDisplayImage(ptDicom, pt_display);

	this->pt_bmp = gcnew BitmapWrapper(pt_display.data, pt_display.width, pt_display.height, "MONOCHROME");
	//this->pt_bmp->Resize(pt_display.width* 3.3, pt_display.height* 3.3);
	this->m_pet = gcnew PET();
	this->m_pet->SetupLookupTable("D:\\GIT-HUB\\PETCT\\Final-Prototype\\LUT\\HotIron.xml");

	U8Data r1 = (U8Data)rad_get_memory(pt_display.size);
	U8Data g1 = (U8Data)rad_get_memory(pt_display.size);
	U8Data b1 = (U8Data)rad_get_memory(pt_display.size);

	for (int i = 0; i < pt_display.size; i++)
	{
		int r, g, b;
		this->m_pet->GetLookupValue(((U8Data)pt_display.data)[i], r, g, b);
		r1[i] = r;
		g1[i] = g;
		b1[i] = b;
	}

	image _rgb_display_image = ::born_image();
	_rgb_display_image.height = pt_display.height;
	_rgb_display_image.width = pt_display.width;
	_rgb_display_image.size = pt_display.size;
	_rgb_display_image.data = rad_get_memory(pt_display.size*rad_sizeof(TYPE_U32Data));

	unsigned char*p = NULL;

	for (u_int i = 0; i<pt_display.size; i++)
	{
		p = (unsigned char*)((U32Data)_rgb_display_image.data + i);
		*p = b1[i];
		p++;
		*p = g1[i];
		p++;
		*p = r1[i];
		p++;
		*p = 125;
	}

	//decode_rgb_to_argb(r1, g1, b1, (U32Data)_rgb_display_image.data, displayImage.size);

	this->pt_lut_bmp = gcnew BitmapWrapper(_rgb_display_image.data, _rgb_display_image.width, _rgb_display_image.height, "RGB");

	this->pt_lut_bmp->Resize(_rgb_display_image.width * (pt_spacing[0] / ct_spacing[0]) ,
		_rgb_display_image.height * (pt_spacing[1] / ct_spacing[1]) );

	this->pt_lut_bmp->ChangeImageOpacity(0.7);
	rad_free_memory(r1);
	rad_free_memory(g1);
	rad_free_memory(b1);
	die_image(_rgb_display_image);
	die_image(pt_display);
	die_image(ct_display);

	this->m_translateX = (ctImagePosition[0] - ptImagePosition[0]) / ct_spacing[0];
	this->m_translateY = (ctImagePosition[1] - ptImagePosition[1]) / ct_spacing[1];
	return;
}

// static methods
void MPR_UI_Interface::WriteLog(String^ msg)
{
	const char* m = convert_to_const_charPtr(msg);
	RAD_LOG_INFO("UI:"<<m);
}