#include "ValidateCameraCalibration.h"

#include "../IGIConfigurationData.h"

ValidateCameraCalibration::ValidateCameraCalibration()
{}

ValidateCameraCalibration::~ValidateCameraCalibration()
{}

void ValidateCameraCalibration::DetectCorners(IplImage* img)
{
  IplImage* imgGrayscale = cvCreateImage(cvGetSize(img), 8, 1);
  cvCvtColor(img, imgGrayscale, CV_BGR2GRAY);

  // Create temporary images required by cvGoodFeaturesToTrack
  IplImage* imgTemp = cvCreateImage(cvGetSize(img), 32, 1);
  IplImage* imgEigen = cvCreateImage(cvGetSize(img), 32, 1);

  // Create the array
  int count = 100;
  CvPoint2D32f* corners = new CvPoint2D32f[count];

  // Find corners
  cvGoodFeaturesToTrack(imgGrayscale, imgEigen, imgTemp, corners, &count, 0.1, 20);

  // Mark these corners on the original image
  for(int i=0;i<count;i++)
  {
    cvLine(img, cvPoint(corners[i].x, corners[i].y), cvPoint(corners[i].x, corners[i].y), CV_RGB(255,0,0), 5);
  }

  // Display it
  cvNamedWindow("Original");
  cvShowImage("Original", img);

  // Print the number of corners
  printf("Count: %d\n", count);
}

void ValidateCameraCalibration::DetectLines(IplImage* img)
{
  IplImage* imgGrayscale = cvCreateImage(cvGetSize(img), 8, 1);
  cvCvtColor(img, imgGrayscale, CV_BGR2GRAY);

  IplImage* dst = cvCreateImage( cvGetSize(imgGrayscale), 8, 1 );
  IplImage* color_dst = cvCreateImage( cvGetSize(imgGrayscale), 8, 3 );
  CvMemStorage* storage = cvCreateMemStorage(0);
  CvSeq* lines = 0;
  int i;
  cvCanny( imgGrayscale, dst, 50, 200, 3 );
  cvCvtColor( dst, color_dst, CV_GRAY2BGR );

  lines = cvHoughLines2( dst,
                      storage,
                      CV_HOUGH_STANDARD,
                      1,// rho
                      CV_PI/180, //theta
                      100,
                      0,
                      0 );

  for( i = 0; i < MIN(lines->total,100); i++ )
  {
    float* line = (float*)cvGetSeqElem(lines,i);
    float rho = line[0];
    float theta = line[1];
    CvPoint pt1, pt2;
    double a = cos(theta), b = sin(theta);
    double x0 = a*rho, y0 = b*rho;
    pt1.x = cvRound(x0 + 1000*(-b));
    pt1.y = cvRound(y0 + 1000*(a));
    pt2.x = cvRound(x0 - 1000*(-b));
    pt2.y = cvRound(y0 - 1000*(a));
    cvLine( color_dst, pt1, pt2, CV_RGB(255,0,0), 3, 8 );
  }

  cvNamedWindow( "Source", 1 );
  cvShowImage( "Source", img );

  cvNamedWindow( "Hough", 1 );
  cvShowImage( "Hough", color_dst );
}

std::string ValidateCameraCalibration::DetectChessboardCorners(IplImage* img,cv::Size boardSize, float squareSize)
{
  IplImage* imgGrayscale = cvCreateImage(cvGetSize(img), 8, 1);
  cvCvtColor(img, imgGrayscale, CV_BGR2GRAY);
  vector<cv::Point2f> ptvec;
  ptvec.resize(boardSize.width * boardSize.height);
  bool found = cv::findChessboardCorners(cv::Mat(imgGrayscale), boardSize, ptvec, 0);

  vector<cv::Point3f> points3D;
    for( int i = 0; i < boardSize.height; i++ )
        for( int j = 0; j < boardSize.width; j++ )
            points3D.push_back(cv::Point3f(float(j*squareSize),
                                       float(i*squareSize),(float) 0));

  cv::Size calibratedImageSize;
  cv::Mat_<float> camera_matrix (3, 3);
  cv::Mat_<float> dist_coef (5, 1);

  bool readCameraParams;
  readCameraParams = readCameraMatrix(std::string("../")
              + std::string(IGIConfigurationData::CONFIGURATION_FOLDER)
              + std::string("/")
              + std::string(IGIConfigurationData::CAMERA_CALIBRATION_FILENAME),
                   camera_matrix, dist_coef,
                   calibratedImageSize);
  if(!readCameraParams)
    return "Camera calibration file not found.";

  cv::Mat_<float> rotation_vector_left (3, 1);
  cv::Mat_<float> translation_vector_left (3, 1);
  cv::Mat_<float> rotation_vector_right (3, 1);
  cv::Mat_<float> translation_vector_right (3, 1);

  if(ptvec.size() != (boardSize.width*boardSize.height))
  {
    return "Validation pattern not detected.\nPosition validation pattern properly in front of the camera.";
  }

  vector<cv::Point3f> points3DleftRectangle;
  points3DleftRectangle.push_back(points3D.at(0));
  points3DleftRectangle.push_back(points3D.at(1));
  points3DleftRectangle.push_back(points3D.at(4));
  points3DleftRectangle.push_back(points3D.at(5));
  points3DleftRectangle.push_back(points3D.at(8));
  points3DleftRectangle.push_back(points3D.at(9));

  vector<cv::Point3f> points3DrightRectangle;
  points3DrightRectangle.push_back(points3D.at(2));
  points3DrightRectangle.push_back(points3D.at(3));
  points3DrightRectangle.push_back(points3D.at(6));
  points3DrightRectangle.push_back(points3D.at(7));
  points3DrightRectangle.push_back(points3D.at(10));
  points3DrightRectangle.push_back(points3D.at(11));

  vector<cv::Point2f> points2DleftImageRectangle;
  points2DleftImageRectangle.push_back(ptvec.at(0));
  points2DleftImageRectangle.push_back(ptvec.at(1));
  points2DleftImageRectangle.push_back(ptvec.at(4));
  points2DleftImageRectangle.push_back(ptvec.at(5));
  points2DleftImageRectangle.push_back(ptvec.at(8));
  points2DleftImageRectangle.push_back(ptvec.at(9));

  vector<cv::Point2f> points2DrightImageRectangle;
  points2DrightImageRectangle.push_back(ptvec.at(2));
  points2DrightImageRectangle.push_back(ptvec.at(3));
  points2DrightImageRectangle.push_back(ptvec.at(6));
  points2DrightImageRectangle.push_back(ptvec.at(7));
  points2DrightImageRectangle.push_back(ptvec.at(10));
  points2DrightImageRectangle.push_back(ptvec.at(11));

  solvePnP( points3DleftRectangle, points2DleftImageRectangle, camera_matrix, dist_coef, rotation_vector_left, translation_vector_left );

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

  solvePnP( points3DleftRectangle, points2DrightImageRectangle, camera_matrix, dist_coef, rotation_vector_right, translation_vector_right );

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

  cvNamedWindow("Displacement",1);
  cvShowImage("Displacement", img);
  cvWaitKey(0);
  cvDestroyWindow("Displacement");
  return "";
}

bool ValidateCameraCalibration::readCameraMatrix(const string& filename,
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
