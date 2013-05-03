#ifndef CAMERA_CALIBRATION_H_
#define CAMERA_CALIBRATION_H_

#ifdef _WIN32
  #include <windows.h>
#endif

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
  bool StartCalibration(std::vector<IplImage*> imageSet, std::string savePath, std::string saveName);

  void SaveCameraParams( const std::string& filename,
                          CvSize imageSize, CvSize boardSize,
                          float squareSize, float aspectRatio, int flags,
                          const CvMat* cameraMatrix, const CvMat* distCoeffs,
                          const std::vector<CvMat>& rvecs, const std::vector<CvMat>& tvecs);

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
  std::vector<CvPoint2D32f> m_TempPoints; 
  std::vector<CvPoint3D32f> m_ObjectPoints;
  std::vector<CvPoint2D32f> m_Points;

  std::vector<int> m_CornerCount;
  std::vector<IplImage*> m_Images;
  std::vector<CvPoint2D32f*> m_Corners;

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
