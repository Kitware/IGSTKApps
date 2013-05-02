#include "ValidateCameraCalibration.h"

#include "../IGIConfigurationData.h"

ValidateCameraCalibration::ValidateCameraCalibration()
{}

ValidateCameraCalibration::~ValidateCameraCalibration()
{}

std::string ValidateCameraCalibration::
  Validate(IplImage* img,cv::Size boardSize, float squareSize, std::string camCalibFile)
{
  IplImage* imgGrayscale = cvCreateImage(cvGetSize(img), 8, 1);
  cvCvtColor(img, imgGrayscale, CV_BGR2GRAY);
  std::vector<cv::Point2f> ptvec;
  ptvec.resize(boardSize.width * boardSize.height);
  bool found = cv::findChessboardCorners(cv::Mat(imgGrayscale),
                                         boardSize, 
                                         ptvec, 
                                         0);

  std::vector<cv::Point3f> points3D;
    for( int i = 0; i < boardSize.height; i++ )
        for( int j = 0; j < boardSize.width; j++ )
            points3D.push_back(cv::Point3f(float(j*squareSize),
                                       float(i*squareSize),(float) 0));

  cv::Size calibratedImageSize;
  cv::Mat_<float> camera_matrix (3, 3);
  cv::Mat_<float> dist_coef (5, 1);

  bool readCameraParams;
  readCameraParams = ReadCameraMatrix(camCalibFile,
                                      camera_matrix, dist_coef,
                                      calibratedImageSize);
  if(!readCameraParams)
    return "Camera calibration file not found.";
	
  cv::Mat rotation_vector_left (3, 1, CV_64F);
  cv::Mat translation_vector_left (3, 1, CV_64F);
  cv::Mat rotation_vector_right (3, 1, CV_64F);
  cv::Mat translation_vector_right (3, 1, CV_64F);

  if(ptvec.size() != (boardSize.width*boardSize.height))
  {
    return "Validation pattern not detected.\nPosition validation pattern properly in front of the camera.";
  }

  std::vector<cv::Point3f> points3DleftRectangle;
  points3DleftRectangle.push_back(points3D.at(0));
  points3DleftRectangle.push_back(points3D.at(1));
  points3DleftRectangle.push_back(points3D.at(4));
  points3DleftRectangle.push_back(points3D.at(5));
  points3DleftRectangle.push_back(points3D.at(8));
  points3DleftRectangle.push_back(points3D.at(9));

  std::vector<cv::Point3f> points3DrightRectangle;
  points3DrightRectangle.push_back(points3D.at(2));
  points3DrightRectangle.push_back(points3D.at(3));
  points3DrightRectangle.push_back(points3D.at(6));
  points3DrightRectangle.push_back(points3D.at(7));
  points3DrightRectangle.push_back(points3D.at(10));
  points3DrightRectangle.push_back(points3D.at(11));

  std::vector<cv::Point2f> points2DleftImageRectangle;
  points2DleftImageRectangle.push_back(ptvec.at(0));
  points2DleftImageRectangle.push_back(ptvec.at(1));
  points2DleftImageRectangle.push_back(ptvec.at(4));
  points2DleftImageRectangle.push_back(ptvec.at(5));
  points2DleftImageRectangle.push_back(ptvec.at(8));
  points2DleftImageRectangle.push_back(ptvec.at(9));

  std::vector<cv::Point2f> points2DrightImageRectangle;
  points2DrightImageRectangle.push_back(ptvec.at(2));
  points2DrightImageRectangle.push_back(ptvec.at(3));
  points2DrightImageRectangle.push_back(ptvec.at(6));
  points2DrightImageRectangle.push_back(ptvec.at(7));
  points2DrightImageRectangle.push_back(ptvec.at(10));
  points2DrightImageRectangle.push_back(ptvec.at(11));
	
  solvePnP(points3DleftRectangle,
           points2DleftImageRectangle,
           camera_matrix,
           dist_coef,
           rotation_vector_left,
           translation_vector_left );

  float one3D[] = {1.0, 0.0, 0.0};
  cv::Mat oneVector = cv::Mat(1,1,CV_32FC3, one3D).clone();

  cv::Mat modif_points_left;
  modif_points_left.create(1, 1, CV_32FC3);
  cv::Mat R_left(3, 3, CV_64FC1);
  cv::Rodrigues(rotation_vector_left, R_left);
  cv::Mat transformation_left(3, 4, CV_64F);
  cv::Mat r_left = transformation_left.colRange(0, 3);
  R_left.copyTo(r_left);
  cv::Mat t_left = transformation_left.colRange(3, 4);
  translation_vector_left.copyTo(t_left);
  transform(oneVector, modif_points_left, transformation_left);

  solvePnP(points3DleftRectangle,
           points2DrightImageRectangle,
           camera_matrix, dist_coef,
           rotation_vector_right,
           translation_vector_right );

  cv::Mat modif_points_right;
  modif_points_right.create(1, 1, CV_32FC3);
  cv::Mat R_right(3, 3, CV_64FC1);
  cv::Rodrigues(rotation_vector_right, R_right);
  cv::Mat transformation_right(3, 4, CV_64F);
  cv::Mat r_right = transformation_right.colRange(0, 3);
  R_right.copyTo(r_right);
  cv::Mat t_right = transformation_right.colRange(3, 4);
  translation_vector_right.copyTo(t_right);
  transform(oneVector, modif_points_right, transformation_right);

  cv::Mat displacement(1,1,CV_32FC3);
  displacement = modif_points_left - modif_points_right;
  double distance = cv::norm(displacement);

  std::stringstream dist;
  dist << distance;
  cv::Mat imageWithAnnotation(img);
  cv::putText(imageWithAnnotation,dist.str().c_str(), points2DleftImageRectangle.at(3),cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,0,255,255),2);
  cv::line(imageWithAnnotation,points2DleftImageRectangle.at(0),points2DrightImageRectangle.at(0), CV_RGB(255,0,0), 3, 8 );
  cv::putText(imageWithAnnotation,"Press any key to close window...",cv::Point(20,20),cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0,255),2);

  cvNamedWindow("Displacement",1);
  cvShowImage("Displacement", img);
  cvWaitKey(0);
  cvDestroyWindow("Displacement");

  cvReleaseImage(&imgGrayscale);
  return "";
}

bool ValidateCameraCalibration::ReadCameraMatrix(const std::string& filename,
                             cv::Mat& mCam32, cv::Mat& mdist32,
                             cv::Size& calibratedImageSize )
{
  cv::Mat cameraMatrix, distCoeffs;
  cv::FileStorage fs(filename, cv::FileStorage::READ);

  if(!fs.isOpened())
    return false;

  fs["image_width"] >> calibratedImageSize.width;
  fs["image_height"] >> calibratedImageSize.height;
  fs["distortion_coefficients"] >> distCoeffs;
  fs["camera_matrix"] >> cameraMatrix;

  if (cameraMatrix.type()!=CV_32FC1)
    cameraMatrix.convertTo(mCam32,CV_32FC1);

  distCoeffs.convertTo(mdist32,CV_32FC1);
  return true;
}
