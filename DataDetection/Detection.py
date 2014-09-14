#!/usr/bin/env python
""" This script segregates dicom files from a folder into seperate folder identified by series instance uid
@input1: Source directory
@input2: Dest directory """

import os, sys
import shutil
import glob
import dicom # python dicom library

def segregate(src, dest):
    fd = open("data.csv","wb")
    dcmFileList = glob.glob(os.path.join(src,"*.dcm"))
    print "Segregating:",len(dcmFileList)," files"
    pDicom = dicom.read_file(dcmFileList[0])
    patientName = pDicom.PatientName
    
    dest = os.path.join(dest, patientName)
    if not os.path.exists(dest):
        os.makedirs(dest)
    
    for f in dcmFileList:
        pDicom = dicom.read_file(f)
        seriesInstanceUID = pDicom.SeriesInstanceUID
        seriesDest = os.path.join(dest, seriesInstanceUID)
        if not os.path.exists(seriesDest):
            os.makedirs(seriesDest)
        shutil.copy(f,seriesDest)
       
        pos = "0,0,0"
        if pDicom.__contains__("ImagePositionPatient"):
            pos = ",".join([str(pDicom.ImagePositionPatient[0]),
                             str(pDicom.ImagePositionPatient[1]),
                             str(pDicom.ImagePositionPatient[2])])
        fd.write(",".join([pDicom.Modality,pDicom.SOPInstanceUID,pos]))
        fd.write("\n")
    fd.close()
    shutil.move("data.csv",dest)
    
        
        
if __name__ == '__main__':
    src = raw_input("Enter source directory:")
    dest = raw_input("Enter dest directory:")
    segregate(src,dest)
    