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
  this->m_Capture = new ImageCapture(this);
  
  connect(m_UI->buttonCamera, SIGNAL(clicked()), this, SLOT(StartCamera()));
  connect(m_UI->calibrateButton, SIGNAL(clicked()), this, SLOT(CalibrateCamera()));
  connect(m_Capture, SIGNAL(Error(const QString&)), this, SLOT(CaptureError(const QString&)));
  connect(m_Capture, SIGNAL(ImageCaptured(const QImage&)), this, SLOT(DisplayImage(const QImage&)));
  connect(m_UI->buttonCapture, SIGNAL(clicked()), this, SLOT(CaptureToFile()));
  connect(m_UI->buttonValidateCalibration, SIGNAL(clicked()), this, SLOT(ValidateCalibration()));
  connect(m_UI->quitButton, SIGNAL(clicked()), this, SLOT(QuitApp()));
  connect(m_UI->helpButton, SIGNAL(clicked()), this, SLOT(ShowHelp()));
  
  // get current application path
  QString path = QApplication::applicationDirPath();
  path.truncate(path.lastIndexOf("/Programs"));
  m_CurrentPath = path + "/Programs";
  m_ImageDirectory = m_CurrentPath.toStdString() + "/tmp";	
  
  m_ConfigDir = path + "/" + IGIConfigurationData::CONFIGURATION_FOLDER;
	
  m_UI->workspace->setText("Current Workspace: " + QString(m_ImageDirectory.c_str()));
	
  // UI state initialization
  m_UI->buttonCamera->setEnabled(true);
  m_UI->calibrateButton->setEnabled(true);
  m_UI->buttonCapture->setEnabled(false);
  m_UI->buttonValidateCalibration->setEnabled(false);
}

MainWidget::~MainWidget()
{
  delete m_Capture;
  delete m_UI;
}

void MainWidget::StartCamera()
{  
  if(this->m_Capture->CaptureFromCamera(-1))
  { 
    // UI state camera started
    m_UI->buttonCamera->setEnabled(true);
    m_UI->calibrateButton->setEnabled(true);
    m_UI->buttonCapture->setEnabled(true);
    m_UI->buttonValidateCalibration->setEnabled(true);
  }
}

void MainWidget::DisplayImage( const QImage& image )
{
  m_Capture->StopTimer();
  const QPixmap pix = QPixmap::fromImage(image);
  const QSize size = m_UI->displayLabel->size();
  this->m_UI->displayLabel->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));
  QApplication::processEvents();
  m_Capture->StartTimer();
} 

void MainWidget::CaptureToFile()
{
  m_Capture->CaptureToFile(m_ImageDirectory);
}

void MainWidget::CalibrateCamera()
{
  QStringList files = QFileDialog::getOpenFileNames(
              this,
              "Select five or more images for calibration.",
              QString(m_ImageDirectory.c_str()),
              "Images (*.jpg)");
  
  if(!files.count())
    return;

  m_Capture->StopTimer();
  if( files.size() >= IGIConfigurationData::NR_CALIBRATION_IMAGES )
  {
    m_FileNames.clear();
    QStringList list = files;
    QStringList::Iterator it = list.begin();
    while(it != list.end())
    {
      m_FileNames.push_back((*it).toStdString());(*it);
      ++it;
    }

    CameraCalibration *camCalib = new CameraCalibration();

    // Display the processed image on displayLabel
		camCalib->SetDisplayCanvas(m_UI->displayLabel);

    camCalib->m_CornersX = IGIConfigurationData::CHECKERBOARD_WIDTH;
    camCalib->m_CornersY = IGIConfigurationData::CHECKERBOARD_HEIGHT;
    camCalib->m_CornersN = camCalib->m_CornersX * camCalib->m_CornersY;
  
    std::vector<IplImage*> loadedImages = camCalib->SetImageSeries(m_FileNames);
   
    if(camCalib->StartCalibration(loadedImages, m_ConfigDir.toStdString(), IGIConfigurationData::CAMERA_CALIBRATION_FILENAME))
    {
      QMessageBox::information(this,windowTitle(),
              QString("Camera successfully calibrated.\nCalibration file saved in: ") +
              m_ConfigDir +
              " [" + IGIConfigurationData::CAMERA_CALIBRATION_FILENAME +"]");
    }
    else
    {
      QMessageBox::information(this,windowTitle(),
              "Calibration failed. Calibration requires five or more images of the calibration grid, Aborting.");
    }
    while(!loadedImages.empty())
    {
      cvReleaseImage(&loadedImages.back());
      loadedImages.pop_back();
    }
  }
  else
  {
    QMessageBox::information(this,windowTitle(),
              "Calibration requires five or more images, Aborting.");
  }
 
  m_Capture->StartTimer();
}

void MainWidget::ValidateCalibration()
{
  ValidateCameraCalibration vcc;
  float squareSize = 30;
  
  std::string camCalibFile = m_ConfigDir.toStdString()
                             + std::string("/")
                             + std::string(IGIConfigurationData::CAMERA_CALIBRATION_FILENAME);
  
  std::string errorMessage = vcc.Validate(m_Capture->GetCurrentFrame(),
                                          cvSize(4,3),squareSize,
                                          camCalibFile);
  if(errorMessage.size() > 0)
  {
    QMessageBox::critical(this,windowTitle(), QString(errorMessage.c_str()));
  }
}

void MainWidget::CaptureError( const QString& text )
{
  QMessageBox::critical(this,windowTitle(), text);
}

/** Catch key press events from the GUI */
void MainWidget::keyPressEvent ( QKeyEvent * e)
{
  switch ( e->key() )
  {
  case Qt::Key_S:
    StartCamera();
    break;
  case Qt::Key_Space:
    m_Capture->CaptureToFile(m_ImageDirectory);
    break;
  case Qt::Key_C:
    CalibrateCamera();
    break;
  case Qt::Key_V:
    ValidateCalibration();
    break;
  case Qt::Key_H:
    ShowHelp();
    break;
  case Qt::Key_Q:
    QuitApp();
    break;
  default:
  return;
  }
}

void MainWidget::ShowHelp()
{
  QDialog* helpBox = new QDialog(this);
  helpBox->setWindowTitle("Help");

  QTextBrowser* browser = new QTextBrowser(helpBox);
  browser->setSource(QUrl::fromLocalFile(m_CurrentPath + "/READMECameraCalibration.html"));
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

void MainWidget::QuitApp()
{
  qApp->quit();
}
