// ITK-Registration.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkTranslationTransform.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImage.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"


#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCheckerBoardImageFilter.h"

#include "itkMetaImageIOFactory.h"
#include "itkJPEGImageIOFactory.h"

#include "itkCommand.h"

#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkNumericSeriesFileNames.h"

#include "gdcmUIDGenerator.h"

//#define MEDDIFF 1
template <typename TRegistration>
class RegistrationInterfaceCommand : public itk::Command
{
public:
	typedef  RegistrationInterfaceCommand   Self;
	typedef  itk::Command                   Superclass;
	typedef  itk::SmartPointer<Self>        Pointer;
	itkNewMacro(Self);

protected:
	RegistrationInterfaceCommand() {};

public:
	typedef   TRegistration                              RegistrationType;
	typedef   RegistrationType *                         RegistrationPointer;
	typedef   itk::RegularStepGradientDescentOptimizer   OptimizerType;
	typedef   OptimizerType *                            OptimizerPointer;
	void Execute(itk::Object * object, const itk::EventObject & event)
	{
		if (!(itk::IterationEvent().CheckEvent(&event)))
		{
			return;
		}
		RegistrationPointer registration =
			dynamic_cast<RegistrationPointer>(object);
		OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			registration->GetModifiableOptimizer());

		std::cout << "-------------------------------------" << std::endl;
		std::cout << "MultiResolution Level : "
			<< registration->GetCurrentLevel() << std::endl;
		std::cout << std::endl;

		if (registration->GetCurrentLevel() == 0)
		{
			optimizer->SetMaximumStepLength(16.00);
			optimizer->SetMinimumStepLength(0.01);
		}
		else
		{
			optimizer->SetMaximumStepLength(
				optimizer->GetMaximumStepLength() * 0.25);
			optimizer->SetMinimumStepLength(
				optimizer->GetMinimumStepLength() * 0.1);
		}
	}
	void Execute(const itk::Object *, const itk::EventObject &)
	{
		return;
	}
};

class CommandIterationUpdate : public itk::Command
{
public:
	typedef  CommandIterationUpdate   Self;
	typedef  itk::Command             Superclass;
	typedef  itk::SmartPointer<Self>  Pointer;
	itkNewMacro(Self);

protected:
	CommandIterationUpdate() {};

public:
	typedef   itk::RegularStepGradientDescentOptimizer  OptimizerType;
	typedef   const OptimizerType *                     OptimizerPointer;

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute((const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		OptimizerPointer optimizer =
			dynamic_cast< OptimizerPointer >(object);
		if (!(itk::IterationEvent().CheckEvent(&event)))
		{
			return;
		}
		std::cout << optimizer->GetCurrentIteration() << "   ";
		std::cout << optimizer->GetValue() << "   ";
		std::cout << optimizer->GetCurrentPosition() << std::endl;
	}
};


void CopyDictionary(itk::MetaDataDictionary &fromDict, itk::MetaDataDictionary &toDict)
{
	typedef itk::MetaDataDictionary DictionaryType;

	DictionaryType::ConstIterator itr = fromDict.Begin();
	DictionaryType::ConstIterator end = fromDict.End();
	typedef itk::MetaDataObject< std::string > MetaDataStringType;

	while (itr != end)
	{
		itk::MetaDataObjectBase::Pointer  entry = itr->second;

		MetaDataStringType::Pointer entryvalue =
			dynamic_cast<MetaDataStringType *>(entry.GetPointer());
		if (entryvalue)
		{
			std::string tagkey = itr->first;
			std::string tagvalue = entryvalue->GetMetaDataObjectValue();
			itk::EncapsulateMetaData<std::string>(toDict, tagkey, tagvalue);
		}
		++itr;
	}
}

int _tmain(int argc, char** argv)
{
	typedef std::list< itk::LightObject::Pointer > RegisteredObjectsContainerType;

	//itk::MetaImageIOFactory::RegisterOneFactory();
	itk::JPEGImageIOFactory::RegisterOneFactory();
	RegisteredObjectsContainerType registeredIOs =
		itk::ObjectFactoryBase::CreateAllInstance( "itkImageIOBase" );

	std::cout << registeredIOs.size()
		<< " IO objects available to the ImageFileReader.\n" << std::endl;
	

	const    unsigned int    Dimension = 2;
	typedef  unsigned short  PixelType;

#ifdef MEDDIFF
	const std::string fixedImageFile = "D:\\DicomDataSet\\PTCT\\Dataset1\\1.3.12.2.1107.5.1.4.1007.30000012032001413290600020180\\1.3.12.2.1107.5.1.4.1007.30000012032001413290600020493.dcm";
	const std::string movingImageFile = "D:\\DicomDataSet\\PTCT\\Dataset1\\1.3.12.2.1107.5.1.4.1007.30000012032001433800000011963\\1.3.12.2.1107.5.1.4.1007.30000012032001433800000011650.dcm";
#else
	const std::string fixedImageFile = "D:\\DicomDataSet\\PTCT\\Dataset1\\CT.jpeg";
	const std::string movingImageFile = "D:\\DicomDataSet\\PTCT\\Dataset1\\PT.jpeg";

#endif
	const int numberOfBins = 128;
	const int numberOfSamples = 5;

	typedef itk::Image< PixelType, Dimension >  FixedImageType;
	typedef itk::Image< PixelType, Dimension >  MovingImageType;

	typedef   float                                    InternalPixelType;
	typedef itk::Image< InternalPixelType, Dimension > InternalImageType;

	typedef itk::TranslationTransform< double, Dimension > TransformType;
	typedef itk::RegularStepGradientDescentOptimizer       OptimizerType;
	typedef itk::LinearInterpolateImageFunction<
		InternalImageType,
		double             > InterpolatorType;
	typedef itk::MattesMutualInformationImageToImageMetric<
		InternalImageType,
		InternalImageType >   MetricType;
	typedef itk::MultiResolutionImageRegistrationMethod<
		InternalImageType,
		InternalImageType >   RegistrationType;


	typedef itk::MultiResolutionPyramidImageFilter<
		InternalImageType, InternalImageType >   FixedImagePyramidType;
	typedef itk::MultiResolutionPyramidImageFilter<
		InternalImageType, InternalImageType >   MovingImagePyramidType;


	TransformType::Pointer      transform = TransformType::New();
	OptimizerType::Pointer      optimizer = OptimizerType::New();
	InterpolatorType::Pointer   interpolator = InterpolatorType::New();
	RegistrationType::Pointer   registration = RegistrationType::New();
	MetricType::Pointer         metric = MetricType::New();

	FixedImagePyramidType::Pointer fixedImagePyramid =
		FixedImagePyramidType::New();
	MovingImagePyramidType::Pointer movingImagePyramid =
		MovingImagePyramidType::New();

	registration->SetOptimizer(optimizer);
	registration->SetTransform(transform);
	registration->SetInterpolator(interpolator);
	registration->SetMetric(metric);
	registration->SetFixedImagePyramid(fixedImagePyramid);
	registration->SetMovingImagePyramid(movingImagePyramid);

	// read input dicom images
#ifdef MEDDIFF
	typedef itk::ImageFileReader< FixedImageType > FixedImageReaderType;
	typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;

	typedef itk::GDCMImageIO ImageIOType;
	typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
	typedef itk::NumericSeriesFileNames OutputNameGeneratorType;

	typedef itk::ImageSeriesWriter< MovingImageType, MovingImageType >             SeriesWriterType;

	ImageIOType::Pointer gdcmIO = ImageIOType::New();

#else
	typedef itk::ImageFileReader< FixedImageType  > FixedImageReaderType;
	typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;

#endif

	FixedImageReaderType::Pointer  fixedImageReader = FixedImageReaderType::New();
	MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

#ifdef MEDDIFF
	fixedImageReader->SetImageIO(gdcmIO);
	movingImageReader->SetImageIO(gdcmIO);

	fixedImageReader->SetFileName(fixedImageFile);
	movingImageReader->SetFileName(movingImageFile);
#else
	fixedImageReader->SetFileName(fixedImageFile);
	movingImageReader->SetFileName(movingImageFile);
#endif

	typedef itk::CastImageFilter<
		FixedImageType, InternalImageType >  FixedCastFilterType;
	typedef itk::CastImageFilter<
		MovingImageType, InternalImageType > MovingCastFilterType;

	FixedCastFilterType::Pointer fixedCaster = FixedCastFilterType::New();
	MovingCastFilterType::Pointer movingCaster = MovingCastFilterType::New();

	fixedCaster->SetInput(fixedImageReader->GetOutput());
	movingCaster->SetInput(movingImageReader->GetOutput());

	registration->SetFixedImage(fixedCaster->GetOutput());
	registration->SetMovingImage(movingCaster->GetOutput());

	fixedCaster->Update();

	registration->SetFixedImageRegion(
		fixedCaster->GetOutput()->GetBufferedRegion());


	typedef RegistrationType::ParametersType ParametersType;
	ParametersType initialParameters(transform->GetNumberOfParameters());

	initialParameters[0] = 0.0;  // Initial offset in mm along X
	initialParameters[1] = 0.0;  // Initial offset in mm along Y

	registration->SetInitialTransformParameters(initialParameters);

	metric->SetNumberOfHistogramBins(128);
	metric->SetNumberOfSpatialSamples(50000);

	metric->ReinitializeSeed(76926294);

	optimizer->SetNumberOfIterations(200);
	optimizer->SetRelaxationFactor(0.9);

	CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
	optimizer->AddObserver(itk::IterationEvent(), observer);

	typedef RegistrationInterfaceCommand<RegistrationType> CommandType;
	CommandType::Pointer command = CommandType::New();
	registration->AddObserver(itk::IterationEvent(), command);

	registration->SetNumberOfLevels(3); 

	try
	{
		registration->Update();
		std::cout << "Optimizer stop condition: "
			<< registration->GetOptimizer()->GetStopConditionDescription()
			<< std::endl;
	}
	catch (itk::ExceptionObject & err)
	{
		std::cout << "ExceptionObject caught !" << std::endl;
		std::cout << err << std::endl;
		return EXIT_FAILURE;
	}

	ParametersType finalParameters = registration->GetLastTransformParameters();

	double TranslationAlongX = finalParameters[0];
	double TranslationAlongY = finalParameters[1];

	unsigned int numberOfIterations = optimizer->GetCurrentIteration();

	double bestValue = optimizer->GetValue();


	// Print out results
	//
	std::cout << "Result = " << std::endl;
	std::cout << " Translation X = " << TranslationAlongX << std::endl;
	std::cout << " Translation Y = " << TranslationAlongY << std::endl;
	std::cout << " Iterations    = " << numberOfIterations << std::endl;
	std::cout << " Metric value  = " << bestValue << std::endl;

	typedef itk::ResampleImageFilter<MovingImageType,FixedImageType >    ResampleFilterType;

	typedef itk::LinearInterpolateImageFunction< MovingImageType, double >
		InterpolatorType2;

	InterpolatorType2::Pointer   interpolator2 = InterpolatorType2::New();

	typedef itk::ResampleImageFilter<
		MovingImageType,
		FixedImageType >    ResampleFilterType;

	MovingImageType::SizeType outputSize;
	outputSize.Fill(500);

	TransformType::Pointer finalTransform = TransformType::New();

	finalTransform->SetParameters(finalParameters);
	finalTransform->SetFixedParameters(transform->GetFixedParameters());

	ResampleFilterType::Pointer resample = ResampleFilterType::New();
	resample->SetInterpolator(interpolator2);
	resample->SetTransform(finalTransform);
	resample->SetInput(movingImageReader->GetOutput());

	FixedImageType::Pointer fixedImage = fixedImageReader->GetOutput();


	resample->SetSize(outputSize);
	resample->SetOutputOrigin(fixedImage->GetOrigin());
	resample->SetOutputSpacing(fixedImage->GetSpacing());
	resample->SetOutputDirection(fixedImage->GetDirection());
	resample->SetDefaultPixelValue(100);
	resample->Update();

#ifdef MEDDIFF
	// Copy the dictionary from the first image and override slice
	// specific fields
	itk::MetaDataDictionary inputDict = fixedImageReader->GetMetaDataDictionary();

	//ReaderType::DictionaryRawPointer inputDict = (*(reader->GetMetaDataDictionaryArray()))[0];
	//ReaderType::DictionaryArrayType outputArray;

	gdcm::UIDGenerator suid;
	std::string seriesUID = suid.Generate();
	gdcm::UIDGenerator fuid;
	std::string frameOfReferenceUID = fuid.Generate();


	std::string studyUID;
	std::string sopClassUID;
	itk::ExposeMetaData<std::string>(inputDict, "0020|000d", studyUID);
	itk::ExposeMetaData<std::string>(inputDict, "0008|0016", sopClassUID);
	gdcmIO->KeepOriginalUIDOn();

	itk::MetaDataDictionary dict;
	CopyDictionary(inputDict, dict);

	// Set the UID's for the study, series, SOP  and frame of reference
	itk::EncapsulateMetaData<std::string>(dict, "0020|000d", studyUID);
	itk::EncapsulateMetaData<std::string>(dict, "0020|000e", seriesUID);
	itk::EncapsulateMetaData<std::string>(dict, "0020|0052", frameOfReferenceUID);

	gdcm::UIDGenerator sopuid;
	std::string sopInstanceUID = sopuid.Generate();

	itk::EncapsulateMetaData<std::string>(dict, "0008|0018", sopInstanceUID);
	itk::EncapsulateMetaData<std::string>(dict, "0002|0003", sopInstanceUID);

	// 4) Shift data to undo the effect of a rescale intercept by the
	//    DICOM reader
	std::string interceptTag("0028|1052");
	typedef itk::MetaDataObject< std::string > MetaDataStringType;
	itk::MetaDataObjectBase::Pointer entry = (inputDict)[interceptTag];

	MetaDataStringType::ConstPointer interceptValue =
		dynamic_cast<const MetaDataStringType *>(entry.GetPointer());

	int interceptShift = 0;
	if (interceptValue)
	{
		std::string tagValue = interceptValue->GetMetaDataObjectValue();
		interceptShift = -atoi(tagValue.c_str());
	}

	typedef itk::ShiftScaleImageFilter< MovingImageType, FixedImageType >
		ShiftScaleType;

	ShiftScaleType::Pointer shiftScale = ShiftScaleType::New();
	shiftScale->SetInput(resample->GetOutput());
	shiftScale->SetShift(interceptShift);

	// 5) Write the new DICOM series

	// Make the output directory and generate the file names.
	itksys::SystemTools::MakeDirectory("D:\\DicomDataSet\\PTCT\\Dataset1\\Test_PT_RESAMPLED");

	// Generate the file names
	itk::NumericSeriesFileNames::Pointer outputNames = itk::NumericSeriesFileNames::New();
	std::string seriesFormat("D:\\DicomDataSet\\PTCT\\Dataset1\\Test_PT_RESAMPLED");
	seriesFormat = seriesFormat + "/" + "IM%d.dcm";
	outputNames->SetSeriesFormat(seriesFormat.c_str());
	outputNames->SetStartIndex(1);
	outputNames->SetEndIndex(1);

	SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();
	seriesWriter->SetInput(shiftScale->GetOutput());
	seriesWriter->SetImageIO(gdcmIO);
	seriesWriter->SetFileNames(outputNames->GetFileNames());
	seriesWriter->SetMetaDataDictionary(dict);
	try
	{
		seriesWriter->Update();
	}
	catch (itk::ExceptionObject & excp)
	{
		std::cerr << "Exception thrown while writing the series " << std::endl;
		std::cerr << excp << std::endl;
		return EXIT_FAILURE;
	}
#else
	typedef  unsigned char  OutputPixelType;

	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

	typedef itk::CastImageFilter<
		FixedImageType,
		OutputImageType > CastFilterType;

	typedef itk::ImageFileWriter< OutputImageType >  WriterType;


	WriterType::Pointer      writer = WriterType::New();
	CastFilterType::Pointer  caster = CastFilterType::New();


	writer->SetFileName("D:\\DicomDataSet\\PTCT\\Dataset1\\Registered.jpg");


	caster->SetInput(resample->GetOutput());
	writer->SetInput(caster->GetOutput());
	writer->Update();

	typedef itk::CheckerBoardImageFilter< FixedImageType > CheckerBoardFilterType;

	CheckerBoardFilterType::Pointer checker = CheckerBoardFilterType::New();

	checker->SetInput1(fixedImage);
	checker->SetInput2(resample->GetOutput());

	caster->SetInput(checker->GetOutput());
	writer->SetInput(caster->GetOutput());

	resample->SetDefaultPixelValue(0);

	// Before registration
	TransformType::Pointer identityTransform = TransformType::New();
	identityTransform->SetIdentity();
	resample->SetTransform(identityTransform);

	writer->SetFileName("D:\\DicomDataSet\\PTCT\\Dataset1\\Registered_Before.jpg");
	writer->Update();

	// After registration
	resample->SetTransform(finalTransform);
	writer->SetFileName("D:\\DicomDataSet\\PTCT\\Dataset1\\Registered_After.jpg");
	writer->Update();

#endif
	return 0;
}

