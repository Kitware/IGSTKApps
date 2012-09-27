#ifndef VALIDATE_CAMERA_CALIBRATION_H_
#define VALIDATE_CAMERA_CALIBRATION_H_

#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace std;

class ValidateCameraCalibration
{
public:
  ValidateCameraCalibration();

  virtual ~ValidateCameraCalibration();

  //IplImage* getGrayImage(IplImage* imageRGB);
  void DetectCorners(IplImage* image);
  void DetectLines(IplImage* image);
  std::string DetectChessboardCorners(IplImage* img, cv::Size boardSize,float squareSize);
  bool readCameraMatrix(const string& filename,
                             cv::Mat& cameraMatrix, cv::Mat& distCoeffs,
                             cv::Size& calibratedImageSize );

  int cornersX;
  int cornersY;
  int cornersN;

  // parameter for calibration
  vector<CvPoint2D32f> tempPoints;
  vector<CvPoint3D32f> objectPoints;
  vector<CvPoint2D32f> points;
  vector<int> npoints;

  vector<int> m_cornerCount;
  vector<IplImage*> m_images;
  vector<CvPoint2D32f*> m_corners;

  CvMat* allObjectPoints;
  CvMat* allImagePoints;
  CvMat* allDetectedPoints;
  CvSize imageSize;

  bool calibrationInited;
  bool calibrationDone;

  CvMat* translationVector;
  CvMat* intrinsicMatrix;
  CvMat* distortionCoeffs;
  CvMat* rotMatrix;
  CvMat* rotVector;
};

#endif /* VALIDATE_CAMERA_CALIBRATION_H_ */