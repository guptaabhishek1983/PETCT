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
	rad_setLogLevel(255);
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
	this->m_pet->SetupLookupTable("D:\\GIT-HUB\\Repo1\\PT-CT\\Final-Prototype\\LUT\\HotIron.xml");

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
	int newWidth, newHeight;
	this->m_mpr->GetOutputImageDisplayDimensions((Axis)axis, newWidth, newHeight);

	BitmapWrapper^ bmp = gcnew BitmapWrapper(displayImage.data, displayImage.width, displayImage.height, "MONOCHROME");
	//bmp->Resize(newWidth, newHeight);
	return bmp;

}
BitmapWrapper^ MPR_UI_Interface::GetPTDisplayImage(int axis, bool applyLut, double^ tx, double^ ty)
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
	int newWidth, newHeight;
	this->m_pt_mpr->GetOutputImageDisplayDimensions((Axis)axis, newWidth, newHeight);

	//bmp->Resize(newWidth, newHeight);

	// find translation
	double x, y, z;
	this->m_mpr->GetOrigin(x,y,z);

	double x1, y1, z1;
	this->m_pt_mpr->GetOrigin(x1, y1, z1);
	
	switch (axis)
	{
		case AxialAxis:
		{
			// translate the point to origin
			tx = fabs(x - x1);
			ty = fabs(y - y1);
		}
			break;

		case SagittalAxis:
		{
			tx = fabs(y- y1);
			ty = fabs(z- z1);
		}
			break;

		case CoronalAxis:
		{
			tx = fabs(x - x1);
			ty = fabs(z - z1);
		}
			break;
	}

	return bmp;
}

int MPR_UI_Interface::GetNumberOfImages(int axis)
{
	return this->m_mpr->GetNumberOfImages((Axis)axis);
}
void MPR_UI_Interface::Scroll(int axis, int delta)
{
	this->m_mpr->Scroll((Axis)axis, delta);
}

int MPR_UI_Interface::GetCurrentImageIndex(int axis)
{
	return this->m_mpr->GetCurrentImageIndex((Axis)axis);
}

double MPR_UI_Interface::GetCurrentImagePosition(int axis)
{
	return this->m_mpr->GetCurrentImagePosition((Axis)axis);
}

double MPR_UI_Interface::GetCurrentImagePositionRelativeToOrigin(int axis)
{
	return this->m_mpr->GetCurrentImagePositionRelativeToOrigin((Axis)axis);
}

void MPR_UI_Interface::UpdateSlicerPosition(int axis, float x, float y)
{
	switch ((Axis)axis)
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
	}
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
	image ct_display = ::born_image();
	ct_display.height = ctDicom->Get_ROW();
	ct_display.width = ctDicom->Get_COLOUMN();
	ct_display.size = ct_display.height* ct_display.width;
	ct_display.type = TYPE_U8Data;
	ct_display.data = rad_get_memory(ct_display.height * ct_display.width *rad_sizeof(ct_display.type));

	::GetDisplayImage(ctDicom, ct_display);

	this->ct_bmp = gcnew BitmapWrapper(ct_display.data, ct_display.width, ct_display.height, "MONOCHROME");

	this->ct_bmp->Resize(ct_display.width* 0.9765625, ct_display.height* 0.9765625);
	
	RadRTDicomInterface* ptDicom = new RTDcmtkDicomInterface(pt_file);
	image pt_display = ::born_image();
	pt_display.height = ptDicom->Get_ROW();
	pt_display.width = ptDicom->Get_COLOUMN();
	pt_display.size = pt_display.height* pt_display.width;
	pt_display.type = TYPE_U8Data;
	pt_display.data = rad_get_memory(pt_display.height * pt_display.width *rad_sizeof(pt_display.type));

	::GetDisplayImage(ptDicom, pt_display);

	this->pt_bmp = gcnew BitmapWrapper(pt_display.data, pt_display.width, pt_display.height, "MONOCHROME");
	this->pt_bmp->Resize(ct_display.width* 0.9765625, ct_display.height* 0.9765625);
	this->m_pet = gcnew PET();
	this->m_pet->SetupLookupTable("D:\\GIT-HUB\\Repo1\\PT-CT\\Final-Prototype\\LUT\\HotIron.xml");

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
	this->pt_lut_bmp->Resize(ct_display.width* 0.9765625, ct_display.height* 0.9765625);
	this->pt_lut_bmp->ChangeImageOpacity(0.7);
	rad_free_memory(r1);
	rad_free_memory(g1);
	rad_free_memory(b1);
	die_image(_rgb_display_image);
	die_image(pt_display);
	die_image(ct_display);

	return;
}

// static methods
void MPR_UI_Interface::WriteLog(String^ msg)
{
	const char* m = convert_to_const_charPtr(msg);
	RAD_LOG_INFO("UI:"<<m);
}