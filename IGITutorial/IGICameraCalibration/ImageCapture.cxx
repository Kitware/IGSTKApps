#include "ImageCapture.h"
#include "ImageCapture.moc"

#include <sys/types.h>
#include <sys/stat.h>

#include <QImage>
#include <QFile>
#include <QTimer>
#include <QTime>
#include <QDir>

ImageCapture::ImageCapture( QObject * parent )
  :QObject(parent)
  ,m_timer(NULL)
  ,m_cvCap(NULL)
  ,m_flipImage(true)
{
  this->m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(timerTick()));
}

ImageCapture::~ImageCapture(){
  this->stopCapture();
}

void ImageCapture::stopCapture(){
  this->m_timer->stop();
  cvReleaseCapture(&m_cvCap);
}

bool ImageCapture::captureFromCamera( int cameraIndex ){
  this->stopCapture();
  this->m_cvCap = cvCreateCameraCapture(cameraIndex);
  if( m_cvCap )
  {
    this->m_timer->setInterval(10);
    this->m_timer->start();
    return true;
  } else
  {
    emit error( "No camera found." );
    return false;
  }
}

std::vector<int> ImageCapture::getAvailableCameras()
{
  std::vector<int> availableCameraHandles;

    for (int i=0;i<10;i++)
    {
      this->stopCapture();
      this->m_cvCap = cvCaptureFromCAM(i);
    if( this->m_cvCap )
      {
        availableCameraHandles.push_back(i);
      }
    }
  return availableCameraHandles;
}

void ImageCapture::timerTick(){

  IplImage * image = cvQueryFrame(this->m_cvCap);
  if( image )
  {
    emit imageCaptured( convert(image) );
  }
}

void ImageCapture::flipImage(bool on){
  m_flipImage = on;
}

QImage ImageCapture::convert( IplImage* image ) {
  QImage qImage;
  if (image) {
    // nChannels: 3, depth: 8
    // colorModel: "RBG", channelSeq: "BGR"
    if ( (image->depth == IPL_DEPTH_8U) && (image->nChannels == 3) ) {
      qImage = QImage(image->width, image->height, QImage::Format_RGB32);
      int x, y;
      char* data = image->imageData;

      for( y = 0; y < image->height; y++, data += image->widthStep )
        for( x = 0; x < image->width; x++ )
        {
          uint *p = (uint*)qImage.scanLine(y) + x;
          *p = qRgb(data[x * image->nChannels+2],
                    data[x * image->nChannels+1],
                    data[x * image->nChannels]);
        } // end for x
    } else {
      qImage = QImage(image->width, image->height, QImage::Format_Invalid);
      emit error( tr("Format not supported: depth=%1, channels=%2").arg(image->depth).arg(image->nChannels));
  } // end if image->depth && image->nChannels
  } else {
    qImage = QImage(image->width, image->height, QImage::Format_Invalid);
    emit error("Image pointer is NULL");
  }
  return qImage;
}

void ImageCapture::captureToFile()
{
  IplImage * image = cvQueryFrame(this->m_cvCap);

  std::string tmpDir = "tmp";
  if( image )
  {
    struct stat sb;
    if (!(stat(tmpDir.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR)))
    {
    	QDir().mkdir(tmpDir.c_str());
    }
    QString filename =  QString(tmpDir.c_str()) +
                        QString("/image_")+
                        QTime::currentTime().toString("hh_mm_ss") + ".jpg";
    cvSaveImage(filename.toAscii().data() , image);
  }
  else
  {
    emit error("Image pointer is NULL");
  }

   if( image )
  {
    cv::Mat imageWithAnnotation(image);
    cv::putText(imageWithAnnotation,"Image captured.", cv::Point2f(100,200),cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(0,0,255,255),2);
    emit imageCaptured( convert(image) );
    emit imageCaptured( convert(image) );
    emit imageCaptured( convert(image) );
  }
}

IplImage* ImageCapture::getCurrentFrame()
{
  IplImage * image = cvQueryFrame(this->m_cvCap);
  if( image )
  {
    return image;
  }
  else
  {
    emit error("Image pointer is NULL");
        return NULL;
  }
}

