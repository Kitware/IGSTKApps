#include "IGI3DDigitizerGUI.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "itkTextOutput.h"

#include <vtkFileOutputWindow.h>
#include <vtkOutputWindow.h>
#include <vtkSmartPointer.h>

#include <QMessageBox>

#include "igstkTransformObserver.h"
#include "igstkTransformFileWriter.h"
#include "igstkRigidTransformXMLFileWriter.h"

#include "../IGIConfigurationData.h"

#define VIEW_3D_REFRESH_RATE 10
#define DEFAULT_ZOOM_FACTOR 1.5
#define TRACKER_FREQUENCY 20

/** ---------------------------------------------------------------
*     Constructor
* -----------------------------------------------------------------
*/
IGI3DDigitizerGUI::IGI3DDigitizerGUI()
{
  /** Initializations */
  m_GUI.setupUi(this);
  this->CreateActions();
  m_GUIQuit = false;
  
  // get current application path
  QString path = QApplication::applicationDirPath();
  QDir currentDir = QDir(path);
  m_CurrentPath = currentDir.absolutePath();  
  currentDir.cdUp();
  m_ConfigDir = currentDir.absolutePath() + "/" + IGIConfigurationData::CONFIGURATION_FOLDER;  
  
  m_GUI.LEDlabelDRF->hide();
  m_GUI.LEDlabelTool->hide();

  // suppress itk output window
  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  vtkSmartPointer<vtkFileOutputWindow> fileOutputWindow =
      vtkSmartPointer<vtkFileOutputWindow>::New();
  fileOutputWindow->SetFileName( "output.txt" );

  vtkOutputWindow* outputWindow = vtkOutputWindow::GetInstance();
  if ( outputWindow )
  {
    outputWindow->SetInstance( fileOutputWindow );
  }

  m_WorldReference  = igstk::AxesObject::New();
  m_WorldReference->SetSize(100.0, 100.0, 100.0);

  QTimer *pulseTimer = new QTimer();
  connect(pulseTimer, SIGNAL(timeout()), this, SLOT(PulseTimerEvent()));
  pulseTimer->start(10);

  m_PointAquisitionPulseTimer = new QTimer();
  connect(m_PointAquisitionPulseTimer, SIGNAL(timeout()), this, SLOT(PointAquisition()));
  m_PointAquisitionStarted = false;

  m_PointCoordsAnnotation = igstk::Annotation2D::New();

  SetupView();

  m_FiducialSet = new FiducialSet(this);
  m_FiducialSet->AddView("view3D",m_View3D.GetPointer());

  // GUI status
  m_GUI.CheckPositionButton->setEnabled(false);
  m_GUI.StartButton->setEnabled(false);
  m_GUI.SaveAsButton->setEnabled(false);
  m_GUI.ClearButton->setEnabled(false);
  m_GUI.editPointNr->setText(QString::number(0));
  m_GUI.editVisNr->setText(QString::number(0));
}

/** -----------------------------------------------------------------
*     Connect GUI elements with actions
*  -----------------------------------------------------------------
*/
void IGI3DDigitizerGUI::CreateActions()
{
  connect(m_GUI.QuitPushButton, SIGNAL(clicked()), this, SLOT(OnQuitAction()));
  connect(m_GUI.helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  connect(m_GUI.CheckPositionButton, SIGNAL(clicked()), this, SLOT(CheckMarkerPosition()));
  connect(m_GUI.StartButton, SIGNAL(clicked()), this, SLOT(TogglePointAquisition()));
  connect(m_GUI.ClearButton, SIGNAL(clicked()), this, SLOT(ClearScene()));

  connect(m_GUI.LoadButton, SIGNAL(clicked()), this, SLOT(LoadPoints()));
  connect(m_GUI.SaveAsButton, SIGNAL(clicked()), this, SLOT(SavePoints()));

  connect(m_GUI.PointerIDBox, SIGNAL(currentIndexChanged(int)), this, SLOT(HandlePointerIDChanged(int)));
  connect(m_GUI.RefIDBox, SIGNAL(currentIndexChanged(int)), this, SLOT(HandleRefIDChanged(int)));

  connect(m_GUI.CameraCalibFileButton, SIGNAL( clicked()), this, SLOT(SelectCameraCalibrationFile()));
  connect(m_GUI.PointerCalibFileButton, SIGNAL( clicked()), this, SLOT(SelectPointerToolCalibrationFile()));

  connect(m_GUI.InitializeButton, SIGNAL( clicked()), this, SLOT(StartInitialization()));
}

void IGI3DDigitizerGUI::SelectCameraCalibrationFile()
{
  QString fName = QFileDialog::getOpenFileName(this,
      "Select Camera Calibration File.",
      m_ConfigDir,
      "Camera Calibration File (*.yml)");

  if(fName.isNull())
    return;
  if (fName.isEmpty())
  {
    handleMessage("No camera calibration file [*.yml] was selected.\n",1);
    return;
  }

  m_GUI.CameraCalibFileEdit->setText(fName);
  m_CameraParametersFile = fName.toStdString(); 
}

void IGI3DDigitizerGUI::SelectPointerToolCalibrationFile()
{
  QString fName = QFileDialog::getOpenFileName(this,
      "Select Pointer Tool Calibration File.",
      m_ConfigDir,
      "Pointer Tool Calibration File (*.xml)");

  if(fName.isNull())
    return;
  if (fName.isEmpty())
  {
    handleMessage("No file was selected\n",1);
    return;
  }

  m_GUI.PointerCalibFileEdit->setText(fName);
  m_PointerToolCalibrationFile = fName;
}

void IGI3DDigitizerGUI::HandlePointerIDChanged(int ID)
{

}

void IGI3DDigitizerGUI::HandleRefIDChanged(int ID)
{

}

void IGI3DDigitizerGUI::SetupView()
{
  //Create view
  m_View3D = View3DType::New();

  // Set IGSTK view for Qt display
  m_GUI.Display3D->RequestSetView (m_View3D);

  // Enable Qt display interaction
  m_GUI.Display3D->RequestEnableInteractions();

  // set background color to the views
  m_View3D->SetRendererBackgroundColor(1,1,1);
    
  ConnectImageRepresentation();
  ResetCamera();
}

/** -----------------------------------------------------------------
* Set ImageSpatialObjects to
* ImageRepresentations, sets image orientations, adds ImageSpatialObjects
* to Views and connects the scene graph
*---------------------------------------------------------------------
*/
void IGI3DDigitizerGUI::ConnectImageRepresentation()
{
  igstk::Transform identity;
  identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

  m_View3D->RequestSetTransformAndParent( identity, m_WorldReference );

  // set up view parameters
  m_View3D->SetRefreshRate( VIEW_3D_REFRESH_RATE );
  m_View3D->SetCameraZoomFactor(DEFAULT_ZOOM_FACTOR);
  m_View3D->SetCameraPosition( 800.0, 800.0, 800.0 );
  m_View3D->SetCameraFocalPoint( 0.0, 0.0, 0.0);
  
  // reset the cameras in the different views
  ResetCamera();
  m_View3D->RequestStart();
  ResetCamera();
}

bool IGI3DDigitizerGUI::InitializeTrackerAction()
{
  igstk::Transform identity;
  identity.SetToIdentity(igstk::TimeStamp::GetLongestPossibleTime());

  this->m_ErrorObserver = NULL;
  //create error observer
  this->m_ErrorObserver = ErrorObserver::New();
  //create tracker
  m_ArucoTracker=NULL;
  m_ArucoTracker = igstk::ArucoTracker::New(); 
  m_ArucoTracker->RequestSetFrequency( TRACKER_FREQUENCY );
  m_ArucoTracker->RequestSetTransformAndParent(identity, m_WorldReference);

  // Set marker size in mm
  this->m_ArucoTracker->SetMarkerSize(50);

  if(m_CameraParametersFile.empty())
  {
    handleMessage( "Camera calibration file not defined.\n" , 1 );
    return false;
  }
  
  // load camera calibration file
  m_ArucoTracker->SetCameraParametersFromYAMLFile(m_CameraParametersFile);
 
  //observe all possible errors generated by the tracker
  unsigned long observerID = m_ArucoTracker->AddObserver(
                             igstk::IGSTKErrorEvent(), this->m_ErrorObserver );
  m_ArucoTracker->RequestOpen();
  m_ArucoTracker->RemoveObserver( observerID );

  if( this->m_ErrorObserver->ErrorOccured() )
  {
    this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
    this->m_ErrorObserver->ClearError();
    handleMessage( this->m_ErrorMessage, 1 );
    return false;
  }
  else  //attach the tools 
  {
    m_ArucoTrackerTool = igstk::ArucoTrackerTool::New();
    m_ArucoTrackerTool->RequestSetMarkerName(m_PointerId.toInt());
    m_ArucoTrackerTool->SetCalibrationTransform(m_ToolCalibrationTransform);
    m_ArucoTrackerTool->RequestConfigure();
    m_ArucoTrackerTool->RequestAttachToTracker( m_ArucoTracker );

    // TrackerToolAvailableObserver
    m_TrackerToolAvailableObserver = LoadedObserverType::New();
    m_TrackerToolAvailableObserver->SetCallbackFunction( this,
                          &IGI3DDigitizerGUI::ToolAvailableCallback );
    m_ArucoTrackerTool->AddObserver(
        igstk::TrackerToolMadeTransitionToTrackedStateEvent(),
                      m_TrackerToolAvailableObserver);

    // TrackerToolNotAvailableObserver
    m_TrackerToolNotAvailableObserver = LoadedObserverType::New();
    m_TrackerToolNotAvailableObserver->SetCallbackFunction(
              this, &IGI3DDigitizerGUI::ToolNotAvailableCallback );
      
    m_ArucoTrackerTool->AddObserver(
              igstk::TrackerToolNotAvailableToBeTrackedEvent(),
                m_TrackerToolNotAvailableObserver);

    if(m_ReferenceId.toInt() != 0)
    {
      m_ArucoReferenceTrackerTool = igstk::ArucoTrackerTool::New();
      m_ArucoReferenceTrackerTool->RequestSetMarkerName(m_ReferenceId.toInt());
      m_ArucoReferenceTrackerTool->RequestConfigure();
      m_ArucoReferenceTrackerTool->RequestAttachToTracker( m_ArucoTracker );
      m_ArucoReferenceTrackerTool->RequestSetTransformAndParent(identity, m_WorldReference);

      m_ArucoTracker->RequestSetReferenceTool( m_ArucoReferenceTrackerTool );

      m_ReferenceNotAvailableObserver = LoadedObserverType::New();
      m_ReferenceNotAvailableObserver->SetCallbackFunction(this,
        &IGI3DDigitizerGUI::ReferenceNotAvailableCallback );
      m_ArucoReferenceTrackerTool->AddObserver(
                igstk::TrackerToolNotAvailableToBeTrackedEvent(),
                    m_ReferenceNotAvailableObserver);

      m_ReferenceAvailableObserver = LoadedObserverType::New();
      m_ReferenceAvailableObserver->SetCallbackFunction( this,
        &IGI3DDigitizerGUI::ReferenceAvailableCallback );

      m_ArucoReferenceTrackerTool->AddObserver(
              igstk::TrackerToolMadeTransitionToTrackedStateEvent(),
                    m_ReferenceAvailableObserver);


    }
  }
 
  m_ArucoTracker->AddObserver(igstk::TrackerStartTrackingEvent(),
  this->m_ErrorObserver);

  m_ArucoTracker->RequestStartTracking();

  //check that start was successful
  if( this->m_ErrorObserver->ErrorOccured() )
  {
    this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
    this->m_ErrorObserver->ClearError();
    handleMessage( "Tracker start error\n" , 1 );
    return false;
  }
 
  m_GUI.LEDlabelTool->show();
  m_GUI.LEDlabelDRF->show();
  if(m_ReferenceId.toInt() == 0)
    m_GUI.LEDlabelDRF->setDisabled(true);
  m_GUI.CameraCalibFileButton->setDisabled(true);
  m_GUI.PointerCalibFileButton->setDisabled(true);
  m_GUI.PointerIDBox->setDisabled(true);
  m_GUI.RefIDBox->setDisabled(true);

  return true;
}

bool IGI3DDigitizerGUI::LoadToolCalibrationTransform(QString transformFile)
{
  igstk::PrecomputedTransformData::Pointer transformData;
  igstk::TransformXMLFileReaderBase::Pointer xmlFileReader;
  xmlFileReader = igstk::RigidTransformXMLFileReader::New();
  igstk::TransformFileReader::Pointer transformFileReader = igstk::TransformFileReader::New();
  transformFileReader->RequestSetReader( xmlFileReader );
  transformFileReader->RequestSetFileName( transformFile.toStdString() );
  
  //observer for the read success and failure events
  igstk::TransformFileReader::ReadObserver::Pointer readObserver = igstk::TransformFileReader::ReadObserver::New();
  //observer for the get data event (initiated by the readObserver)
  TransformDataObserver::Pointer getDataObserver = TransformDataObserver::New();
  //add our observers
  transformFileReader->AddObserver( igstk::TransformFileReader::ReadFailureEvent(),
                                  readObserver );
  transformFileReader->AddObserver( igstk::TransformFileReader::ReadSuccessEvent(),
                                  readObserver );
  transformFileReader->AddObserver( igstk::TransformFileReader::TransformDataEvent(),
                                  getDataObserver );

  transformFileReader->RequestRead();

  if( readObserver->GotReadFailure() )
  {
    std::cerr<<readObserver->GetErrorMessage()<<"\n";
    handleMessage("Could not read tool calibration file.", 1 );
    return false;
  }

  igstk::Transform toolCalibrationTransform;

  if( getDataObserver->GotTransformData() ) 
  {
    transformData = getDataObserver->GetTransformData();
       
    TransformRequestObserver::Pointer transformObserver = 
                                            TransformRequestObserver::New();
    transformData->AddObserver(igstk::PrecomputedTransformData::TransformTypeEvent(), 
                              transformObserver );

    TransformErrorObserver::Pointer transformErrorObserver = 
                                            TransformErrorObserver::New();
    
    transformData->AddObserver(igstk::PrecomputedTransformData::TransformErrorTypeEvent(), 
                                transformErrorObserver );
    
    transformData->RequestTransform();
    if( transformObserver->GotTransformRequest() )
    {
      igstk::TransformBase *transformData = 
        transformObserver->GetTransformRequest();
      igstk::Transform *toolCalibrationTransform = 
        dynamic_cast<igstk::Transform*>( transformData );
      m_ToolCalibrationTransform = *toolCalibrationTransform;
    }
    else
    {
      handleMessage("Could not retrieve transformation information from tool calibration file.", 1);
      return false;
    }
  }
  else
  {
    handleMessage("Could not retrieve transformation information from tool calibration file.", 1);
    return false;
  }
  return true;
}

void IGI3DDigitizerGUI::TogglePointAquisition()
{
  if(m_PointAquisitionStarted)
  {
    m_PointAquisitionStarted = false;
    m_PointAquisitionPulseTimer->stop();
    m_GUI.StartButton->setText("Start");
    m_GUI.SaveAsButton->setEnabled(true);
    m_GUI.ClearButton->setEnabled(true);
    m_GUI.LoadButton->setEnabled(true);
  }
  else
  {
    unsigned int delay = 5000;
    std::ostringstream msg;
    for(unsigned int i=delay; i>0; i-=1000 )
    {
      delay-=1000;
      msg << "Data acquisition starts in "<<(int)(i/1000)<<" seconds.";
      m_GUI.StartButton->setText(QString(msg.str().c_str()));
      QApplication::processEvents();
      igstk::PulseGenerator::Sleep(1000);
      msg.str("");
    }
    m_PointAquisitionStarted = true;
    m_GUI.StartButton->setText("Stop");
    m_PointAquisitionPulseTimer->start(TRACKER_FREQUENCY);
    m_GUI.SaveAsButton->setEnabled(false);
    m_GUI.ClearButton->setEnabled(false);
    m_GUI.LoadButton->setEnabled(false);

    QApplication::processEvents();
  }
}

void IGI3DDigitizerGUI::ClearScene()
{
  m_FiducialSet->Clear();
  m_CurrentPointSetSaved = false;
  m_GUI.editPointNr->setText(QString::number(0));
  m_GUI.editVisNr->setText(QString::number(0));
}

void IGI3DDigitizerGUI::PointAquisition()
{
  // get transform
  typedef igstk::TransformObserver ObserverType;
  ObserverType::Pointer transformObserver = ObserverType::New();
  transformObserver->ObserveTransformEventsFrom( m_ArucoTrackerTool );
  transformObserver->Clear();
  
  if(m_ReferenceId.toInt() == 0)
    m_ArucoTrackerTool->RequestComputeTransformTo( m_WorldReference );
  else
    m_ArucoTrackerTool->RequestComputeTransformTo( m_ArucoReferenceTrackerTool );
  


  if( !transformObserver->GotTransform() )
  {  
    //if error occurs we silently continue
    return;
  }
  
  igstk::Transform transform;
  transform = transformObserver->GetTransform();

  if(transform.IsValidNow())
  {
    // store transform in the landmark container
    PointType point = TransformToPoint( transform );
    m_FiducialSet->AddNewFiducial(point,m_WorldReference);
    m_GUI.editPointNr->setText(QString::number(m_FiducialSet->GetNrOfPoints()));
    m_GUI.editVisNr->setText(QString::number(m_FiducialSet->GetNrOfVisualizedPoints()));
  }
}

/**
 *  Write the points to file
 */
void IGI3DDigitizerGUI::SavePoints()
{
  m_PointSetFilename = m_FiducialSet->SaveFiducials(m_ConfigDir);
  m_CurrentPointSetSaved = true;
}

/** Callback for tool available */
void IGI3DDigitizerGUI::ToolAvailableCallback(const itk::EventObject & event )
{
  m_ToolNotAvailable = false;
  m_GUI.LEDlabelTool->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/greenLED.png")));
}

/** Callback for tool not available */
void IGI3DDigitizerGUI::ToolNotAvailableCallback(const itk::EventObject & event )
{
  m_ToolNotAvailable = true;
  m_GUI.LEDlabelTool->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/redLED.png")));
}

/** Callback for reference tool available */
void IGI3DDigitizerGUI::ReferenceAvailableCallback(const itk::EventObject & event )
{
  m_ReferenceNotAvailable = false;
  m_GUI.LEDlabelDRF->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/greenLED.png")));
}

/** Callback for reference tool not available */
void IGI3DDigitizerGUI::ReferenceNotAvailableCallback(const itk::EventObject & event )
{
  m_ReferenceNotAvailable = true;
  m_GUI.LEDlabelDRF->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/redLED.png")));
}


void IGI3DDigitizerGUI::keyPressEvent ( QKeyEvent * event)
{
  switch ( event->key() )
  {
    case Qt::Key_C: CheckMarkerPosition();
        break;
    case Qt::Key_S: TogglePointAquisition(); 
        break;
    default:
        return;
  }
}

void IGI3DDigitizerGUI::ResetCamera()
{
  m_View3D->RequestResetCamera();
}

void IGI3DDigitizerGUI::StartInitialization()
{
  m_ReferenceId = m_GUI.RefIDBox->currentText(); 
  m_PointerId = m_GUI.PointerIDBox->currentText();
 
  if(m_ReferenceId.toInt() == m_PointerId.toInt())
  {
    QMessageBox::information(this, "IGI 3DDigitizer", "Same marker ID for pointer and reference choosen. Select different.");
    return;
  }
  if(m_CameraParametersFile.empty())
  {
    QMessageBox::information(this, "IGI 3DDigitizer", "Camera calibration file not defined.");
    return;
  }

  if(m_PointerToolCalibrationFile.isEmpty())
  {
    QMessageBox::information(this, "IGI 3DDigitizer", "Pointer tool calibration file not defined.");
    return;
  }
 
  if(!LoadToolCalibrationTransform(m_PointerToolCalibrationFile))
    return;

  if(!InitializeTrackerAction()) 
    return;

  // GUI status
  m_GUI.CheckPositionButton->setEnabled(true);
  m_GUI.StartButton->setEnabled(true);
  m_GUI.InitializeButton->setEnabled(false);
}

/** -----------------------------------------------------------------
*  Load points from file
*---------------------------------------------------------------------
*/
void IGI3DDigitizerGUI::LoadPoints()
{
  ClearScene();
  m_FiducialSet->LoadFiducialsFromTXTPath(m_ConfigDir, m_WorldReference);
  m_GUI.editPointNr->setText(QString::number(m_FiducialSet->GetNrOfPoints()));
  m_GUI.editVisNr->setText(QString::number(m_FiducialSet->GetNrOfVisualizedPoints()));

  // GUI status
  m_GUI.SaveAsButton->setEnabled(true);
  m_GUI.ClearButton->setEnabled(true);
}

void IGI3DDigitizerGUI::PulseTimerEvent()
{
  igstk::PulseGenerator::CheckTimeouts();
}

void IGI3DDigitizerGUI::showHelp()
{
  QDialog* helpBox = new QDialog(this);
  helpBox->setWindowTitle("Help");

  QTextBrowser* browser = new QTextBrowser(helpBox); 
  browser->setSource(QUrl::fromLocalFile(m_CurrentPath + "/README3DDigitizer.html"));
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

/**-----------------------------------------------------------------
*  Show quit dialog
*---------------------------------------------------------------------
*/
void IGI3DDigitizerGUI::OnQuitAction()
{
  QMessageBox::StandardButton value = 
    QMessageBox::information(this,
    "IGI3DDigitizer:", "Are you sure you want to quit?",
    QMessageBox::Yes | QMessageBox::No );

  if( value == QMessageBox::Yes )
  {
    this->close();
    m_GUIQuit = true;
  }
}

bool IGI3DDigitizerGUI::HasQuit() 
{
  return m_GUIQuit;
}

/** -----------------------------------------------------------------
*     Destructor
*  -----------------------------------------------------------------
*/
IGI3DDigitizerGUI::~IGI3DDigitizerGUI()
{
  delete m_GUI.Display3D;
}

void IGI3DDigitizerGUI::CheckMarkerPosition()
{
  cv::Mat currentImage = m_ArucoTracker->GetCurrentVideoFrame(); 
  cv::imshow("video",currentImage);
  cv::waitKey(2000);
  cv::destroyWindow("video");
}

/** -----------------------------------------------------------------
*  Construct an error observer for all the possible errors that occur in 
*   the observed IGSTK components.
*---------------------------------------------------------------------
*/
IGI3DDigitizerGUI::ErrorObserver::ErrorObserver() : m_ErrorOccured( false )
{ //serial communication errors
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( igstk::OpenPortErrorEvent().GetEventName(),
                                       "Error opening com port." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( igstk::ClosePortErrorEvent().GetEventName(),
                                        "Error closing com port." ) );
  //tracker errors
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( igstk::TrackerOpenErrorEvent().GetEventName(),
      "Error opening tracker communication." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( 
      igstk::TrackerInitializeErrorEvent().GetEventName(),
      "Error initializing tracker." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( 
      igstk::TrackerStartTrackingErrorEvent().GetEventName(),
      "Error starting tracking." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( 
      igstk::TrackerStopTrackingErrorEvent().GetEventName(),
      "Error stopping tracking." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( 
      igstk::TrackerCloseErrorEvent().GetEventName(),
      "Error closing tracker communication." ) );
}

/** -----------------------------------------------------------------
*   When an error occurs in an IGSTK component it will invoke this method 
*   with the appropriate error event object as a parameter.
*---------------------------------------------------------------------
*/
void IGI3DDigitizerGUI::ErrorObserver::Execute( const itk::Object * itkNotUsed(caller), 
  const itk::EventObject & event ) throw (std::exception)
{
  std::map<std::string,std::string>::iterator it;
  std::string className = event.GetEventName();
  it = this->m_ErrorEvent2ErrorMessage.find(className);

  if( it != this->m_ErrorEvent2ErrorMessage.end() )
  {
    this->m_ErrorOccured = true;
    this->m_ErrorMessage = (*it).second;
  }
  //if the event we got wasn't in the error events map then we
  //silently ignore it
}

/** -----------------------------------------------------------------
*   When an error occurs in an IGSTK component it will invoke this method 
*   with the appropriate error event object as a parameter.
*---------------------------------------------------------------------
*/
void IGI3DDigitizerGUI::ErrorObserver::Execute( itk::Object *caller, 
  const itk::EventObject & event ) throw (std::exception)
{
  const itk::Object * constCaller = caller;
  this->Execute(constCaller, event);
}

