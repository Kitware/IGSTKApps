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

using namespace cv;
using namespace std;

CameraCalibration::CameraCalibration():m_Canvas(NULL)
{
}

CameraCalibration::~CameraCalibration(void)
{
  cvReleaseMat(&rotVector);
  cvReleaseMat(&translationVector);
  cvReleaseMat(&intrinsicMatrix);
  cvReleaseMat(&distortionCoeffs);
  cvReleaseMat(&rotMatrix);
}

void CameraCalibration::setPoints(double squareSize=1)
{
  allObjectPoints = cvCreateMat(m_images.size()* cornersN, 3, CV_32FC1); // 3D-Koordinaten der (verzerrten) Eckpunkte des Schachbrettmusters
  allImagePoints = cvCreateMat(m_images.size()* cornersN, 2, CV_32FC1); // 2D-Koordinaten der (verzerrten) Eckpunkte im Kamerabild
  allDetectedPoints = cvCreateMat(m_images.size(), 1, CV_32SC1); // Anzahl Eckpunkte in den Bildern

  for (int i = 0; i < m_images.size(); i++) { // erkannte Bilder durchlaufen
    CV_MAT_ELEM( *allDetectedPoints, int, i, 0 ) = m_cornerCount[i];
    for(int y = 0; y < cornersY; y++ )
    {
      for(int x = 0; x < cornersX; x++ )
      {
        CV_MAT_ELEM( *allImagePoints, float, x + y * cornersX + i * cornersX * cornersY, 0 ) = m_corners[i][x + y * cornersX].x;
        CV_MAT_ELEM( *allImagePoints, float, x + y * cornersX + i * cornersX * cornersY, 1 ) = m_corners[i][x + y * cornersX].y;
          
        CV_MAT_ELEM( *allObjectPoints, float,x + y * cornersX + i * cornersX * cornersY , 0 ) = x * squareSize;
        CV_MAT_ELEM( *allObjectPoints, float,x + y * cornersX + i * cornersX * cornersY, 1 ) = y * squareSize;
        CV_MAT_ELEM( *allObjectPoints, float, x + y * cornersX + i * cornersX * cornersY, 2 ) = 0.0;
      }
    }
  }
}

void CameraCalibration::calibrateCameraEnd(CvSize imageSize)
{
  intrinsicMatrix = cvCreateMat(3, 3, CV_64F);
  distortionCoeffs = cvCreateMat(1, 4, CV_64F);
  rotVector = cvCreateMat(m_images.size(), 3, CV_64F);
  rotMatrix = cvCreateMat(3, 3, CV_64F);
  translationVector = cvCreateMat(m_images.size(), 3, CV_64F);

  cvSetIdentity(intrinsicMatrix);
  cvZero(distortionCoeffs);

  cvCalibrateCamera2(allObjectPoints,allImagePoints,allDetectedPoints,imageSize,
                intrinsicMatrix, distortionCoeffs, rotVector, translationVector,0);
}

IplImage* CameraCalibration::getGrayImage(IplImage* imageRGB)
{
  IplImage *imageGray = cvCreateImage( cvGetSize(imageRGB), 8, 1 );
  cvCvtColor( imageRGB, imageGray, CV_BGR2GRAY );
  return imageGray;
}

int CameraCalibration::findChessboardCorners(IplImage* image)
{ 
  int cornersDetected = 0;
  tempPoints.resize(cornersN);

  int result = cvFindChessboardCorners(
  image, cvSize(cornersX, cornersY),
    &tempPoints[0], &cornersDetected,
    CV_CALIB_CB_ADAPTIVE_THRESH
  );

  printf("Corners detected = %d\n",cornersDetected);
  cvDrawChessboardCorners(image,cvSize(cornersX,cornersY),&tempPoints[0],cornersDetected,result);
 
  // Display the image
	displayImage(image);
	// Give system a chance to update the GUI
  QApplication::processEvents();
  //cvWaitKey(9);

#ifdef _WIN32
  Sleep(1000);
#else
  sleep(1);  // wait 1 sec
#endif



  if(result && cornersDetected == cornersN)
  {
    cvFindCornerSubPix(
      image,&tempPoints[0], cornersDetected,
      cvSize(11, 11), cvSize(-1,-1),
      cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,30, 0.01));

    CvPoint2D32f* newPoints = new CvPoint2D32f[cornersDetected];
    for (int i = 0; i < cornersDetected; i++) newPoints[i] = tempPoints[i];

    m_cornerCount.push_back(cornersDetected);
    m_corners.push_back(newPoints);
    m_images.push_back(image);
  }
  return cornersDetected;
}

bool CameraCalibration::calibrate(vector<IplImage*> imageSet)
{
  int nrValidImages = 0;
  for(vector<IplImage*>::iterator it = imageSet.begin();it!=imageSet.end();it++)
  {
    IplImage* imgIn = *it;
    IplImage *imageGray = getGrayImage(imgIn);
  
    imageSize = cvGetSize(imageGray);
    int cornersFound = findChessboardCorners(imageGray);
    if(cornersFound == cornersN)
      nrValidImages++;
  }

  if (nrValidImages < IGIConfigurationData::NR_CALIBRATION_IMAGES)
    return false;

  setPoints();
  calibrateCameraEnd(imageSize);

  CvSize boardSize;
  boardSize.height=5;
  boardSize.width=8;
  float squareSize = 1.f, aspectRatio = 1.f;
  int flags = 0;
  bool writeExtrinsics = false;
  vector<CvMat> emptyMat;
  vector<float> emptyFloat;
  double totalAvgErr = 0;

  std::string path(std::string("../")
              + std::string(IGIConfigurationData::CONFIGURATION_FOLDER));

  std::string file(std::string("../")
              + std::string(IGIConfigurationData::CONFIGURATION_FOLDER)
              + std::string("/")
              + std::string(IGIConfigurationData::CAMERA_CALIBRATION_FILENAME));

  struct stat sb;

  if (!(stat(path.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR)))
  {
    QDir().mkdir(path.c_str());
  }

  saveCameraParams(file,
              imageSize,
              boardSize, squareSize, aspectRatio,
              flags, intrinsicMatrix, distortionCoeffs,
              emptyMat,
              emptyMat);
  return true;
}

vector<IplImage*> CameraCalibration::setImageSeries(std::vector<std::string> filenames)
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

    imageTmp= cvLoadImage( fileName, 1);

    if(!imageTmp)
    {
      fprintf(stderr, "Error loading image %s : %s\n", fileName, cvErrorStr(cvGetErrStatus()));
    }
    else
    {
      std::cerr << imageVectorConatiner.size() << ". Image name: " << fileName << std::endl;
      imageVectorConatiner.push_back(imageTmp);
    }
  }
  return imageVectorConatiner;
}

void CameraCalibration::saveCameraParams( const string& filename,
                       CvSize imageSize, CvSize boardSize,
                       float squareSize, float aspectRatio, int flags,
                       const CvMat* cameraMatrix, const CvMat* distCoeffs,
                       const vector<CvMat>& rvecs, const vector<CvMat>& tvecs)
{
    FileStorage fs( filename, FileStorage::WRITE );

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
	void CameraCalibration::displayImage( IplImage *image ){
		// Do nothing if no display canvas is assigned
		if (!m_Canvas)
			return;

		// Convert the IplImage to QImage without copying data
		QImage img = QImage((uchar*)image->imageData, image->width, image->height, QImage::Format_Indexed8);
	  const QPixmap pix = QPixmap::fromImage(img);
	  const QSize size = m_Canvas->size();
		m_Canvas->setPixmap(pix.scaled(size, Qt::KeepAspectRatio));
	}
