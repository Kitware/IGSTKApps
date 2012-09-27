/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: main.cxx,v $
  Language:  C++
  Date:      $Date: 2009-03-20 15:25:38 $
  Version:   $Revision: 1.3 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//QT header file
#include <QApplication>

#include "PivotCalibration.h"
#include "igstkRealTimeClock.h"

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
  PivotCalibration mainWindow;
  mainWindow.show();
  return app.exec();
}
