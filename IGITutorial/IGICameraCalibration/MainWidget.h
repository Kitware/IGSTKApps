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

public slots:
  void StartCamera();
  void DisplayImage( const QImage& image );
  void CalibrateCamera();
  void ValidateCalibration();
  void QuitApp();
  void ShowHelp();
  void CaptureToFile();

protected slots:
  void CaptureError( const QString& text );

private:
  Ui_MainWidget* m_UI;
  ImageCapture* m_Capture;
  std::vector<std::string> m_FileNames;
  std::string m_ImageDirectory;
  QString m_CurrentPath;
  QString m_ConfigDir;
  void keyPressEvent(QKeyEvent * e );
};

#endif
