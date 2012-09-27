#if defined(_MSC_VER)
//  Warning about: identifier was truncated to '255' characters 
//  in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif

#include "../IGIConfigurationData.h"

/** Qt */
#include <QApplication>

#include "IGIPlanningGUI.h"

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
  IGIPlanningGUI mainWindow;
  mainWindow.show();
  return app.exec();
}
