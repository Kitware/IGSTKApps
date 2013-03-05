#ifndef CAMERA_CALIBRATION_H_
#define CAMERA_CALIBRATION_H_

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <QLabel>

using namespace std;

class CameraCalibration
{
public:
  CameraCalibration();
  virtual ~CameraCalibration();
  void SetPoints(double squareSize);
  // Set the Canvas on which to display the processed image
	void SetDisplayCanvas(QLabel *Canvas) {m_Canvas = Canvas;};
  IplImage* GetGrayImage(IplImage* imageRGB);
  std::vector<IplImage*> SetImageSeries(std::vector<std::string> filenames);
  bool StartCalibration(vector<IplImage*> imageSet, std::string savePath, std::string saveName);

  void SaveCameraParams( const string& filename,
                          CvSize imageSize, CvSize boardSize,
                          float squareSize, float aspectRatio, int flags,
                          const CvMat* cameraMatrix, const CvMat* distCoeffs,
                          const vector<CvMat>& rvecs, const vector<CvMat>& tvecs);

  int m_CornersX;
  int m_CornersY;
  int m_CornersN;

private:
  // Display the processed image on the canvas m_Canvas
  void DisplayImage(IplImage *image);
  int FindChessboardCorners(IplImage* image);
  void CalibrateCamera(CvSize imageSize);

  // The canvas on which to draw processed images
  QLabel *m_Canvas;

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

#endif /* CAMERA_CALIBRATION_H_ */
