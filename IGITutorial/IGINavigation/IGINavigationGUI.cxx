#include "IGINavigationGUI.h"

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

#define VIEW_2D_REFRESH_RATE 30
#define VIEW_3D_REFRESH_RATE 30
#define DEFAULT_ZOOM_FACTOR 1.5
#define TRACKER_FREQUENCY 20

/** ---------------------------------------------------------------
*     Constructor
* -----------------------------------------------------------------
*/
IGINavigationGUI::IGINavigationGUI()
{
  /** Initializations */
  m_GUI.setupUi(this);
  this->CreateActions();
  m_GUIQuit = false;
  m_ImageLoaded = false;
  m_GUI.progressBar->setValue( 0);
  m_TrackerRMS = 0.0;
  
  // get current application path
  QString path = QApplication::applicationDirPath();
  QDir currentDir = QDir(path);
  m_CurrentPath = currentDir.absolutePath();  
  currentDir.cdUp();
  m_TutorialDir = currentDir.absolutePath();
  m_ConfigDir = currentDir.absolutePath() + "/" + IGIConfigurationData::CONFIGURATION_FOLDER;  
  
  m_GUI.LEDlabelDRF->hide();
  m_GUI.LEDlabelTool->hide();
  m_GUI.labelDRF->hide();
  m_GUI.labelTool->hide();

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

  /** SCENE GRAPH: origin world reference */
  m_WorldReference  = igstk::AxesObject::New();
  m_WorldReference->SetSize(200.0, 200.0, 200.0);
  m_WorldReferenceRepresentation = AxesObjectRepresentationType::New();
  m_WorldReferenceRepresentation->RequestSetAxesObject(m_WorldReference);

  //create error observer
  this->m_ErrorObserver = ErrorObserver::New();

  QTimer *pulseTimer = new QTimer();
  connect(pulseTimer, SIGNAL(timeout()), this, SLOT(PulseTimerEvent()));
  pulseTimer->start(10);

  m_PointCoordsAnnotation = igstk::Annotation2D::New();

  SetupView();

  m_FiducialSet = new FiducialSet(this);
  m_FiducialSet->AddView("view3D",m_View3D.GetPointer());
  m_FiducialSet->AddView("axial",m_ViewAxial.GetPointer());
  m_FiducialSet->AddView("sagittal",m_ViewSagittal.GetPointer());
  m_FiducialSet->AddView("coronal",m_ViewCoronal.GetPointer());
  m_GUI.TabLayout->addWidget(m_FiducialSet->GetButtonWidget());
  connect(m_FiducialSet->GetButtonWidget(),SIGNAL(clicked(QString)),this,SLOT(SelectFiducial(QString)));

  m_TargetSet = new FiducialSet(this);
  m_TargetSet->SetColor(1.0,0.0,0.0);
  m_TargetSet->AddView("view3D",m_View3D.GetPointer());
  m_TargetSet->AddView("axial",m_ViewAxial.GetPointer());
  m_TargetSet->AddView("sagittal",m_ViewSagittal.GetPointer());
  m_TargetSet->AddView("coronal",m_ViewCoronal.GetPointer());
  m_GUI.TargetTabLayout->addWidget(m_TargetSet->GetButtonWidget());
  connect(m_TargetSet->GetButtonWidget(),SIGNAL(clicked(QString)),this,SLOT(SelectTarget(QString)));

  m_GUI.AxialSlider->setEnabled(false);
  m_GUI.CoronalSlider->setEnabled(false);
  m_GUI.SagittalSlider->setEnabled(false);
  m_GUI.CheckPositionButton->setEnabled(false);
}

/** -----------------------------------------------------------------
*     Connect GUI elements with actions
*  -----------------------------------------------------------------
*/
void IGINavigationGUI::CreateActions()
{
  connect(m_GUI.QuitPushButton, SIGNAL(clicked()), this, SLOT(OnQuitAction()));
  connect(m_GUI.LoadConfigurationButton, SIGNAL(clicked()), this, SLOT(LoadConfiguration()));
  connect(m_GUI.AcceptFiducialPushButton, SIGNAL(clicked()), this, SLOT(AcceptTrackerFiducialAction()));
  connect(m_GUI.StartRegistrationPushButton,SIGNAL(clicked()), this, SLOT(CalculateRegistrationAction()));
  connect(m_GUI.helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  connect(m_GUI.AxialSlider, SIGNAL( valueChanged(int)), this, SLOT(ResliceImageCallback1(int)));
  connect(m_GUI.SagittalSlider, SIGNAL( valueChanged(int)), this, SLOT(ResliceImageCallback2(int)));
  connect(m_GUI.CoronalSlider, SIGNAL( valueChanged(int)), this, SLOT(ResliceImageCallback3(int)));

  connect(m_GUI.CheckPositionButton, SIGNAL(clicked()), this, SLOT(CheckMarkerPosition()));
}

void IGINavigationGUI::SetupView()
{
  // Create views
  m_ViewAxial = View2DType::New();
  m_ViewSagittal = View2DType::New();
  m_ViewCoronal = View2DType::New();
  m_View3D = View3DType::New();

  m_ViewAxial->RequestSetOrientation(View2DType::Axial);
  m_ViewSagittal->RequestSetOrientation(View2DType::Sagittal);
  m_ViewCoronal->RequestSetOrientation(View2DType::Coronal);

  // Set IGSTK view for Qt display
  m_GUI.Display3D->RequestSetView (m_View3D);
  m_GUI.DisplayAxial->RequestSetView (m_ViewAxial);
  m_GUI.DisplayCoronal->RequestSetView (m_ViewCoronal);
  m_GUI.DisplaySagittal->RequestSetView (m_ViewSagittal);

  // Enable Qt display interaction
  m_GUI.Display3D->RequestEnableInteractions();
  m_GUI.DisplayAxial->RequestEnableInteractions();
  m_GUI.DisplayCoronal->RequestEnableInteractions();
  m_GUI.DisplaySagittal->RequestEnableInteractions();

  // set background color to the views
  m_ViewAxial->SetRendererBackgroundColor(1,1,1);
  m_ViewSagittal->SetRendererBackgroundColor(1,1,1);
  m_ViewCoronal->SetRendererBackgroundColor(1,1,1);
  m_View3D->SetRendererBackgroundColor(1,1,1);

  ResetCamera();
}

bool IGINavigationGUI::LoadImageAction()
{
  handleMessage("IGINavigationGUI::LoadImageProcessing called...\n", 0);

  //check for slash
  if (itksys::SystemTools::StringEndsWith(m_ImageDir.c_str(),"/") )
  {
    m_ImageDir = m_ImageDir.substr (0,m_ImageDir.length()-1);
  }
  handleMessage( std::string("Set image directory: " + m_ImageDir + "\n"), 0 );

  /** Setup image reader  */
  CTImageReaderType::Pointer CTImageReader        = CTImageReaderType::New();
  
  /** Build itk progress command to assess image load progress */
  itk::SmartPointer<ProgressCommandType>            progressCommand;
  progressCommand = ProgressCommandType::New();

  /** Set the callback to the itk progress command */
  progressCommand->SetCallbackFunction( this, &IGINavigationGUI::OnITKProgressEvent );

  // Provide a progress observer to the image reader
  CTImageReader->RequestSetProgressCallback( progressCommand );

  //Add observer for invalid directory
  DICOMImageReaderInvalidDirectoryNameErrorObserver::Pointer didcb =
  DICOMImageReaderInvalidDirectoryNameErrorObserver::New();

  CTImageReader->AddObserver(
  igstk::DICOMImageDirectoryIsNotDirectoryErrorEvent(), didcb );

  //Add observer for a non-existing directory
  DICOMImageReaderNonExistingDirectoryErrorObserver::Pointer dndcb =
  DICOMImageReaderNonExistingDirectoryErrorObserver::New();

  CTImageReader->AddObserver(
  igstk::DICOMImageDirectoryDoesNotExistErrorEvent(), dndcb );

  //Add observer for a an empty directory name (null string)
  DICOMImageReaderEmptyDirectoryErrorObserver::Pointer decb =
  DICOMImageReaderEmptyDirectoryErrorObserver::New();
  CTImageReader->AddObserver( igstk::DICOMImageDirectoryEmptyErrorEvent(), decb );

  //Add observer for a directory which does not have enough number of files
  DICOMImageDirectoryNameDoesNotHaveEnoughFilesErrorObserver::Pointer ddhefcb =
  DICOMImageDirectoryNameDoesNotHaveEnoughFilesErrorObserver::New();

  CTImageReader->AddObserver(
  igstk::DICOMImageDirectoryDoesNotHaveEnoughFilesErrorEvent(), ddhefcb );

  //Add observer for a directory containing non-DICOM files
  DICOMImageDirectoryDoesNotContainValidDICOMSeriesErrorObserver::Pointer disgcb =
  DICOMImageDirectoryDoesNotContainValidDICOMSeriesErrorObserver::New();

  CTImageReader->AddObserver(
  igstk::DICOMImageSeriesFileNamesGeneratingErrorEvent(), disgcb );

  //Add observer for reading invalid/corrupted dicom files
  DICOMImageInvalidErrorObserver::Pointer dircb =
  DICOMImageInvalidErrorObserver::New();

  CTImageReader->AddObserver( igstk::DICOMImageReadingErrorEvent(), dircb );

  // Set directory
  CTImageReader->RequestSetDirectory( m_ImageDir );

  if( didcb->GotDICOMImageReaderInvalidDirectoryNameError() )
  {
    handleMessage("Invalid DICOM directory.\n", 1 );
    return false;
  }

  if( dndcb->GotDICOMImageReaderNonExistingDirectoryError() )
  {
    handleMessage("Image directory does not exist.\n", 1 );
    return false;
  }

  if( decb->GotDICOMImageReaderEmptyDirectoryError() )
  {
    handleMessage("Image directory is empty.\n", 1 );
    return false;
  }

  if( ddhefcb->GotDICOMImageDirectoryNameDoesNotHaveEnoughFilesError() )
  {
    handleMessage("Image directory does not have enough files.\n", 1 );
    return false;
  }

  if( disgcb->GotDICOMImageDirectoryDoesNotContainValidDICOMSeriesError() )
  {
    handleMessage("Image directory does not contain valid DICOM series.\n", 1 );
    return false;
  }

  // Read Image
  CTImageReader->RequestReadImage();

  CTImageObserver::Pointer CTImageObserver = CTImageObserver::New();
  CTImageReader->AddObserver(CTImageReaderType::ImageModifiedEvent(),
  CTImageObserver);

  CTImageReader->RequestGetImage();

  if(!CTImageObserver->GotCTImage())
  {
    handleMessage(std::string("Could not open CT dataset, check modality."), 1 );
    CTImageObserver->RemoveAllObservers();
    CTImageObserver = NULL;
    return false;
  }
  if ( CTImageReader->FileSuccessfullyRead() )
  {
    if ( CTImageObserver.IsNotNull() )
    {
      m_CTImageSpatialObject = CTImageObserver->GetCTImage();
      this->ConnectImageRepresentation();
        m_GUI.AxialSlider->setEnabled(true);
        m_GUI.CoronalSlider->setEnabled(true);
        m_GUI.SagittalSlider->setEnabled(true);
      this->ResetCamera();
    }
  }
  return true;
}

/** -----------------------------------------------------------------
* Set ImageSpatialObjects to
* ImageRepresentations, sets image orientations, adds ImageSpatialObjects
* to Views and connects the scene graph
*---------------------------------------------------------------------
*/
void IGINavigationGUI::ConnectImageRepresentation()
{
  handleMessage("IGINavigationGUI::ConnectImageRepresentation called...\n", 0 );

  //once a dataset is loaded remove all objects from all views
  if(m_ImageLoaded)
  {
    m_ViewAxial->RequestRemoveObject( m_AxialPlaneRepresentation );
    m_ViewSagittal->RequestRemoveObject( m_SagittalPlaneRepresentation );
    m_ViewCoronal->RequestRemoveObject( m_CoronalPlaneRepresentation );

    m_View3D->RequestRemoveObject( m_AxialPlaneRepresentation2 );
    m_View3D->RequestRemoveObject( m_SagittalPlaneRepresentation2 );
    m_View3D->RequestRemoveObject( m_CoronalPlaneRepresentation2 );

    m_ViewAxial->RequestRemoveObject( m_AxialCrossHairRepresentation );
    m_ViewSagittal->RequestRemoveObject( m_SagittalCrossHairRepresentation );
    m_ViewCoronal->RequestRemoveObject( m_CoronalCrossHairRepresentation );
    m_View3D->RequestRemoveObject( m_3DViewCrossHairRepresentation );

    m_View3D->RequestStop();
    m_ViewAxial->RequestStop();
    m_ViewSagittal->RequestStop();
    m_ViewCoronal->RequestStop();
  }

  /** SCENE GRAPH: Image and multimodal views */
  igstk::Transform identity;
  identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

  // set transform and parent to the image spatial object
  m_CTImageSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
  
  m_ViewAxial->RequestSetTransformAndParent( identity, m_WorldReference );
  m_ViewSagittal->RequestSetTransformAndParent( identity, m_WorldReference );
  m_ViewCoronal->RequestSetTransformAndParent( identity, m_WorldReference );
  m_View3D->RequestSetTransformAndParent( identity, m_WorldReference );
  /***/

  // set up view parameters
  m_ViewAxial->SetRefreshRate( VIEW_2D_REFRESH_RATE );
  m_ViewSagittal->SetRefreshRate( VIEW_2D_REFRESH_RATE );
  m_ViewCoronal->SetRefreshRate( VIEW_2D_REFRESH_RATE );
  m_View3D->SetRefreshRate( VIEW_3D_REFRESH_RATE );

  m_ViewAxial->SetCameraZoomFactor(DEFAULT_ZOOM_FACTOR);
  m_ViewSagittal->SetCameraZoomFactor(DEFAULT_ZOOM_FACTOR);
  m_ViewCoronal->SetCameraZoomFactor(DEFAULT_ZOOM_FACTOR);
  m_View3D->SetCameraZoomFactor(DEFAULT_ZOOM_FACTOR);

  /** ReslicePlanes */
  // create reslice plane spatial object for axial view
  m_AxialPlaneSpatialObject = ReslicerPlaneType::New();
  m_AxialPlaneSpatialObject->RequestSetReslicingMode( ReslicerPlaneType::Orthogonal );
  m_AxialPlaneSpatialObject->RequestSetOrientationType( ReslicerPlaneType::Axial );
  m_AxialPlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject( m_CTImageSpatialObject );

  // create reslice plane spatial object for sagittal view
  m_SagittalPlaneSpatialObject = ReslicerPlaneType::New();
  m_SagittalPlaneSpatialObject->RequestSetReslicingMode( ReslicerPlaneType::Orthogonal );
  m_SagittalPlaneSpatialObject->RequestSetOrientationType( ReslicerPlaneType::Sagittal );
  m_SagittalPlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject( m_CTImageSpatialObject );

  // create reslice plane spatial object for coronal view
  m_CoronalPlaneSpatialObject = ReslicerPlaneType::New();
  m_CoronalPlaneSpatialObject->RequestSetReslicingMode( ReslicerPlaneType::Orthogonal );
  m_CoronalPlaneSpatialObject->RequestSetOrientationType( ReslicerPlaneType::Coronal );
  m_CoronalPlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject( m_CTImageSpatialObject );
  
  // create reslice plane representation for axial view
  m_AxialPlaneRepresentation = CTImageRepresentationType::New();
  m_AxialPlaneRepresentation->RequestSetImageSpatialObject( m_CTImageSpatialObject );
  m_AxialPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_AxialPlaneSpatialObject );

  // create reslice plane representation for sagittal view
  m_SagittalPlaneRepresentation = CTImageRepresentationType::New();
  m_SagittalPlaneRepresentation->RequestSetImageSpatialObject( m_CTImageSpatialObject );
  m_SagittalPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_SagittalPlaneSpatialObject );

  // create reslice plane representation for coronal view
  m_CoronalPlaneRepresentation = CTImageRepresentationType::New();
  m_CoronalPlaneRepresentation->RequestSetImageSpatialObject( m_CTImageSpatialObject );
  m_CoronalPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_CoronalPlaneSpatialObject );

  /** SCENE GRAPH: World, image, reslicer planes and multimodal views */
  m_AxialPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
  m_SagittalPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
  m_CoronalPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
  m_MeshMap[m_PointerId]->RequestSetTransformAndParent(identity, m_WorldReference);
  
  m_View3D->RequestAddObject(m_MeshRepresentationMap[m_PointerId]);
  
  QStringList markerKeys = m_MarkerIdMeshMap.keys();
  for(int i = 0; i<markerKeys.size(); i++)
  {
    QString markerId =markerKeys[i];
    LoadToolSpatialObjectAction(markerId, m_MarkerIdMeshMap[markerId].toStdString());
    m_MeshMap[markerId]->RequestSetTransformAndParent(identity, m_WorldReference);
    m_View3D->RequestAddObject(m_MeshRepresentationMap[markerId]);
  }

  m_ViewAxial->RequestDetachFromParent();
  m_ViewSagittal->RequestDetachFromParent();
  m_ViewCoronal->RequestDetachFromParent();

  m_ViewAxial->RequestSetTransformAndParent( identity, m_AxialPlaneSpatialObject );
  m_ViewSagittal->RequestSetTransformAndParent( identity, m_SagittalPlaneSpatialObject );
  m_ViewCoronal->RequestSetTransformAndParent( identity, m_CoronalPlaneSpatialObject );
  /***/

  // set parallel projection in the 2D views
  m_ViewAxial->SetCameraParallelProjection(true);
  m_ViewSagittal->SetCameraParallelProjection(true);
  m_ViewCoronal->SetCameraParallelProjection(true);

  // add reslice plane representations to the orthogonal views
  m_ViewAxial->RequestAddObject( m_AxialPlaneRepresentation );
  m_ViewSagittal->RequestAddObject( m_SagittalPlaneRepresentation );
  m_ViewCoronal->RequestAddObject( m_CoronalPlaneRepresentation );

  // add reslice plane representations to the 3D views
  m_AxialPlaneRepresentation2 = m_AxialPlaneRepresentation->Copy(); 
  m_SagittalPlaneRepresentation2 = m_SagittalPlaneRepresentation->Copy();  
  m_CoronalPlaneRepresentation2 = m_CoronalPlaneRepresentation->Copy();

  m_View3D->RequestAddObject( m_AxialPlaneRepresentation2 );
  m_View3D->RequestAddObject( m_SagittalPlaneRepresentation2 );
  m_View3D->RequestAddObject( m_CoronalPlaneRepresentation2 );

  m_PointCoordsAnnotation->RequestSetFontColor( 0.0, 1.0, 0.0, 0.0 ); 

  m_View3D->RequestAddAnnotation2D(m_PointCoordsAnnotation);

  ///** Set up cross hairs */
  m_CrossHair = CrossHairType::New();
  m_CrossHair->RequestSetBoundingBoxProviderSpatialObject( m_CTImageSpatialObject );

  // buid the cross hair representations
  m_AxialCrossHairRepresentation = CrossHairRepresentationType::New();
  m_AxialCrossHairRepresentation->SetOpacity(1);
  m_AxialCrossHairRepresentation->SetLineWidth(2);
  m_AxialCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );

  m_SagittalCrossHairRepresentation = CrossHairRepresentationType::New();
  m_SagittalCrossHairRepresentation->SetOpacity(1);
  m_SagittalCrossHairRepresentation->SetLineWidth(2);
  m_SagittalCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );

  m_CoronalCrossHairRepresentation = CrossHairRepresentationType::New();
  m_CoronalCrossHairRepresentation->SetOpacity(1);
  m_CoronalCrossHairRepresentation->SetLineWidth(2);
  m_CoronalCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );
  //delete 3D cross har
  m_3DViewCrossHairRepresentation = CrossHairRepresentationType::New();
  m_3DViewCrossHairRepresentation->SetOpacity(1);
  m_3DViewCrossHairRepresentation->SetLineWidth(2);
  m_3DViewCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );

  // add the cross hair representation to the different views
  m_ViewAxial->RequestAddObject( m_AxialCrossHairRepresentation );
  m_ViewSagittal->RequestAddObject( m_SagittalCrossHairRepresentation );
  m_ViewCoronal->RequestAddObject( m_CoronalCrossHairRepresentation );

  /**
  *  Request information about the slice bounds. The answer will be
  *  received in the form of an event. This will be used to initialize
  *  the reslicing sliders and set initial slice position
  */
  ImageExtentObserver::Pointer extentObserver = ImageExtentObserver::New();
  unsigned int extentObserverID;
  extentObserverID = m_CTImageSpatialObject->AddObserver( igstk::ImageExtentEvent(), extentObserver );
  m_CTImageSpatialObject->RequestGetImageExtent();
  if( extentObserver->GotImageExtent() )
  {
    const igstk::EventHelperType::ImageExtentType& extent = extentObserver->GetImageExtent();

    const unsigned int zmin = extent.zmin;
    const unsigned int zmax = extent.zmax;
    const unsigned int zslice = static_cast< unsigned int > ( (zmin + zmax) / 2.0 );
    m_GUI.AxialSlider->setEnabled(true);
    m_GUI.AxialSlider->setMinimum( zmin );
    m_GUI.AxialSlider->setMaximum( zmax );
    m_GUI.AxialSlider->setValue( zslice );      

    const unsigned int ymin = extent.ymin;
    const unsigned int ymax = extent.ymax;
    const unsigned int yslice = static_cast< unsigned int > ( (ymin + ymax) / 2.0 );
    m_GUI.SagittalSlider->setEnabled(true);
    m_GUI.SagittalSlider->setMinimum( ymin );
    m_GUI.SagittalSlider->setMaximum( ymax );
    m_GUI.SagittalSlider->setValue( yslice );

    const unsigned int xmin = extent.xmin;
    const unsigned int xmax = extent.xmax;
    const unsigned int xslice = static_cast< unsigned int > ( (xmin + xmax) / 2.0 );
    m_GUI.CoronalSlider->setEnabled(true);
    m_GUI.CoronalSlider->setMinimum( xmin );
    m_GUI.CoronalSlider->setMaximum( xmax );
    m_GUI.CoronalSlider->setValue( xslice );

    // Show initial annotation
    IndexType index;
    index[2]= zslice;
    index[0]= yslice;
    index[1]= xslice;

    PointType point;
    m_CTImageSpatialObject->TransformIndexToPhysicalPoint( index, point );
    QString coord = "[ " + QString::number(point[0]) + ", " 
      + QString::number(point[1]) + ", " 
      + QString::number(point[2]) + "]";    
    m_PointCoordsAnnotation->RequestSetAnnotationText(0,coord.toStdString());
  }

  m_CTImageSpatialObject->RemoveObserver( extentObserverID );
    
  // set transform and parent to the cross hair object
  m_CrossHair->RequestSetTransformAndParent( identity, m_WorldReference );
  
  /** Add observer for picking events in the Axial view */
  m_AxialViewPickerObserver = LoadedObserverType::New();
  m_AxialViewPickerObserver->SetCallbackFunction( this, &IGINavigationGUI::AxialViewPickingCallback );

  m_ViewAxial->AddObserver(
  igstk::CoordinateSystemTransformToEvent(), m_AxialViewPickerObserver );
  
  /** Add observer for picking events in the Axial view */
  m_SagittalViewPickerObserver = LoadedObserverType::New();
  m_SagittalViewPickerObserver->SetCallbackFunction( this, &IGINavigationGUI::SagittalViewPickingCallback );

  m_ViewSagittal->AddObserver(
  igstk::CoordinateSystemTransformToEvent(), m_SagittalViewPickerObserver );

  /** Add observer for picking events in the Axial view */
  m_CoronalViewPickerObserver = LoadedObserverType::New();
  m_CoronalViewPickerObserver->SetCallbackFunction( this, &IGINavigationGUI::CoronalViewPickingCallback );

  m_ViewCoronal->AddObserver(
  igstk::CoordinateSystemTransformToEvent(), m_CoronalViewPickerObserver );

  m_View3D->SetCameraPosition( 550.0, 600.0, 300.0 );
  m_View3D->SetCameraFocalPoint( 0.0, 0.0, 0.0);
  // reset the cameras in the different views
  ResetCamera();

  m_View3D->RequestStart();
  m_ViewAxial->RequestStart();
  m_ViewSagittal->RequestStart();
  m_ViewCoronal->RequestStart();

  ResetCamera();
  ResetCamera();
  m_ImageLoaded = true;

  m_AxialPlaneRepresentation->SetWindowLevel(m_Window,m_Level);
  m_SagittalPlaneRepresentation->SetWindowLevel(m_Window,m_Level);
  m_CoronalPlaneRepresentation->SetWindowLevel(m_Window,m_Level);
  m_AxialPlaneRepresentation2->SetWindowLevel(m_Window,m_Level);
  m_SagittalPlaneRepresentation2->SetWindowLevel(m_Window,m_Level);
  m_CoronalPlaneRepresentation2->SetWindowLevel(m_Window,m_Level);
}

bool IGINavigationGUI::InitializeTrackerAction()
{
  handleMessage("IGINavigationGUI::InitializeTrackerAction called...\n", 0 );

  igstk::Transform identity;
  identity.SetToIdentity(igstk::TimeStamp::GetLongestPossibleTime());

  //create tracker
  m_ArucoTracker = igstk::ArucoTracker::New(); 
  m_ArucoTracker->RequestSetFrequency( TRACKER_FREQUENCY );

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
  else   //attach the tools 
  {
    m_ArucoTrackerTool = igstk::ArucoTrackerTool::New();
    m_ArucoTrackerTool->RequestSetMarkerName(m_PointerId.toInt());
    m_ArucoTrackerTool->SetCalibrationTransform(m_ToolCalibrationTransform);
    m_ArucoTrackerTool->RequestConfigure();

    m_ArucoReferenceTrackerTool = igstk::ArucoTrackerTool::New();
    m_ArucoReferenceTrackerTool->RequestSetMarkerName(m_ReferenceId.toInt());
    m_ArucoReferenceTrackerTool->RequestConfigure();

    /** SCENE GRAPH: world, tracker, tracker tool, reference tool */
    m_ArucoTrackerTool->RequestAttachToTracker( m_ArucoTracker );
    m_ArucoReferenceTrackerTool->RequestAttachToTracker( m_ArucoTracker );

    m_ArucoTracker->RequestSetReferenceTool( m_ArucoReferenceTrackerTool );

    m_ArucoReferenceTrackerTool->RequestSetTransformAndParent(identity, m_WorldReference);

    if ( m_MeshMap[m_PointerId].IsNull() )
    {
      handleMessage("Tool spatial object not available\n", 1 );
      return false;
    }
    m_MeshMap[m_PointerId]->RequestDetachFromParent();
    m_MeshMap[m_PointerId]->RequestSetTransformAndParent( identity, m_ArucoTrackerTool );
    /***/ 
  }

  // setup all other tools
  QStringList markerKeys = m_MarkerIdMeshMap.keys();
  for(int i = 0; i<markerKeys.size(); i++)
  {
    QString markerId = markerKeys[i];
    QString XMLmarkerId = markerId;
    markerId.replace("id",""); 
    m_ArucoTrackerToolMap[markerId] = igstk::ArucoTrackerTool::New();
    m_ArucoTrackerToolMap[markerId]->RequestSetMarkerName(markerId.toInt());
    m_ArucoTrackerToolMap[markerId]->RequestConfigure();
    m_ArucoTrackerToolMap[markerId]->RequestAttachToTracker( m_ArucoTracker );

    m_MeshMap[XMLmarkerId]->RequestDetachFromParent();
    m_MeshMap[XMLmarkerId]->RequestSetTransformAndParent( identity, m_ArucoTrackerToolMap[markerId] );
  }
 
  // TrackerToolAvailableObserver
  m_TrackerToolAvailableObserver = LoadedObserverType::New();
  m_TrackerToolAvailableObserver->SetCallbackFunction( this,
                        &IGINavigationGUI::ToolAvailableCallback );
  m_ArucoTrackerTool->AddObserver(
      igstk::TrackerToolMadeTransitionToTrackedStateEvent(),
                    m_TrackerToolAvailableObserver);

  // TrackerToolNotAvailableObserver
  m_TrackerToolNotAvailableObserver = LoadedObserverType::New();
  m_TrackerToolNotAvailableObserver->SetCallbackFunction(
            this, &IGINavigationGUI::ToolNotAvailableCallback );
      
  m_ArucoTrackerTool->AddObserver(
            igstk::TrackerToolNotAvailableToBeTrackedEvent(),
              m_TrackerToolNotAvailableObserver);

  m_ReferenceNotAvailableObserver = LoadedObserverType::New();
  m_ReferenceNotAvailableObserver->SetCallbackFunction(this,
    &IGINavigationGUI::ReferenceNotAvailableCallback );
  m_ArucoReferenceTrackerTool->AddObserver(
            igstk::TrackerToolNotAvailableToBeTrackedEvent(),
                m_ReferenceNotAvailableObserver);

  m_ReferenceAvailableObserver = LoadedObserverType::New();
  m_ReferenceAvailableObserver->SetCallbackFunction( this,
    &IGINavigationGUI::ReferenceAvailableCallback );

  m_ArucoReferenceTrackerTool->AddObserver(
          igstk::TrackerToolMadeTransitionToTrackedStateEvent(),
                m_ReferenceAvailableObserver);

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
 
  // Select first fiducial after tracking is started
  SelectFiducial(m_FiducialSet->GetButtonWidget()->CheckButton(""));

  m_GUI.LEDlabelDRF->show();
  m_GUI.LEDlabelTool->show();
  m_GUI.labelDRF->show();
  m_GUI.labelTool->show();

  return true;
}

bool IGINavigationGUI::LoadToolCalibrationTransform(QString transformFile)
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

/** -----------------------------------------------------------------
* Load tool spatial object mesh. This method asks for a file with the
* spatial object mesh in the .msh format (see mesh SpatialObject in ITK)
*  -----------------------------------------------------------------
*/
bool IGINavigationGUI::LoadToolSpatialObjectAction(QString id, std::string meshFile)
{
  handleMessage(
     "IGINavigationGUI::LoadToolSpatialObjectAction called...\n", 0 );
  // read mesh
  MeshReaderType::Pointer reader = MeshReaderType::New();
  reader->RequestSetFileName( meshFile );
  reader->RequestReadObject();

  MeshObjectObserver::Pointer observer = MeshObjectObserver::New();

  reader->AddObserver( igstk::MeshReader::MeshModifiedEvent(), observer);
  reader->RequestGetOutput();

  if( !observer->GotMeshObject() )
  {
    handleMessage("Cannot read 3D model.\n", 1 );
    return false;
  }

  // get meshobject from the reader
  m_MeshMap[id] = observer->GetMeshObject();
  
  if ( m_MeshMap[id].IsNotNull() )
  {
    m_MeshRepresentationMap[id] = MeshRepresentationType::New();
    m_MeshRepresentationMap[id]->RequestSetMeshObject( m_MeshMap[id] );
    m_MeshRepresentationMap[id]->SetOpacity(1.0);
    m_MeshRepresentationMap[id]->SetColor(0,0,1);
  }
  else
  {
    handleMessage("Tool visualization could not added to the views.", 1 ); 
    return false;
  }
  return true;
}


void IGINavigationGUI::AcceptTrackerFiducialAction()
{
  handleMessage("igstk::IGINavigationGUI::"
    "AcceptTrackerFiducialAction called...\n", 0 );
    
  // get transform
  typedef igstk::TransformObserver ObserverType;
  ObserverType::Pointer transformObserver = ObserverType::New();
  transformObserver->ObserveTransformEventsFrom( m_ArucoTrackerTool );
  transformObserver->Clear();
  m_ArucoTrackerTool->RequestComputeTransformTo( m_ArucoReferenceTrackerTool );
   
  if( !transformObserver->GotTransform() )
  {  
    handleMessage("Failed to acquire fiducial position (make sure DRF and tool are visible).\n", 1 );
    return;
  }

  igstk::TrackerTool::TransformType transform;
  transform = transformObserver->GetTransform();
  if(transform.IsValidNow())
  {
    // store transform in the landmark container
    QString selectedId = m_FiducialSet->GetSelectedId();
    m_LandmarksContainer[selectedId] = TransformToPoint( transform );

    // jump to the next fiducial
    SelectFiducial(m_FiducialSet->GetButtonWidget()->CheckNextButton(selectedId));
  }
  else
  {
    handleMessage("Failed to acquire fiducial position (make sure DRF and tool are visible).\n", 1 );
  }
}

void IGINavigationGUI::SceneObjectsVisibility(double opacity)
{
  m_MeshRepresentationMap[m_PointerId]->SetOpacity(opacity);

  m_FiducialSet->SetOpacity(opacity);
  m_TargetSet->SetOpacity(opacity);
}

/** Callback for tool available */
void IGINavigationGUI::ToolAvailableCallback(const itk::EventObject & event )
{
  m_ToolNotAvailable = false;
  m_GUI.LEDlabelTool->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/greenLED.png")));

  if (!m_ReferenceNotAvailable)
  {
    SceneObjectsVisibility(0.6);
  }
}

/** Callback for tool not available */
void IGINavigationGUI::ToolNotAvailableCallback(const itk::EventObject & event )
{
  m_ToolNotAvailable = true;

  m_GUI.LEDlabelTool->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/redLED.png")));

  SceneObjectsVisibility(0.0);
}

/** Callback for reference tool available */
void IGINavigationGUI::ReferenceAvailableCallback(const itk::EventObject & event )
{
  m_ReferenceNotAvailable = false;

  m_GUI.LEDlabelDRF->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/greenLED.png")));

  if (!m_ToolNotAvailable)
  {
    SceneObjectsVisibility(0.6);
  }
}

/** Callback for reference tool not available */
void IGINavigationGUI::ReferenceNotAvailableCallback(const itk::EventObject & event )
{
  m_ReferenceNotAvailable = true;

  m_GUI.LEDlabelDRF->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/redLED.png")));

  SceneObjectsVisibility(0.0);
}

/** -----------------------------------------------------------------
* Registers the tracker to the working image
*---------------------------------------------------------------------
*/
void IGINavigationGUI::CalculateRegistrationAction()
{
  handleMessage(
        "IGINavigationGUI::TrackerRegistrationProcessing called...\n", 0 );

  RegistrationType::Pointer registration  = RegistrationType::New();
  registration->RequestResetRegistration();

  QStringList keys = m_LandmarksContainer.keys();
  for(int i = 0; i<keys.size(); i++)
  {
    QString key = keys[i];
    registration->RequestAddImageLandmarkPoint(
      m_FiducialSet->GetPositionOfFiducial(key));
  
    registration->RequestAddTrackerLandmarkPoint(
      m_LandmarksContainer[key]);
  }   

  registration->RequestComputeTransform();

  igstk::TransformObserver::Pointer lrtcb = igstk::TransformObserver::New();
  lrtcb->ObserveTransformEventsFrom( registration );
  lrtcb->Clear();

  registration->RequestGetTransformFromTrackerToImage();

  if( lrtcb->GotTransform() )
  {
    RegistrationErrorObserver::Pointer lRmscb =
    RegistrationErrorObserver::New();

    registration->AddObserver( igstk::DoubleTypeEvent(), lRmscb );
    registration->RequestGetRMSError();
    if( lRmscb->GotRegistrationError() )
    {
      m_TrackerRMS = lRmscb->GetRegistrationError();
      QString rmsErrorInfo = QString("Fiducial Registration Error: ") + 
                              QString::number(m_TrackerRMS,'f',2)
                              + " mm";
      QMessageBox::information(this,windowTitle(),rmsErrorInfo);
    }
    else
    {
      handleMessage("Could not retrieve RMS error.\n", 1 );
      return;
    }

    // set new transformation
    m_RegistrationTransform = lrtcb->GetTransform();
    StoreTransformInXMLFormat(m_RegistrationTransform);

    if ( m_ArucoReferenceTrackerTool.IsNotNull() )
    {
      m_ArucoReferenceTrackerTool->RequestDetachFromParent();
      m_ArucoReferenceTrackerTool->RequestSetTransformAndParent(m_RegistrationTransform, m_WorldReference);
    }
    AcceptingRegistration();
  }
  else
  {
    handleMessage("Registration failed (possibly collinear fiducial configuration).\n", 1 );
  }
return;
}

void IGINavigationGUI::keyPressEvent ( QKeyEvent * event)
{
  switch ( event->key() )
  {
    case Qt::Key_A: 
      AcceptTrackerFiducialAction();
      break;
    case Qt::Key_R: 
      CalculateRegistrationAction();
      break;
    default:
      return;
  }
}

/** Method to be invoked on successful registration acceptance */
void IGINavigationGUI::AcceptingRegistration()
{
  handleMessage("igstk::IGINavigationGUI::"
  "ReportSuccessAcceptingRegistration called...\n", 0 );
  
  // add the tool object to the image planes
  m_AxialPlaneSpatialObject->RequestSetToolSpatialObject( m_MeshMap[m_PointerId] );
  m_SagittalPlaneSpatialObject->RequestSetToolSpatialObject( m_MeshMap[m_PointerId] );
  m_CoronalPlaneSpatialObject->RequestSetToolSpatialObject( m_MeshMap[m_PointerId] );
  m_CrossHair->RequestSetToolSpatialObject( m_MeshMap[m_PointerId] );

  m_GUI.AxialSlider->setEnabled(false);
  m_GUI.SagittalSlider->setEnabled(false);
  m_GUI.CoronalSlider->setEnabled(false);
  m_PointCoordsAnnotation->RequestSetFontColor( 1.0, 1.0, 1.0, 0.0 ); 
  // reset the cameras in the different views
  ResetCamera();
}

// Reslices the views to the picked position
void IGINavigationGUI::SetImagePickingProcessing()
{
  handleMessage(
      "IGINavigationGUI::SetImagePickingProcessing called...\n", 0 );

  PointType point = TransformToPoint( m_PickingTransform );

  if ( m_CTImageSpatialObject->IsInside( point ) )
  {
    IndexType index;
    m_CTImageSpatialObject->TransformPhysicalPointToIndex( point, index);

    const double *data = point.GetVnlVector().data_block();

    m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
    m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
    m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
    m_CrossHair->RequestSetCursorPosition( data );
    this->ResliceImage( index );

    QString str1;
    QString str2;
    QString str3;
    str1.append(QString("%1").arg(data[0]));
    str2.append(QString("%1").arg(data[1]));
    str3.append(QString("%1").arg(data[2]));

    QString coord = "[ " + str1 + ", " + str2 + ", " + str3 + "]";    
    m_PointCoordsAnnotation->RequestSetAnnotationText(0,coord.toStdString());
  }
  else
  {
    handleMessage("Picked point outside image...\n", 0 );
  }
}

/** -----------------------------------------------------------------
*  Callback function for picking event in the axial view.
*  Upon receiving a valid picking event, this method will reslice the
*  image to that location.
*---------------------------------------------------------------------
*/
void IGINavigationGUI::AxialViewPickingCallback( const itk::EventObject & event)
{
  if ( igstk::CoordinateSystemTransformToEvent().CheckEvent( &event ) )
  {
    typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
    const TransformEventType * tmevent =
    dynamic_cast< const TransformEventType *>( & event );

    // get the transform from the view to its parent (reslicer plane)
    igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
    m_PickingTransform = transformCarrier.GetTransform();

    // get the transform from the reslicer plane to its parent (world reference)
    CoordinateSystemTransformObserver::Pointer coordinateObserver =
    CoordinateSystemTransformObserver::New();

    unsigned int obsId = m_AxialPlaneSpatialObject->AddObserver(
    igstk::CoordinateSystemTransformToEvent(), coordinateObserver );

    m_AxialPlaneSpatialObject->RequestComputeTransformTo( m_WorldReference );

    if( coordinateObserver->GotCoordinateSystemTransform() )
    {
      igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
      igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
      m_PickingTransform = igstk::Transform::TransformCompose( viewToWorldReferenceTransform, m_PickingTransform );
    }
    else
    {
      handleMessage(
      "IGINavigationGUI::AxialViewPickingCallback could not get coordinate system transform...\n", 0 );
      return;
    }

    m_AxialPlaneSpatialObject->RemoveObserver( obsId );
    SetImagePickingProcessing();
  }
}

/** -----------------------------------------------------------------
*  Callback function for picking event in the sagittal view.
*  Upon receiving a valid picking event, this method will reslice the
*  image to that location.
*---------------------------------------------------------------------
*/
void IGINavigationGUI::SagittalViewPickingCallback( const itk::EventObject & event)
{
  if ( igstk::CoordinateSystemTransformToEvent().CheckEvent( &event ) )
  {
  typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
  const TransformEventType * tmevent =
    dynamic_cast< const TransformEventType *>( & event );

  // get the transform from the view to its parent (reslicer plane)
  igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
  m_PickingTransform = transformCarrier.GetTransform();

  // get the transform from the reslicer plane to its parent (world reference)
  CoordinateSystemTransformObserver::Pointer coordinateObserver =
    CoordinateSystemTransformObserver::New();

  unsigned int obsId = m_SagittalPlaneSpatialObject->AddObserver(
    igstk::CoordinateSystemTransformToEvent(), coordinateObserver );

  m_SagittalPlaneSpatialObject->RequestComputeTransformTo( m_WorldReference );

  if( coordinateObserver->GotCoordinateSystemTransform() )
  {
    igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
    igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
    m_PickingTransform = igstk::Transform::TransformCompose( viewToWorldReferenceTransform, m_PickingTransform );
  }
  else
  {
      handleMessage(
      "IGINavigationGUI::SagittalViewPickingCallback could not get coordinate system transform...\n", 0 );
      return;
  }

  m_SagittalPlaneSpatialObject->RemoveObserver( obsId );
    SetImagePickingProcessing();
  }
}

/** -----------------------------------------------------------------
*  Callback function for picking event in the coronal view.
*  Upon receiving a valid picking event, this method will reslice the
*  image to that location.
*---------------------------------------------------------------------
*/
void IGINavigationGUI::CoronalViewPickingCallback( const itk::EventObject & event)
{
  if ( igstk::CoordinateSystemTransformToEvent().CheckEvent( &event ) )
  {
    typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
    const TransformEventType * tmevent =
      dynamic_cast< const TransformEventType *>( & event );

    // get the transform from the view to its parent (reslicer plane)
    igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
    m_PickingTransform = transformCarrier.GetTransform();

    // get the transform from the reslicer plane to its parent (world reference)
    CoordinateSystemTransformObserver::Pointer coordinateObserver =
      CoordinateSystemTransformObserver::New();

    unsigned int obsId = m_CoronalPlaneSpatialObject->AddObserver(
      igstk::CoordinateSystemTransformToEvent(), coordinateObserver );

    m_CoronalPlaneSpatialObject->RequestComputeTransformTo( m_WorldReference );

    if( coordinateObserver->GotCoordinateSystemTransform() )
    {
      igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
      igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
      m_PickingTransform = igstk::Transform::TransformCompose( viewToWorldReferenceTransform, m_PickingTransform );
    }
    else
    {
      handleMessage(
        "IGINavigationGUI::CoronalViewPickingCallback could not get coordinate system transform...\n", 0 );
        return;
    }

    m_CoronalPlaneSpatialObject->RemoveObserver( obsId );
      SetImagePickingProcessing();
  }
}

/** -----------------------------------------------------------------
*  Show progress bar while loading image dataset
*---------------------------------------------------------------------
*/
void IGINavigationGUI::OnITKProgressEvent(itk::Object *source, const itk::EventObject &)
{
  // Get the value of the progress
  float progress = reinterpret_cast<itk::ProcessObject *>(source)->GetProgress();

  if (progress >= 0.0 && progress < 1.0)
  {
    m_GUI.progressBar->setVisible(true);
    // Update the progress bar and value
    m_GUI.progressBar->setValue( 100 * progress );
    QApplication::processEvents();
  }
  else if(progress >= 1.0)
  {
    m_GUI.progressBar->setVisible(false);
  }
}

void IGINavigationGUI::ResetCamera()
{
  m_ViewAxial->RequestResetCamera();
  m_ViewSagittal->RequestResetCamera();
  m_ViewCoronal->RequestResetCamera();
  m_View3D->RequestResetCamera();
}

/** -----------------------------------------------------------------
*  Callback function for observer listening to the slider bar
*  ReslicingEvent
*---------------------------------------------------------------------
*/
void IGINavigationGUI::ResliceImageCallback1( int value )
{
  IndexType index;
  index[2]= m_GUI.AxialSlider->value();
  index[0]= m_GUI.SagittalSlider->value();
  index[1]= m_GUI.CoronalSlider->value();

  PointType point;
  m_CTImageSpatialObject->TransformIndexToPhysicalPoint( index, point );

  const double *data = point.GetVnlVector().data_block();

  m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
  m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
  m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
  m_CrossHair->RequestSetCursorPosition( data );
}

void IGINavigationGUI::ResliceImageCallback2( int value )
{
  IndexType index;
  index[2]= m_GUI.AxialSlider->value();
  index[0]= m_GUI.SagittalSlider->value();
  index[1]= m_GUI.CoronalSlider->value();

  PointType point;
  m_CTImageSpatialObject->TransformIndexToPhysicalPoint( index, point );

  const double *data = point.GetVnlVector().data_block();

  m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
  m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
  m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
  m_CrossHair->RequestSetCursorPosition( data );
}

void IGINavigationGUI::ResliceImageCallback3( int value )
{
  IndexType index;
  index[2]= m_GUI.AxialSlider->value();
  index[0]= m_GUI.SagittalSlider->value();
  index[1]= m_GUI.CoronalSlider->value();

  PointType point;
  m_CTImageSpatialObject->TransformIndexToPhysicalPoint( index, point );

  const double *data = point.GetVnlVector().data_block();

  m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
  m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
  m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
  m_CrossHair->RequestSetCursorPosition( data );
}

/** -----------------------------------------------------------------
*  Reslices image dataset  
*---------------------------------------------------------------------
*/
void IGINavigationGUI::ResliceImage ( IndexType index )
{
  m_GUI.AxialSlider->setValue( index[2] );
  m_GUI.SagittalSlider->setValue( index[0] );
  m_GUI.CoronalSlider->setValue( index[1] );
}

void IGINavigationGUI::LoadConfiguration()
{
  handleMessage("IGIPlanningGUI::LoadConfiguration called...\n", 0);

  /** reset */
  m_MeshMap.clear();
  m_MeshRepresentationMap.clear();
  m_CTImageSpatialObject = NULL;

  /** Load configuration */
  QSettings::Format XmlFormat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);
  QSettings::setPath(XmlFormat, QSettings::UserScope, m_TutorialDir);
  QSettings::setDefaultFormat(XmlFormat);

  std::string file = m_ConfigDir.toStdString()
              + std::string("/")
              + std::string(IGIConfigurationData::CONFIGURATION_NAME)
              + std::string(".xml");

  struct stat sb;
  if (!( stat(file.c_str(), &sb) == 0) )
  {
    handleMessage("Configuration file not found.", 1);
    return;
  }
  m_Settings = new QSettings(XmlFormat, 
                              QSettings::UserScope,
                              IGIConfigurationData::CONFIGURATION_FOLDER,
                              IGIConfigurationData::CONFIGURATION_NAME);
  m_ImageDir = m_Settings->value("ImageDataSetDir","").toString().toStdString();
  m_MeshFileName = 
          m_Settings->value("PointerToolMeshFile","").toString().toStdString();
  m_CameraParametersFile = 
          m_Settings->value("CameraParametersFile","").toString().toStdString();
  m_FiducialSetFilename = m_Settings->value("FiducialSetFile","").toString();
  m_TargetSetFilename = m_Settings->value("TargetSetFile", "").toString();
  m_PointerToolCalibrationFile =
                m_Settings->value("PointerToolCalibrationFile", "").toString();
  m_ReferenceId =  m_Settings->value("ReferenceId", "").toString(); 
  m_PointerId = m_Settings->value("PointerId", "").toString();
  m_Window = m_Settings->value("Window", "").toInt(); 
  m_Level = m_Settings->value("Level", "").toInt();
 
  m_Settings->beginGroup("PointerTools");
  QStringList keys = m_Settings->childKeys();
  for( unsigned int i = 0; i<keys.size(); i++)
  {
    QString key = keys[i];
    m_MarkerIdMeshMap[key] = m_Settings->value(key).toString();
  }
  m_Settings->endGroup();

  /** Setup environment */
  if(m_ImageDir.empty())
  {
    QMessageBox::information(this, "IGI Navigation", "CT dataset directory not defined.");
    return;
  }
  
  if(m_MeshFileName.empty())
  {
    QMessageBox::information(this, "IGI Navigation", "Pointer tool 3D model file name not defined.");
    return;
  }

  if(m_PointerToolCalibrationFile.isEmpty())
  {
    QMessageBox::information(this, "IGI Navigation", "Pointer tool calibration file not defined.");
    return;
  }

  if(m_FiducialSetFilename.isEmpty())
  {
    QMessageBox::information(this, "IGI Navigation", "Fiducial point set file not defined.");
    return;
  }

  if(m_CameraParametersFile.empty())
  {
    QMessageBox::information(this, "IGI Navigation", "Camera calibration file not defined.");
    return;
  }

  if(!LoadToolSpatialObjectAction(m_PointerId,m_MeshFileName))
    return;
 
  if(!LoadImageAction())
    return;

  if(!LoadToolCalibrationTransform(m_PointerToolCalibrationFile))
    return;

  if(!m_FiducialSet->LoadFiducialsFromXMLfile( m_FiducialSetFilename, m_WorldReference.GetPointer()))
    return;

  if(m_TargetSetFilename.isEmpty())
  {}
  else
  {
    if(!m_TargetSet->LoadFiducialsFromXMLfile( m_TargetSetFilename, m_WorldReference.GetPointer()))
      return;
  }
  
  ResetCamera();

  if(!InitializeTrackerAction()) return;

  m_GUI.AcceptFiducialPushButton->setEnabled(true);
  m_GUI.StartRegistrationPushButton->setEnabled(true);
  m_GUI.LoadConfigurationButton->setEnabled(false);

  m_GUI.CheckPositionButton->setEnabled(true);
}

/** -----------------------------------------------------------------
*  Load fiducial points from file
*---------------------------------------------------------------------
*/
void IGINavigationGUI::LoadFiducials()
{
  handleMessage("IGIPlanningGUI::LoadFiducials called...\n", 0 );
}

/** -----------------------------------------------------------------
*  Switches the currently active image fiducial
*---------------------------------------------------------------------
*/
void IGINavigationGUI::SelectFiducial(QString buttonName)
{
  handleMessage("IGIPlanningGUI::RequestChangeSelectedFiducial called...\n", 0 );

  // get fiducial coordinates
  PointType point;
  point = m_FiducialSet->GetPositionOfFiducial(buttonName);

  m_FiducialSet->RepositionFiducial(buttonName,PointToTransform(point),m_WorldReference.GetPointer());
  
  if(m_CTImageSpatialObject.IsNotNull())
  {
    // Reslice image to the selected point position 
    if( m_CTImageSpatialObject->IsInside( point ) )
    {
      IndexType index;
      m_CTImageSpatialObject->TransformPhysicalPointToIndex( point, index);

      const double *data = point.GetVnlVector().data_block();

      m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
      m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
      m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
      m_CrossHair->RequestSetCursorPosition( data );
      this->ResliceImage( index );
    }
    else
    {
      handleMessage("This point is not defined in the image...\n", 0 );
    }
  }
  ResetCamera();
}

/** -----------------------------------------------------------------
*  Switches the currently active image target
*---------------------------------------------------------------------
*/
void IGINavigationGUI::SelectTarget(QString buttonName)
{
  handleMessage("IGIPlanningGUI::RequestChangeSelectedTarget called...\n", 0 );

  // get target coordinates
  PointType point;
  point = m_TargetSet->GetPositionOfFiducial(buttonName);

  m_TargetSet->RepositionFiducial(buttonName,PointToTransform(point),m_WorldReference.GetPointer());
  
  if(m_CTImageSpatialObject.IsNotNull())
  {
    // Reslice image to the selected point position 
    if( m_CTImageSpatialObject->IsInside( point ) )
    {
      IndexType index;
      m_CTImageSpatialObject->TransformPhysicalPointToIndex( point, index);

      const double *data = point.GetVnlVector().data_block();

      m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
      m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
      m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
      m_CrossHair->RequestSetCursorPosition( data );
      this->ResliceImage( index );

      // Get current tool position
      // get transform
      typedef igstk::TransformObserver ObserverType;
      ObserverType::Pointer transformObserver = ObserverType::New();
      transformObserver->ObserveTransformEventsFrom( m_ArucoTrackerTool );
      transformObserver->Clear();
      m_ArucoTrackerTool->RequestComputeTransformTo( m_WorldReference );

      if ( !transformObserver->GotTransform() )
      {  
        handleMessage("SelectTarget: No transform received.\n", 0 );
        return;
      }

      PointType pointerPosition;
      pointerPosition = TransformToPoint( transformObserver->GetTransform());

      double deltaX = point[0] - pointerPosition[0];
      double deltaY = point[1] - pointerPosition[1];
      double deltaZ = point[2] - pointerPosition[2];
      double distanceError = sqrt(pow(deltaX,2) + pow(deltaY,2) + pow(deltaZ,2));
      m_GUI.TargetErrorLabel->setText(QString::number(distanceError,'f',2));
    }
    else
    {
      handleMessage("This point is not defined in the image...\n", 0 );
    }
  }
  ResetCamera();
}

void IGINavigationGUI::PulseTimerEvent()
{
  igstk::PulseGenerator::CheckTimeouts();
}

void IGINavigationGUI::showHelp()
{
  QDialog* helpBox = new QDialog(this);
  helpBox->setWindowTitle("Help");

  QTextBrowser* browser = new QTextBrowser(helpBox); 
  browser->setSource(QUrl::fromLocalFile(m_CurrentPath + "/READMENavigation.html"));
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
void IGINavigationGUI::OnQuitAction()
{
  QMessageBox::StandardButton value = 
    QMessageBox::information(this,
    "IGINavigation:", "Are you sure you want to quit ?",
    QMessageBox::Yes | QMessageBox::No );

  if( value == QMessageBox::Yes )
  {
    this->close();
    m_GUIQuit = true;
  }
}

bool IGINavigationGUI::HasQuit() 
{
  return m_GUIQuit;
}

/** -----------------------------------------------------------------
*     Destructor
*  -----------------------------------------------------------------
*/
IGINavigationGUI::~IGINavigationGUI()
{
  delete m_GUI.Display3D;
  delete m_GUI.DisplayAxial;
  delete m_GUI.DisplayCoronal;
  delete m_GUI.DisplaySagittal;
}

void IGINavigationGUI::CheckMarkerPosition()
{
  cv::Mat currentImage = m_ArucoTracker->GetCurrentVideoFrame(); 
  cv::imshow("video",currentImage);
  cv::waitKey(2000);
  cv::destroyWindow("video");
}

void 
IGINavigationGUI::OnWriteFailureEvent( itk::Object * itkNotUsed(caller), 
                            const itk::EventObject & itkNotUsed(event) )
{
  handleMessage("Failed writing registration result to file.", 1);
}

/** -----------------------------------------------------------------
*  Function saves a transformation to file in XML format
*--------------------------------------------------------------------
*/
void IGINavigationGUI::StoreTransformInXMLFormat(igstk::Transform transform)
{
  std::string file = m_ConfigDir.toStdString()
                       + std::string("/")
                       + std::string(IGIConfigurationData::REGISTRATION_OUPUT_NAME);

  igstk::PrecomputedTransformData::Pointer transformationData = 
  igstk::PrecomputedTransformData::New();

  //get transformation description
  std::ostringstream descriptionStream;
  descriptionStream<<"Object to Image transformation ";
    
  //get current date
  std::string estimationDate = 
  itksys::SystemTools::GetCurrentDateTime( "%Y %b %d %H:%M:%S" );
    
  transformationData->RequestInitialize( &transform, estimationDate,
                    descriptionStream.str(), m_TrackerRMS );
  
  //setup the writer
  igstk::TransformFileWriter::Pointer transformFileWriter = 
  igstk::TransformFileWriter::New();

  igstk::TransformXMLFileWriterBase::Pointer xmlFileWriter;
  xmlFileWriter = igstk::RigidTransformXMLFileWriter::New();

  WriteFailureObserverType::Pointer writeFailureObserver = 
  WriteFailureObserverType::New();  
  writeFailureObserver->SetCallbackFunction( 
  this,
  &IGINavigationGUI::OnWriteFailureEvent );

  transformFileWriter->AddObserver( igstk::TransformFileWriter::WriteFailureEvent() , 
                writeFailureObserver );

  transformFileWriter->RequestSetWriter( xmlFileWriter );
  transformFileWriter->RequestSetData( transformationData,  
                                      file );
  transformFileWriter->RequestWrite();
}

/** -----------------------------------------------------------------
*  Construct an error observer for all the possible errors that occur in 
*   the observed IGSTK components.
*---------------------------------------------------------------------
*/
IGINavigationGUI::ErrorObserver::ErrorObserver() : m_ErrorOccured( false )
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
void IGINavigationGUI::ErrorObserver::Execute( const itk::Object * itkNotUsed(caller), 
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
void IGINavigationGUI::ErrorObserver::Execute( itk::Object *caller, 
  const itk::EventObject & event ) throw (std::exception)
{
  const itk::Object * constCaller = caller;
  this->Execute(constCaller, event);
}

