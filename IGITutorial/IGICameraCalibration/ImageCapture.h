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

class ImageCapture : public QObject{
  Q_OBJECT
public:
  ImageCapture( QObject * parent = NULL );
  ~ImageCapture();
  std::vector<int> getAvailableCameras();
  void stopTimer() { m_timer->stop(); }
  void startTimer() { m_timer->start(); };
public slots:
  bool captureFromCamera( int cameraIndex = -1 );
  void flipImage( bool on );          // switch image flipping on / off
  void stopCapture();
  IplImage* getCurrentFrame();

  /** Save captured file to image file */
  void captureToFile();

protected slots:
  /** Trigger image capture */
  void timerTick();

signals:
  void imageCaptured( const QImage& image );
  void error( const QString& text );

protected:
  /** Convert OpenCV image data to QImage */
  QImage convert( IplImage * image );

  CvCapture * m_cvCap;
  bool m_flipImage;
  QTimer * m_timer;
};

#endif
