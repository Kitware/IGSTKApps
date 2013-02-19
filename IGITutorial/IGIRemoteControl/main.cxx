/*=========================================================================

  Program:   IGI Remote Control
  Module:    $RCSfile: main.cxx,v $
  Language:  C++
  Date:      $Date: 20012-09-20 15:25:38 $
  Version:   $Revision: 1.0 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//QT header file
#include <QApplication>

#include "RemoteControl.h"

#include "igstkRealTimeClock.h"

/**
 * This program uses a monocular (read webcam) tracker to send "Page Down" key
 * and "Page Up" key to either a user specified program that will have focus or
 * the current program that has focus. In practice this is a free remote control 
 * for a power point presentation. Allows the user to move forward and backward 
 * in the presentation by showing tracked markers (see the markers.pdf file).
 *
 * Page Down:          marker 202
 * Page Up:            marker 502
 * Exit this program:  marker 201.
 * 
 * To build this program from source you will need to:
 * a. build the Insight Segmentation and Registration Toolkit (ITK) [www.itk.org]
 * b. build the Visualization Toolkit (VTK) [www.vtk.org]
 * c. build the Open Source Computer Vision library (OpenCV) [www.opencv.org]
 * d. build the Image-Guided Surgery Toolkit (IGSTK) [www.igstk.org]
 */

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
#else
int main(int, char**)
#endif
{
  igstk::RealTimeClock::Initialize();
  int argc = 0;
  QApplication app(argc,0);
  RemoteControl mainWindow;
  //if initialization succeeded run the application
  if( ( mainWindow.Setup() ).IsNotNull() )
  {
    mainWindow.show();
	  return app.exec();
  }
  else  //keep the error message visible for 2s and then exit
  {
    igstk::PulseGenerator::Sleep(2000);
	  return EXIT_FAILURE;
  }
}
