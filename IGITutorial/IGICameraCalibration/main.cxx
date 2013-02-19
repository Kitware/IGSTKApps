/*=========================================================================
 
 Program:   IGI Camera Calibration
 Module:    $RCSfile: main.cxx,v $
 Language:  C++
 Date:      $Date: 2012-09-20 15:25:38 $
 Version:   $Revision: 1.0 $
 
 Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
 See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include <QApplication>
#include "MainWidget.h"

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
#else
int main(int, char**)
#endif
{
  int argc = 0;
  QApplication app(argc,0);
  MainWidget w;
  w.show();
  return app.exec();
}
