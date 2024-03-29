/*=========================================================================
 
 Program:   IGI 3D Digitizer
 Module:    $RCSfile: main.cxx,v $
 Language:  C++
 Date:      $Date: 20012-10-18 15:25:38 $
 Version:   $Revision: 1.0 $
 
 Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
 See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#if defined(_MSC_VER)
//  Warning about: identifier was truncated to '255' characters
//  in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif

/** Qt */
#include <QApplication>

#include "IGI3DDigitizerGUI.h"

#ifdef _WIN32
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
  IGI3DDigitizerGUI mainWindow;
  mainWindow.show();

  return app.exec();
}
