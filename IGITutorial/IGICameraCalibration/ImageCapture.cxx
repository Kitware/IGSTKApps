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
  ,m_Timer(NULL)
  ,m_CvCap(NULL)
{
  this->m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(TimerTick()));
}

ImageCapture::~ImageCapture()
{
  this->StopCapture();
}

void ImageCapture::StopCapture()
{
  this->m_Timer->stop();
  cvReleaseCapture(&m_CvCap);
}

bool ImageCapture::CaptureFromCamera( int cameraIndex )
{
  this->StopCapture();
  this->m_CvCap = cvCreateCameraCapture(cameraIndex);
  if( m_CvCap )
  {
    this->m_Timer->setInterval(10);
    this->m_Timer->start();
    return true;
  } 
  else
  {
    emit Error( "No camera found." );
    return false;
  }
}

void ImageCapture::TimerTick()
{
  IplImage * image = cvQueryFrame(this->m_CvCap);
  if( image )
  {
    emit ImageCaptured( Convert(image) );
  }
}

QImage ImageCapture::Convert( IplImage* image )
{
  QImage qImage;
  if (image)
  {
    // nChannels: 3, depth: 8
    // colorModel: "RBG", channelSeq: "BGR"
    if ( (image->depth == IPL_DEPTH_8U) && (image->nChannels == 3) )
    {
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
        }
    } 
    else 
    {
      qImage = QImage(image->width, image->height, QImage::Format_Invalid);
      emit Error( tr("Format not supported: depth=%1, channels=%2").arg(image->depth).arg(image->nChannels));
    }
  }
  else
  {
    qImage = QImage(image->width, image->height, QImage::Format_Invalid);
    emit Error("Image pointer is NULL");
  }
  return qImage;
}

void ImageCapture::CaptureToFile(std::string path)
{
  IplImage * image = cvQueryFrame(this->m_CvCap);
	
  if( image )
  {
    struct stat sb;
    if (!(stat(path.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR)))
    {
    	QDir().mkdir(path.c_str());
    }
    QString filename =  QString(path.c_str()) +
                        QString("/image_")+
                        QTime::currentTime().toString("hh_mm_ss") + ".jpg";
    cvSaveImage(filename.toAscii().data() , image);
  }
  else
  {
    emit Error("Image pointer is NULL");
  }

  if( image )
  {
    cv::Mat imageWithAnnotation(image);
    cv::putText(imageWithAnnotation,"Image captured.", cv::Point2f(100,200),cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(0,0,255,255),2);
    emit ImageCaptured( Convert(image) );
  }
}

IplImage* ImageCapture::GetCurrentFrame()
{
  IplImage * image = cvQueryFrame(this->m_CvCap);
  if( image )
  {
    return image;
  }
  else
  {
    emit Error("Image pointer is NULL");
    return NULL;
  }
}

