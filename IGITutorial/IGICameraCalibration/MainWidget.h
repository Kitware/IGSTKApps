#ifndef _MAIN_WIDGET_H_
#define _MAIN_WIDGET_H_

#include <QObject>
#include <QWidget>
#include <QKeyEvent>

#include "ImageCapture.h"
#include "CameraCalibration.h"
#include "ValidateCameraCalibration.h"

class Ui_MainWidget;

class MainWidget : public QWidget
{
  Q_OBJECT
public:
  MainWidget( QWidget * parent = NULL );
  ~MainWidget();
  void getImagesFromCurrentDirectory();
public slots:
  void startCamera();
  void displayImage( const QImage& image );
  void calibrateCamera();
  void validateCalibration();
  void quitApp();
  void showHelp();
protected slots:
  void captureError( const QString& text );
private:
  Ui_MainWidget* m_UI;
  ImageCapture* m_Capture;
  vector<std::string> m_fileNames;
  void keyPressEvent(QKeyEvent * e );
};

#endif
