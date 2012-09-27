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

  bool calibrate(vector<IplImage*> imageSet);
  void saveCameraParams( const string& filename,
                CvSize imageSize, CvSize boardSize,
                float squareSize, float aspectRatio, int flags,
                const CvMat* cameraMatrix, const CvMat* distCoeffs,
                const vector<CvMat>& rvecs, const vector<CvMat>& tvecs);

  void setPoints(double squareSize);

  // Set the Canvas on which to display the processed image
	void setDisplayCanvas(QLabel *Canvas) {m_Canvas = Canvas;};

  IplImage* getGrayImage(IplImage* imageRGB);
  std::vector<IplImage*> setImageSeries(std::vector<std::string> filenames);

  int cornersX;
  int cornersY;
  int cornersN;

private:
	// Display the processed image on the canvas m_Canvas
	void displayImage(IplImage *image);
  int findChessboardCorners(IplImage* image);
  void calibrateCameraEnd(CvSize imageSize);

  // The canvas on which to draw processed images
	QLabel *m_Canvas;

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

#endif /* CAMERA_CALIBRATION_H_ */
