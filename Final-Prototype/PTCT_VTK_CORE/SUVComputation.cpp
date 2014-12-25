#include "SUVComputation.h"
#include <iostream>

#include "rad_util.h"

using namespace std;
using namespace radspeed;

namespace radspeed
{
	struct SUVParameters
	{
		std::string seriesReferenceTime;
		std::string injectionTime;
		double injectedDose;
		double radionuclideHalfLife;
		std::string radioNuclidePositronFraction;
		std::string radioactivityUnits;
		std::string tissueRadioactivityUnits;
		std::string volumeUnits;
		std::string decayCorrection;
		double decayFactor;
		std::string frameReferenceTime;
		double patientWeight;
		std::string weightUnits;
		double calibrationFactor;

		double correctedDose;
		double weightByDose;
		double tissueConversionFactor;
		bool isSuccessfull;
	};

	double ConvertTimeToSeconds(const char *time)
	{
		if (time == NULL)
		{
			std::cerr << "ConvertTimeToSeconds got a NULL time string." << std::endl;
			return -1.0;
		}

		std::string h;
		std::string m;
		std::string minAndsecStr;
		std::string secStr;

		double hours;
		double minutes;
		double seconds;

		if (time == NULL)
		{
			return 0.0;
		}

		// ---
		// --- time will be in format HH:MM:SS.SSSS
		// --- convert to a double count of seconds.
		// ---
		std::string timeStr = time;
		h = timeStr.substr(0, 2);
		hours = atof(h.c_str());

		minAndsecStr = timeStr.substr(3);
		m = minAndsecStr.substr(0, 2);
		minutes = atof(m.c_str());

		secStr = minAndsecStr.substr(3);
		seconds = atof(secStr.c_str());

		double retval = (seconds
			+ (60.0 * minutes)
			+ (3600.0 * hours));
		return retval;
	}

	// ...
	// ...............................................................................................
	// ...
	double ConvertWeightUnits(double count, const char *fromunits, const char *tounits)
	{

		double conversion = count;

		if (fromunits == NULL)
		{
			std::cout << "Got NULL parameter fromunits. A bad param was probably specified." << std::endl;
			return -1.0;
		}
		if (tounits == NULL)
		{
			std::cout << "Got NULL parameter from tounits. A bad parameter was probably specified." << std::endl;
			return -1.0;
		}

		/*
		possibilities include:
		---------------------------
		"kilograms [kg]"
		"grams [g]"
		"pounds [lb]"
		*/

		// --- kg to...
		if (!strcmp(fromunits, "kg"))
		{
			if (!strcmp(tounits, "kg"))
			{
				return conversion;
			}
			else if (!strcmp(tounits, "g"))
			{
				conversion *= 1000.0;
			}
			else if (!strcmp(tounits, "lb"))
			{
				conversion *= 2.2;
			}
		}
		else if (!strcmp(fromunits, "g"))
		{
			if (!strcmp(tounits, "kg"))
			{
				conversion /= 1000.0;
			}
			else if (!strcmp(tounits, "g"))
			{
				return conversion;
			}
			else if (!strcmp(tounits, "lb"))
			{
				conversion *= .0022;
			}
		}
		else if (!strcmp(fromunits, "lb"))
		{
			if (!strcmp(tounits, "kg"))
			{
				conversion *= 0.45454545454545453;
			}
			else if (!strcmp(tounits, "g"))
			{
				conversion *= 454.54545454545453;
			}
			else if (!strcmp(tounits, "lb"))
			{
				return conversion;
			}
		}
		return conversion;

	}

	// ...
	// ...............................................................................................
	// ...
	double ConvertRadioactivityUnits(double count, const char *fromunits, const char *tounits)
	{

		double conversion = count;

		if (fromunits == NULL)
		{
			std::cout << "Got NULL parameter in fromunits. A bad parameter was probably specified." << std::endl;
			return -1.0;
		}
		if (tounits == NULL)
		{
			std::cout << "Got NULL parameter in tounits. A bad parameter was probably specified." << std::endl;
			return -1.0;
		}

		/*
		possibilities include:
		---------------------------
		"megabecquerels [MBq]"
		"kilobecquerels [kBq]"
		"becquerels [Bq]"
		"millibecquerels [mBq]"
		"microbecquerels [uBq]
		"megacuries [MCi]"
		"kilocuries [kCi]"
		"curies [Ci]"
		"millicuries [mCi]"
		"microcuries [uCi]"
		*/

		// --- MBq to...
		if (!strcmp(fromunits, "MBq"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 1000.0;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 1000000.0;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 1000000000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 1000000000000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.000000000027027027027027;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.000000027027027027027;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 0.000027027027027027;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 0.027027027027027;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 27.027027027;
			}
		}
		// --- kBq to...
		else if (!strcmp(fromunits, "kBq"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= .001;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 1000.0;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 1000000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 1000000000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.000000000027027027027027;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 0.000000027027027027027;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 0.000027027027027027;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 0.027027027027027;
			}
		}
		// --- Bq to...
		else if (!strcmp(fromunits, "Bq"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 0.000001;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 0.001;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 1000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 1000000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.000000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 0.000000000027027027027027;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 0.000000027027027027027;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 0.000027027027027027;
			}
		}
		// --- mBq to...
		else if (!strcmp(fromunits, "mBq"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 0.000000001;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 0.000001;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 0.001;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 1000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.00000000000000000002702702702702;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.000000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 0.000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 0.000000000027027027027027;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 0.000000027027027027027;
			}
		}
		// --- uBq to...
		else if (!strcmp(fromunits, "uBq"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 0.000000000001;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 0.000000001;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 0.000001;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 0.001;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.000000000000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.000000000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 0.000000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 0.000000000000027027027027027;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 0.000000000027027027027027;
			}
		}
		// --- MCi to...
		else if (!strcmp(fromunits, "MCi"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 37000000000.0;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 37000000000000.0;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 37000000000000000.0;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 37000000000000000000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 37000000000000000000848.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 1000.0;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 1000000.0;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 1000000000.0;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 1000000000000.0;
			}
		}
		// --- kCi to...
		else if (!strcmp(fromunits, "kCi"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 37000000.0;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 37000000000.0;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 37000000000000.0;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 37000000000000000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 37000000000000000000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.001;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 1000.0;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 1000000.0;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 1000000000.0;
			}
		}
		// --- Ci to...
		else if (!strcmp(fromunits, "Ci"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 37000.0;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 37000000.0;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 37000000000.0;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 37000000000000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 37000000000000000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.0000010;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.001;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 1000.0;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 1000000.0;
			}
		}
		// --- mCi to...
		else if (!strcmp(fromunits, "mCi"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 37.0;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 37000.0;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 37000000.0;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 37000000000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 37000000000000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.0000000010;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.0000010;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 0.001;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				return conversion;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				conversion *= 1000.0;
			}
		}
		// --- uCi to...
		else if (!strcmp(fromunits, " uCi"))
		{
			if (!(strcmp(tounits, "MBq")))
			{
				conversion *= 0.037;
			}
			else if (!(strcmp(tounits, "kBq")))
			{
				conversion *= 37.0;
			}
			else if (!(strcmp(tounits, "Bq")))
			{
				conversion *= 37000.0;
			}
			else if (!(strcmp(tounits, "mBq")))
			{
				conversion *= 37000000.0;
			}
			else if (!(strcmp(tounits, " uBq")))
			{
				conversion *= 37000000000.0;
			}
			else if (!(strcmp(tounits, "MCi")))
			{
				conversion *= 0.0000000000010;
			}
			else if (!(strcmp(tounits, "kCi")))
			{
				conversion *= 0.0000000010;
			}
			else if (!(strcmp(tounits, "Ci")))
			{
				conversion *= 0.0000010;
			}
			else if (!(strcmp(tounits, "mCi")))
			{
				conversion *= 0.001;
			}
			else if (!(strcmp(tounits, "uCi")))
			{
				return conversion;
			}
		}

		return conversion;
	}

	// ...
	// ...............................................................................................
	// ...
	double DecayCorrection(SUVParameters* list, double inVal)
	{

		double scanTimeSeconds = ConvertTimeToSeconds(list->seriesReferenceTime.c_str());
		double startTimeSeconds = ConvertTimeToSeconds(list->injectionTime.c_str());
		double halfLife = list->radionuclideHalfLife;
		double decayTime = scanTimeSeconds - startTimeSeconds;
		double correctedVal = inVal * (double)pow(2.0, -(decayTime / halfLife));

		return correctedVal;
	}
}
SUVComputation::SUVComputation()
{
	d = new SUVParameters();
	d->isSuccessfull = false;
}


SUVComputation::~SUVComputation()
{
	delete d;
}

void SUVComputation::SetParams(RTDcmtkDicomInterface* pDicom)
{
	DcmDataset* ds = pDicom->dataset;
	std::string yearstr;
	std::string monthstr;
	std::string daystr;
	std::string hourstr;
	std::string minutestr;
	std::string secondstr;
	int len;

	/// start setting up params
	
	// Nuclear Medicine DICOM info:
	/*
	0054,0016  Radiopharmaceutical Information Sequence:
	0018,1072  Radionuclide Start Time: 090748.000000
	0018,1074  Radionuclide Total Dose: 370500000
	0018,1075  Radionuclide Half Life: 6586.2
	0018,1076  Radionuclide Positron Fraction: 0
	*/
	DcmItem * radioPharmaInfoSeq = NULL;
	signed long radioPharmaInfoSeq_Count = 0;
	if (pDicom->dataset->findAndGetSequenceItem(DCM_RadiopharmaceuticalInformationSequence, radioPharmaInfoSeq).good())
	{
		d->isSuccessfull = true;

		//--- Radiopharmaceutical Start Time
		const char * pAddress = NULL;
		if (!pDicom->getDcmTagFromSequenceData(DCM_RadiopharmaceuticalStartTime, radioPharmaInfoSeq, pAddress))
			d->injectionTime = "MODULE_INIT_NO_VALUE";
		else
		{
			d->injectionTime = pAddress;

			len = d->injectionTime.length();
			hourstr.clear();
			minutestr.clear();
			secondstr.clear();
			if (len >= 2)
			{
				hourstr = d->injectionTime.substr(0, 2);
			}
			else
			{
				hourstr = "00";
			}
			if (len >= 4)
			{
				minutestr = d->injectionTime.substr(2, 2);
			}
			else
			{
				minutestr = "00";
			}
			if (len >= 6)
			{
				secondstr = d->injectionTime.substr(4);
			}
			else
			{
				secondstr = "00";
			}
			d->injectionTime.clear();
			d->injectionTime = hourstr.c_str();
			d->injectionTime += ":";
			d->injectionTime += minutestr.c_str();
			d->injectionTime += ":";
			d->injectionTime += secondstr.c_str();

		}

		//--- Radionuclide Total Dose
		d->injectedDose = 0.0;
		if (pDicom->getDcmTagFromSequenceData(DCM_RadionuclideTotalDose, radioPharmaInfoSeq, pAddress))
		{
			d->injectedDose = convert_to_double(pAddress);

		}

		//--- RadionuclideHalfLife
		d->radionuclideHalfLife = 0.0;
		if (pDicom->getDcmTagFromSequenceData(DCM_RadionuclideHalfLife, radioPharmaInfoSeq, pAddress))
		{
			d->radionuclideHalfLife = convert_to_double(pAddress);

		}

		//---Radionuclide Positron Fraction
		d->radioNuclidePositronFraction = "MODULE_INIT_NO_VALUE";
		if (pDicom->getDcmTagFromSequenceData(DCM_RadionuclidePositronFraction, radioPharmaInfoSeq, pAddress))
		{
			d->radioNuclidePositronFraction = pAddress;
		}

		//--
		//--- UNITS: something like BQML:
		//--- CNTS, NONE, CM2, PCNT, CPS, BQML,
		//--- MGMINML, UMOLMINML, MLMING, MLG,
		//--- 1CM, UMOLML, PROPCNTS, PROPCPS,
		//--- MLMINML, MLML, GML, STDDEV
		//---

		if (pDicom->getDcmTagData(DCM_Units, pAddress))
		{
			std::string units = pAddress;

			if ((units.find("BQML") != std::string::npos) ||
				(units.find("BQML") != std::string::npos))
			{
				d->radioactivityUnits = "Bq";
				d->tissueRadioactivityUnits = "Bq";
			}
			else if ((units.find("MBq") != std::string::npos) ||
				(units.find("MBQ") != std::string::npos))
			{
				d->radioactivityUnits = "MBq";
				d->tissueRadioactivityUnits = "MBq";
			}
			else if ((units.find("kBq") != std::string::npos) ||
				(units.find("kBQ") != std::string::npos) ||
				(units.find("KBQ") != std::string::npos))
			{
				d->radioactivityUnits = "kBq";
				d->tissueRadioactivityUnits = "kBq";
			}
			else if ((units.find("mBq") != std::string::npos) ||
				(units.find("mBQ") != std::string::npos))
			{
				d->radioactivityUnits = "mBq";
				d->tissueRadioactivityUnits = "mBq";
			}
			else if ((units.find("uBq") != std::string::npos) ||
				(units.find("uBQ") != std::string::npos))
			{
				d->radioactivityUnits = "uBq";
				d->tissueRadioactivityUnits = "uBq";
			}
			else if ((units.find("Bq") != std::string::npos) ||
				(units.find("BQ") != std::string::npos))
			{
				d->radioactivityUnits = "Bq";
				d->tissueRadioactivityUnits = "Bq";
			}
			else if ((units.find("MCi") != std::string::npos) ||
				(units.find("MCI") != std::string::npos))
			{
				d->radioactivityUnits = "MCi";
				d->tissueRadioactivityUnits = "MCi";
			}
			else if ((units.find("kCi") != std::string::npos) ||
				(units.find("kCI") != std::string::npos) ||
				(units.find("KCI") != std::string::npos))
			{
				d->radioactivityUnits = "kCi";
				d->tissueRadioactivityUnits = "kCi";
			}
			else if ((units.find("mCi") != std::string::npos) ||
				(units.find("mCI") != std::string::npos))
			{
				d->radioactivityUnits = "mCi";
				d->tissueRadioactivityUnits = "mCi";
			}
			else if ((units.find("uCi") != std::string::npos) ||
				(units.find("uCI") != std::string::npos))
			{
				d->radioactivityUnits = "uCi";
				d->tissueRadioactivityUnits = "uCi";
			}
			else if ((units.find("Ci") != std::string::npos) ||
				(units.find("CI") != std::string::npos))
			{
				d->radioactivityUnits = "Ci";
				d->tissueRadioactivityUnits = "Ci";
			}
			d->volumeUnits = "ml";
		}
		else
		{
			//--- default values.
			d->radioactivityUnits = "MBq";
			d->tissueRadioactivityUnits = "MBq";
			d->volumeUnits = "ml";
		}

		//--- DecayCorrection
		//--- Possible values are:
		//--- NONE = no decay correction
		//--- START= acquisition start time
		//--- ADMIN = radiopharmaceutical administration time
		//--- Frame Reference Time  is the time that the pixel values in the Image occurred.
		//--- It's defined as the time offset, in msec, from the Series Reference Time.
		//--- Series Reference Time is defined by the combination of:
		//--- Series Date (0008,0021) and
		//--- Series Time (0008,0031).
		//--- We don't pull these out now, but can if we have to.

		d->decayCorrection = "MODULE_INIT_NO_VALUE";
		if (pDicom->getDcmTagData(DCM_DecayCorrection, pAddress))
		{
			d->decayCorrection = pAddress;
		}

		//--- DecayFactor
		d->decayFactor = 0.0;
		if (pDicom->getDcmTagData(DCM_DecayFactor, pAddress))
		{
			d->decayFactor = convert_to_double(pAddress);
		}

		//--- FrameReferenceTime
		d->frameReferenceTime = "MODULE_INIT_NO_VALUE";
		if (pDicom->getDcmTagData(DCM_FrameReferenceTime, pAddress))
		{
			d->frameReferenceTime = pAddress;
		}

		//--- SeriesTime

		d->seriesReferenceTime = "MODULE_INIT_NO_VALUE";

		if (pDicom->getDcmTagData(DCM_SeriesTime, pAddress))
		{
			d->seriesReferenceTime = pAddress;
			hourstr.clear();
			minutestr.clear();
			secondstr.clear();
			len = d->seriesReferenceTime.length();
			if (len >= 2)
			{
				hourstr = d->seriesReferenceTime.substr(0, 2);
			}
			else
			{
				hourstr = "00";
			}
			if (len >= 4)
			{
				minutestr = d->seriesReferenceTime.substr(2, 2);
			}
			else
			{
				minutestr = "00";
			}
			if (len >= 6)
			{
				secondstr = d->seriesReferenceTime.substr(4);
			}
			else
			{
				secondstr = "00";
			}
			d->seriesReferenceTime.clear();
			d->seriesReferenceTime = hourstr.c_str();
			d->seriesReferenceTime += ":";
			d->seriesReferenceTime += minutestr.c_str();
			d->seriesReferenceTime += ":";
			d->seriesReferenceTime += secondstr.c_str();
		}

		//--- PatientWeight
		d->patientWeight = 0.0;
		d->weightUnits = "";
		if (pDicom->getDcmTagData(DCM_PatientWeight, pAddress))
		{
			d->patientWeight = convert_to_double(pAddress);
			d->weightUnits = "kg";
		}

		//--- CalibrationFactor
		d->calibrationFactor = 0.0;
		if (pDicom->getDcmTagData(DCM_DoseCalibrationFactor, pAddress))
		{
			d->calibrationFactor = convert_to_double(pAddress);
		}

		// --- we want to use the following units:
		// --- CPET(t) -- tissue radioactivity in pixels-- kBq/mlunits
		// --- injectced dose-- MBq and
		// --- patient weight-- kg.
		// --- computed SUV should be in units g/ml
		double weight = d->patientWeight;
		d->correctedDose = d->injectedDose;

		d->tissueConversionFactor = ConvertRadioactivityUnits(1, d->radioactivityUnits.c_str(), "kBq");

		d->correctedDose = ConvertRadioactivityUnits(d->correctedDose, d->radioactivityUnits.c_str(), "MBq");
		d->correctedDose = DecayCorrection(d, d->correctedDose);

		weight = ConvertWeightUnits(weight, d->weightUnits.c_str(), "kg");

		if (d->correctedDose == 0.0)
		{
			d->weightByDose = 1.0;
		}
		else
		{
			d->weightByDose = weight / d->correctedDose;
		}

	}
}

double SUVComputation::GetSUV(long int pixVal)
{
	return (pixVal * d->tissueConversionFactor) * d->weightByDose;
}