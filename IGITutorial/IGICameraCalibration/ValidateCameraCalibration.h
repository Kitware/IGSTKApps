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

  std::string Validate(IplImage* img, 
                       cv::Size boardSize,
                       float squareSize,
                       std::string camCalibFile);
  
  bool ReadCameraMatrix(const string& filename,
                        cv::Mat& cameraMatrix, cv::Mat& distCoeffs,
                        cv::Size& calibratedImageSize );

private:
  int m_CornersX;
  int m_CornersY;
  int m_CornersN;

  // parameter for calibration
  vector<CvPoint2D32f> m_TempPoints;
  vector<CvPoint3D32f> m_ObjectPoints;
  vector<CvPoint2D32f> m_Points;

  vector<int> m_CornerCount;
  vector<IplImage*> m_Images;
  vector<CvPoint2D32f*> m_Corners;

  CvMat* m_AllObjectPoints;
  CvMat* m_AllImagePoints;
  CvMat* m_AllDetectedPoints;
  CvSize m_ImageSize;

  CvMat* m_TranslationVector;
  CvMat* m_IntrinsicMatrix;
  CvMat* m_DistortionCoeffs;
  CvMat* m_RotMatrix;
  CvMat* m_RotVector;
};

#endif /* VALIDATE_CAMERA_CALIBRATION_H_ */