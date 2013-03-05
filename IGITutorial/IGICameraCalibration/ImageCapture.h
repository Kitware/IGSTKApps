#ifndef _IMAGE_CAPTURE_H_
#define _IMAGE_CAPTURE_H_

#include <QObject>
#include <QPixmap>
#include <QTimer>

#include <cv.h>
#include <highgui.h>

/** \class ImageCapture
 *
 * \brief Captures frames from the camera and stores them as image files.
 *
 */

class ImageCapture : public QObject
{
  Q_OBJECT

public:
  ImageCapture( QObject * parent = NULL );
  ~ImageCapture();

  void StopTimer() 
       { m_Timer->stop(); }
  void StartTimer() 
       { m_Timer->start(); };

public slots:
  void StopCapture();
  IplImage* GetCurrentFrame();
  bool CaptureFromCamera( int cameraIndex = -1 );
  /** Save captured file to image file */
  void CaptureToFile(std::string path);

protected slots:
  /** Trigger image capture */
  void TimerTick();

signals:
  void ImageCaptured( const QImage& image );
  void Error( const QString& text );

protected:
  /** Convert OpenCV image data to QImage */
  QImage Convert( IplImage * image );

  CvCapture * m_CvCap;
  QTimer * m_Timer;
};

#endif
