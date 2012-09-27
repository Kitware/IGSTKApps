#include "MainWidget.h"
#include "ui_MainWidget.h"
#include "MainWidget.moc"
#include "ImageCapture.h"
#include <QFileDialog>
#include <QMessageBox>
#include <qdir.h>
#include <QInputDialog>
#include <QTextBrowser>
#include <qfile.h>
#include <qtextstream.h>
#include <qscrollarea.h>

#include "../IGIConfigurationData.h"

MainWidget::MainWidget( QWidget * parent )
  :QWidget(parent), m_UI(new Ui::MainWidget)
{
  this->m_UI->setupUi(this);
  connect(m_UI->buttonCamera, SIGNAL(clicked()), this, SLOT(startCamera()));
  connect(m_UI->calibrateButton, SIGNAL(clicked()), this, SLOT(calibrateCamera()));
  this->m_Capture = new ImageCapture(this);
  connect(m_Capture, SIGNAL(error(const QString&)), this, SLOT(captureError(const QString&)));
  connect(m_Capture, SIGNAL(imageCaptured(const QImage&)), this, SLOT(displayImage(const QImage&)));
  connect(m_UI->buttonCapture, SIGNAL(clicked()), m_Capture, SLOT(captureToFile()));
  connect(m_UI->buttonValidateCalibration, SIGNAL(clicked()), this, SLOT(validateCalibration()));
  connect(m_UI->quitButton, SIGNAL(clicked()), this, SLOT(quitApp()));
  connect(m_UI->helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
  
  QDir currentDir = QDir::current();
  m_UI->workspace->setText("Current Workspace: " + currentDir.absolutePath() + "/tmp");

  // UI state initial
  m_UI->buttonCamera->setEnabled(true);
  m_UI->calibrateButton->setEnabled(true);
  m_UI->buttonCapture->setEnabled(false);
  m_UI->buttonValidateCalibration->setEnabled(false);
}

MainWidget::~MainWidget(){
  delete m_Capture;
  delete m_UI;
}

void MainWidget::startCamera(){
  
  if(this->m_Capture->captureFromCamera(-1))
  { 
    // UI state camera started
    m_UI->buttonCamera->setEnabled(true);
    m_UI->calibrateButton->setEnabled(true);
    m_UI->buttonCapture->setEnabled(true);
    m_UI->buttonValidateCalibration->setEnabled(true);
  }
}

void MainWidget::displayImage( const QImage& image ){
  m_Capture->stopTimer();
  const QPixmap pix = QPixmap::fromImage(image);
  const QSize size = m_UI->displayLabel->size();
  this->m_UI->displayLabel->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));
  QApplication::processEvents();
  m_Capture->startTimer();
} 

void MainWidget::getImagesFromCurrentDirectory()
{
  QDir currentDir = QDir::current();
  currentDir.setFilter( QDir::Files );

  QFileInfoList list = currentDir.entryInfoList();
  QFileInfo info;

  m_fileNames.clear();
  
  for (int i = 0; i < list.size(); ++i)
  {
    QFileInfo fileInfo = list.at(i);
    QString fName = fileInfo.fileName();
    if(fName.contains(".jpg"))
    {
      m_fileNames.push_back(fName.toStdString());
    }
  }
}

void MainWidget::calibrateCamera()
{
  QStringList files = QFileDialog::getOpenFileNames(
              this,
              "Select five or more images for calibration.",
              QDir::current().absolutePath() + "/tmp",
              "Images (*.jpg)");
  
  if(!files.count())
    return;

  m_Capture->stopTimer();
  if( files.size() >= IGIConfigurationData::NR_CALIBRATION_IMAGES )
  {
    m_fileNames.clear();
    QStringList list = files;
    QStringList::Iterator it = list.begin();
    while(it != list.end())
    {
      m_fileNames.push_back((*it).toStdString());(*it);
      ++it;
    }

    CameraCalibration *camCalib = new CameraCalibration();

    // Display the processed image on displayLabel
		camCalib->setDisplayCanvas(m_UI->displayLabel);

    camCalib->cornersX = 8;
    camCalib->cornersY = 5;
    camCalib->cornersN = camCalib->cornersX * camCalib->cornersY;
  
    vector<IplImage*> loadedImages = camCalib->setImageSeries(m_fileNames);

    if(camCalib->calibrate(loadedImages))
    {
      QDir currentDir = QDir::current();
      currentDir.cdUp();
      QString configDir = currentDir.absolutePath();
      QMessageBox::information(this,windowTitle(),
              QString("Camera successfully calibrated.\nCalibration file saved in: ") +
              configDir + "/" + IGIConfigurationData::CONFIGURATION_FOLDER +
              " [" + IGIConfigurationData::CAMERA_CALIBRATION_FILENAME +"]");
    }
    else
    {
    QMessageBox::information(this,windowTitle(),
              "Calibration failed. Calibration requires five or more images of the calibration grid, Aborting.");
    }
  }
  else
  {
    QMessageBox::information(this,windowTitle(),
              "Calibration requires five or more images, Aborting.");
  }
  m_Capture->startTimer();
}

void MainWidget::validateCalibration()
{
  ValidateCameraCalibration vcc;
  float squareSize = 30;
  std::string errorMessage = vcc.DetectChessboardCorners(m_Capture->getCurrentFrame(),cvSize(4,3),squareSize);
  if(errorMessage.size() > 0)
  {
    QMessageBox::critical(this,windowTitle(), QString(errorMessage.c_str()));
  }
}
void MainWidget::captureError( const QString& text ){
  QMessageBox::critical(this,windowTitle(), text);
}

/** Catch key press events from the GUI */
void MainWidget::keyPressEvent ( QKeyEvent * e)
{
  switch ( e->key() )
  {
  case Qt::Key_S:
    startCamera();
    break;
  case Qt::Key_Space:
    m_Capture->captureToFile();
    break;
  case Qt::Key_C:
    calibrateCamera();
    break;
  case Qt::Key_V:
    validateCalibration();
    break;
  case Qt::Key_H:
    showHelp();
    break;
  case Qt::Key_Q:
    quitApp();
    break;
  default:
  return;
  }
}

void MainWidget::showHelp()
{
  QDialog* helpBox = new QDialog(this);
  helpBox->setWindowTitle("Help");

  QTextBrowser* browser = new QTextBrowser(helpBox);
  browser->setSource(*new QUrl("READMECameraCalibration.html"));
  browser->setWindowTitle("Help");

  QPushButton* okButton = new QPushButton(helpBox);
  connect(okButton, SIGNAL(clicked()), helpBox, SLOT(close()));
  okButton->setGeometry(QRect(150, 260, 100, 25));
  okButton->setText("Quit");

  QVBoxLayout* helpLayout = new QVBoxLayout(helpBox);
  helpLayout->addWidget(browser);
  helpLayout->addWidget(okButton);

  helpBox->resize(500,500);
  helpBox->show();
}

void MainWidget::quitApp()
{
  qApp->quit();
}
