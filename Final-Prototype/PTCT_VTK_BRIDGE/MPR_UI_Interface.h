// MPR_VTK_BRIDGE.h

#pragma once

using namespace System;
#include "MPR.h"
#include "SUVComputation.h"

using namespace radspeed;
using namespace ImageUtils;

namespace PTCT_VTK_BRIDGE {

	public ref class MPR_UI_Interface
	{
		
	private: // members
		static MPR_UI_Interface^ m_handle;
		MPR* m_mpr;
		MPR* m_pt_mpr;
		SUVComputation* m_suv_computation;
		PET^ m_pet;
		
		BitmapWrapper^ ct_bmp;
		BitmapWrapper^ pt_bmp;
		BitmapWrapper^ pt_lut_bmp;
		double m_translateX;
		double m_translateY;
	public: // methods
		static MPR_UI_Interface^ GetHandle();
		~MPR_UI_Interface(void);
		void InitMPR(String^ path);
		void Init_PT_MPR(String^ path);

		BitmapWrapper^ GetDisplayImage(int axis);
		BitmapWrapper^ GetPTDisplayImage(int axis, bool applyLut);
		int GetNumberOfImages(int axis);
		void Scroll(int axis, int delta);
		int GetCurrentImageIndex(int axis);
		double GetCurrentImagePosition(int axis);
		void GetCurrentSlicerPositionRelativeToIndex(int axis, double* pos);
		void UpdateSlicerPosition(int axis, float x, float y);
		String^ GetOrientationMarkerLeft(int axis);
		String^ GetOrientationMarkerRight(int axis);
		String^ GetOrientationMarkerTop(int axis);
		String^ GetOrientationMarkerBottom(int axis);

		void InitDisplay(int axis);
		void RaiseSlicerPositionUpdate(int axis);
		long int GetPixelIntensity(int axis, int pos_x, int pos_y);
		// PT-CT Winform2 porject calls.

		void LoadImages(String^ ctImage, String^ ptImage);
		BitmapWrapper^ GetCTDisplay() { return ct_bmp; }
		BitmapWrapper^ GetPTDisplay() { return pt_bmp; }
		BitmapWrapper^ GetPT_LUTDisplay() { return pt_lut_bmp; }
		double GetTranslateX(){ return m_translateX; }
		double GetTranslateY(){ return m_translateY; }

	private:
		void UpdateDisplay(int axis, bool applyLUT);

	//public: //delegates
	//	delegate void CursorTranslationCompleted();
	//	delegate void UpdateImage(BitmapWrapper^ bmpWrapper, BitmapWrapper^ ptBmpWrapper, int axis, double reslicerPositionX, double reslicerPositionY);

	//	delegate void UpdateFusedImage(BitmapWrapper^ bmpWrapper, BitmapWrapper^ ptBmpWrapper);

	public: //static methods
		static void WriteLog(String^ msg);
	//public: // event implementation
	//	event CursorTranslationCompleted^ EVT_CursorTranslationCompleted
	//	{
	//		void add(CursorTranslationCompleted^ p){ m_cursorTranslationCompleted += p; }
	//		void remove(CursorTranslationCompleted^ p){ m_cursorTranslationCompleted -= p; }
	//		void raise()
	//		{
	//			if (m_cursorTranslationCompleted != nullptr)
	//			{
	//				m_cursorTranslationCompleted();
	//			}
	//		}
	//	}

	//	event UpdateImage^ EVT_UpdateImage
	//	{
	//		void add(UpdateImage^ p){ m_updateImage += p; }
	//		void remove(UpdateImage^ p){ m_updateImage -= p; }
	//		void raise(BitmapWrapper^ bmp, BitmapWrapper^ pt_bmp, int axis, double reslicerPositionX, double reslicerPositionY)
	//		{
	//			if (m_updateImage != nullptr)
	//			{
	//				m_updateImage(bmp, pt_bmp, axis, reslicerPositionX, reslicerPositionY);
	//			}
	//		}
	//	}

	//	event UpdateFusedImage^ EVT_UpdateFusedImage
	//	{
	//		void add(UpdateFusedImage^ p){ m_updateFusedImage += p; }
	//		void remove(UpdateFusedImage^ p){ m_updateFusedImage -= p; }
	//		void raise(BitmapWrapper^ bmp, BitmapWrapper^ pt_bmp)
	//		{
	//			if (m_updateFusedImage != nullptr)
	//			{
	//				m_updateFusedImage(bmp, pt_bmp);
	//			}
	//		}
	//	}
	//private: //events
	//	CursorTranslationCompleted^ m_cursorTranslationCompleted;
	//	UpdateImage^ m_updateImage;
	//	UpdateFusedImage^ m_updateFusedImage;
	protected:
		MPR_UI_Interface(void);
	};
}
