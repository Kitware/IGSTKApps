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
