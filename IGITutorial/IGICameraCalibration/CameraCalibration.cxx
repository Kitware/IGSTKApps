#include "CameraCalibration.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <QDir>
#include <QApplication>

#include "../IGIConfigurationData.h"

CameraCalibration::CameraCalibration():m_Canvas(NULL)
{}

CameraCalibration::~CameraCalibration(void)
{
  cvReleaseMat(&m_AllObjectPoints);
  cvReleaseMat(&m_AllImagePoints);
  cvReleaseMat(&m_AllDetectedPoints);
  cvReleaseMat(&m_RotVector);
  cvReleaseMat(&m_TranslationVector);
  cvReleaseMat(&m_IntrinsicMatrix);
  cvReleaseMat(&m_DistortionCoeffs);
  cvReleaseMat(&m_RotMatrix);
}

void CameraCalibration::SetPoints(double squareSize=1)
{
  m_AllObjectPoints = cvCreateMat(m_Images.size()* m_CornersN, 3, CV_32FC1); 
  m_AllImagePoints = cvCreateMat(m_Images.size()* m_CornersN, 2, CV_32FC1);
  m_AllDetectedPoints = cvCreateMat(m_Images.size(), 1, CV_32SC1);

  for (int i = 0; i < m_Images.size(); i++)
  {
    CV_MAT_ELEM( *m_AllDetectedPoints, int, i, 0 ) = m_CornerCount[i];
    for(int y = 0; y < m_CornersY; y++ )
    {
      for(int x = 0; x < m_CornersX; x++ )
      {
        CV_MAT_ELEM( *m_AllImagePoints, float, x + y * m_CornersX + i * m_CornersX * m_CornersY, 0 ) = m_Corners[i][x + y * m_CornersX].x;
        CV_MAT_ELEM( *m_AllImagePoints, float, x + y * m_CornersX + i * m_CornersX * m_CornersY, 1 ) = m_Corners[i][x + y * m_CornersX].y;
          
        CV_MAT_ELEM( *m_AllObjectPoints, float,x + y * m_CornersX + i * m_CornersX * m_CornersY , 0 ) = x * squareSize;
        CV_MAT_ELEM( *m_AllObjectPoints, float,x + y * m_CornersX + i * m_CornersX * m_CornersY, 1 ) = y * squareSize;
        CV_MAT_ELEM( *m_AllObjectPoints, float, x + y * m_CornersX + i * m_CornersX * m_CornersY, 2 ) = 0.0;
      }
    }
  }
}

void CameraCalibration::CalibrateCamera(CvSize imageSize)
{
  m_IntrinsicMatrix = cvCreateMat(3, 3, CV_64F);
  m_DistortionCoeffs = cvCreateMat(1, 4, CV_64F);
  m_RotVector = cvCreateMat(m_Images.size(), 3, CV_64F);
  m_RotMatrix = cvCreateMat(3, 3, CV_64F);
  m_TranslationVector = cvCreateMat(m_Images.size(), 3, CV_64F);

  cvSetIdentity(m_IntrinsicMatrix);
  cvZero(m_DistortionCoeffs);

  cvCalibrateCamera2(m_AllObjectPoints,
                     m_AllImagePoints,
                     m_AllDetectedPoints,
                     imageSize,
                     m_IntrinsicMatrix,
                     m_DistortionCoeffs,
                     m_RotVector,
                     m_TranslationVector,0);
}

IplImage* CameraCalibration::GetGrayImage(IplImage* imageRGB)
{
  IplImage *imageGray = cvCreateImage( cvGetSize(imageRGB), 8, 1 );
  cvCvtColor( imageRGB, imageGray, CV_BGR2GRAY );
  return imageGray;
}

int CameraCalibration::FindChessboardCorners(IplImage* image)
{ 
  int cornersDetected = 0;
  m_TempPoints.resize(m_CornersN);

  IplImage *imageGray = GetGrayImage(image);
  int result = cvFindChessboardCorners(imageGray,
                                       cvSize(m_CornersX, m_CornersY),
                                       &m_TempPoints[0], &cornersDetected,
                                       CV_CALIB_CB_ADAPTIVE_THRESH);

  cvDrawChessboardCorners(image,cvSize(m_CornersX,m_CornersY),&m_TempPoints[0],cornersDetected,result);

  // Display the image
  DisplayImage(image);
  // Give system a chance to update the GUI
  QApplication::processEvents();
  
  // wait 1 sec
#ifdef _WIN32
  Sleep(1000);
#else
  sleep(1);
#endif

  if(result && cornersDetected == m_CornersN)
  {
    cvFindCornerSubPix(imageGray,&m_TempPoints[0],
                       cornersDetected,
                       cvSize(11, 11),
                       cvSize(-1,-1),
                       cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,
                                      30,
                                      0.01));

    CvPoint2D32f* newPoints = new CvPoint2D32f[cornersDetected];
    for (int i = 0; i < cornersDetected; i++) 
      newPoints[i] = m_TempPoints[i];

    m_CornerCount.push_back(cornersDetected);
    m_Corners.push_back(newPoints);
    m_Images.push_back(image);
  }
  cvReleaseImage(&imageGray);
  // it is not enough that corners are detected, also the pattern
  // has to be found. If result = 0 (no pattern detected) then 
  // false is returned
  return result ? cornersDetected : false;
}

bool CameraCalibration::StartCalibration(std::vector<IplImage*> imageSet, std::string savePath, std::string saveName)
{
  int nrValidImages = 0;
  for(std::vector<IplImage*>::iterator it = imageSet.begin();it!=imageSet.end();it++)
  {
    IplImage* imgIn = *it;
  
    m_ImageSize = cvGetSize(imgIn);
    int cornersFound = FindChessboardCorners(imgIn);
    if(cornersFound == m_CornersN)
      nrValidImages++;
  }

  if (nrValidImages < IGIConfigurationData::NR_CALIBRATION_IMAGES)
    return false;

  SetPoints();
  CalibrateCamera(m_ImageSize);

  CvSize boardSize;
  boardSize.height=5;
  boardSize.width=8;
  float squareSize = 1.f, aspectRatio = 1.f;
  int flags = 0;
  std::vector<CvMat> emptyMat;
  std::vector<float> emptyFloat;

  struct stat sb;

  if (!(stat(savePath.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR)))
  {
    QDir().mkdir(savePath.c_str());
  }

  std::string saveFileName = savePath + "/" + saveName;
  
  SaveCameraParams(saveFileName,
              m_ImageSize,
              boardSize, squareSize, aspectRatio,
              flags, m_IntrinsicMatrix, m_DistortionCoeffs,
              emptyMat,
              emptyMat);
  return true;
}

std::vector<IplImage*> CameraCalibration::SetImageSeries(std::vector<std::string> filenames)
{
  IplImage* imageTmp;
  std::vector<IplImage*> imageVectorConatiner;
  std::vector<std::string>::iterator it;

  for(it=filenames.begin(); it < filenames.end(); it++)
  {
    const char * fileName = it->c_str();
    if(strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0)
    {
      continue;
    }

    imageTmp = cvLoadImage( fileName, 1);

    if(!imageTmp)
    {
      fprintf(stderr, "Error loading image %s : %s\n", fileName, cvErrorStr(cvGetErrStatus()));
    }
    else
    {
      imageVectorConatiner.push_back(imageTmp);
    }
  }
  return imageVectorConatiner;
}

void CameraCalibration::SaveCameraParams( const std::string& filename,
                       CvSize imageSize, CvSize boardSize,
                       float squareSize, float aspectRatio, int flags,
                       const CvMat* cameraMatrix, const CvMat* distCoeffs,
                       const std::vector<CvMat>& rvecs, const std::vector<CvMat>& tvecs)
{
    cv::FileStorage fs( filename, cv::FileStorage::WRITE );

    time_t t;
    time( &t );
    struct tm *t2 = localtime( &t );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_time" << buf;

    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs << "board_width" << boardSize.width;
    fs << "board_height" << boardSize.height;
    fs << "square_size" << squareSize;

    if( flags & CV_CALIB_FIX_ASPECT_RATIO )
        fs << "aspectRatio" << aspectRatio;

    if( flags != 0 )
    {
      sprintf( buf, "flags: %s%s%s%s",
              flags & CV_CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
              flags & CV_CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
              flags & CV_CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
              flags & CV_CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "" );
      cvWriteComment( *fs, buf, 0 );
    }

    fs << "flags" << flags;

    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;
}

// Display the processed image on the given canvas.
// The canvas is a QLabel object stored in m_Canvas.
void CameraCalibration::DisplayImage( IplImage *image )
{
  // Do nothing if no display canvas is assigned
  if (!m_Canvas)
    return;

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
        } // end for x
    }
	  else 
    {
	    qImage = QImage(image->width, image->height, QImage::Format_Invalid);
    }
  }
  else 
  {
    qImage = QImage(image->width, image->height, QImage::Format_Invalid);
  }
 
  const QPixmap pix = QPixmap::fromImage(qImage);
  const QSize size = m_Canvas->size();
  m_Canvas->setPixmap(pix.scaled(size, Qt::KeepAspectRatio));
}
