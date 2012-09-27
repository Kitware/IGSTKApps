#include "IGIPlanningGUI.h"

#include "igstkTransformObserver.h"
#include "itkTextOutput.h"

#include "../IGIConfigurationData.h"

#define VIEW_2D_REFRESH_RATE 30
#define VIEW_3D_REFRESH_RATE 30
#define DEFAULT_ZOOM_FACTOR 1.5

/** ---------------------------------------------------------------
*     Constructor
* -----------------------------------------------------------------
*/
IGIPlanningGUI::IGIPlanningGUI()
{
  /** Initializations */
  m_GUI.setupUi(this);
  m_GUIQuit = false;
  m_ImageLoaded = false;
  m_GUI.progressBar->setValue( 0);
  m_GUI.WizardTab->setCurrentIndex(0);
  m_CurrentPointSetSaved = true;
  m_Window = 4000;
  m_Level = 1000;

  m_WindowLevelSlider = new ctkRangeSlider(this);
  m_WindowLevelSlider->setOrientation(Qt::Horizontal);
  m_WindowLevelSlider->setRange(-1024,3071);
  m_WindowLevelSlider->setTracking(true);
  m_WindowLevelSlider->setValues(m_Level - m_Window / 2, m_Level + m_Window / 2);
  m_GUI.WindowLevelLayout->addWidget(m_WindowLevelSlider);
  m_WindowLevelSlider->setEnabled(false);

  this->CreateActions();

  // suppress itk output window
  itk::OutputWindow::SetInstance(itk::TextOutput::New());
  
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

  SetupView();

  m_FiducialSet = new FiducialSet(this);
  m_FiducialSet->AddView("view3D",m_View3D.GetPointer());
  m_FiducialSet->AddView("axial",m_ViewAxial.GetPointer());
  m_FiducialSet->AddView("sagittal",m_ViewSagittal.GetPointer());
  m_FiducialSet->AddView("coronal",m_ViewCoronal.GetPointer());
  m_GUI.TabLayout->addWidget(m_FiducialSet->GetButtonWidget());
  connect(m_FiducialSet->GetButtonWidget(),SIGNAL(clicked(QString)),this,SLOT(SelectFiducial(QString)));

  m_GUI.AxialSlider->setEnabled(false);
  m_GUI.CoronalSlider->setEnabled(false);
  m_GUI.SagittalSlider->setEnabled(false);

  m_PointCoordsAnnotation = igstk::Annotation2D::New();

  m_ImageDir="";
  m_CameraCalibrationFile="";
  m_PointerToolCalibrationFile="";
  m_MeshFileName="";
  m_FiducialSetFilename="";
  m_TargetSetFilename="";

  /** Default marker settings */
  std::string marker102 = "102";
  std::string marker403 = "403";
  std::string marker502 = "502";
  std::string marker201 = "201";
  std::string marker202 = "202";
  
  m_ReferenceId = "102";
  m_PointerId = "403";    

  m_ReferenceRadioButtons[marker102] = m_GUI.reference102Radio;
  m_ReferenceRadioButtons[marker403] = m_GUI.reference403Radio;
  m_ReferenceRadioButtons[marker502] = m_GUI.reference502Radio;
  m_ReferenceRadioButtons[marker201] = m_GUI.reference201Radio;
  m_ReferenceRadioButtons[marker202] = m_GUI.reference202Radio;

  QButtonGroup* referenceRadioButtonGroup = new QButtonGroup(this);
  referenceRadioButtonGroup->addButton(m_GUI.reference102Radio);
  referenceRadioButtonGroup->addButton(m_GUI.reference403Radio);
  referenceRadioButtonGroup->addButton(m_GUI.reference502Radio);
  referenceRadioButtonGroup->addButton(m_GUI.reference201Radio);
  referenceRadioButtonGroup->addButton(m_GUI.reference202Radio);

  m_PointerRadioButtons[marker102] = m_GUI.pointer102Radio;
  m_PointerRadioButtons[marker403] = m_GUI.pointer403Radio;
  m_PointerRadioButtons[marker502] = m_GUI.pointer502Radio;
  m_PointerRadioButtons[marker201] = m_GUI.pointer201Radio;
  m_PointerRadioButtons[marker202] = m_GUI.pointer202Radio;

  QButtonGroup* pointerRadioButtonGroup = new QButtonGroup(this);
  pointerRadioButtonGroup->addButton(m_GUI.pointer102Radio);
  pointerRadioButtonGroup->addButton(m_GUI.pointer403Radio);
  pointerRadioButtonGroup->addButton(m_GUI.pointer502Radio);
  pointerRadioButtonGroup->addButton(m_GUI.pointer201Radio);
  pointerRadioButtonGroup->addButton(m_GUI.pointer202Radio);

  QSignalMapper *signalMapperCheckBox = new QSignalMapper(this);
  connect(signalMapperCheckBox, SIGNAL(mapped(QWidget*)), this, SLOT(OnCheckBoxToggled(QWidget*)));
  
  signalMapperCheckBox->setMapping(m_GUI.A,m_GUI.A);
  signalMapperCheckBox->setMapping(m_GUI.B,m_GUI.B);
  signalMapperCheckBox->setMapping(m_GUI.C,m_GUI.C);
  signalMapperCheckBox->setMapping(m_GUI.D,m_GUI.D);
  signalMapperCheckBox->setMapping(m_GUI.E,m_GUI.E);

  connect(m_GUI.A, SIGNAL(clicked()), signalMapperCheckBox, SLOT(map()));
  connect(m_GUI.B, SIGNAL(clicked()), signalMapperCheckBox, SLOT(map())); 
  connect(m_GUI.C, SIGNAL(clicked()), signalMapperCheckBox, SLOT(map())); 
  connect(m_GUI.D, SIGNAL(clicked()), signalMapperCheckBox, SLOT(map())); 
  connect(m_GUI.E, SIGNAL(clicked()), signalMapperCheckBox, SLOT(map())); 

  m_MarkerCheckBoxes[marker102]=m_GUI.A;
  m_MarkerCheckBoxes[marker403]=m_GUI.B;
  m_MarkerCheckBoxes[marker502]=m_GUI.C;
  m_MarkerCheckBoxes[marker201]=m_GUI.D;
  m_MarkerCheckBoxes[marker202]=m_GUI.E;
}

/** -----------------------------------------------------------------
*     Connect GUI elements with actions
*  -----------------------------------------------------------------
*/
void IGIPlanningGUI::CreateActions()
{
  connect(m_GUI.QuitPushButton, SIGNAL(clicked()), this, SLOT(OnQuitAction()));
  connect(m_GUI.LoadImagePushButton, SIGNAL(clicked()), this, SLOT(LoadImageAction()));

  //Fiducial operations
  connect(m_GUI.LoadFiducialsButton, SIGNAL( clicked()), this, SLOT(LoadFiducials()));
  connect(m_GUI.AddFiducialButton, SIGNAL( clicked()), this, SLOT(AddNewFiducial()));
  connect(m_GUI.delFiducialButton, SIGNAL( clicked()), this, SLOT( RemoveFiducial()));
  connect(m_GUI.newFiducialSetButton, SIGNAL( clicked()), this, SLOT( NewFiducialSet()));
  connect(m_GUI.saveFiducialSetButton, SIGNAL( clicked()), this, SLOT( SaveFiducials()));

  connect(m_GUI.selectCalibrationFileButton, SIGNAL( clicked()), this, SLOT(SelectCameraCalibrationFile()));
  connect(m_GUI.selectToolCalibrationFileButton, SIGNAL( clicked()), this, SLOT(SelectPointerToolCalibrationFile()));
  connect(m_GUI.selectToolMeshFileButton, SIGNAL( clicked()), this, SLOT(LoadToolSpatialObjectAction()));
  connect(m_GUI.saveConfigurationButton, SIGNAL( clicked()), this, SLOT( SaveConfiguration()));
  connect(m_GUI.AxialSlider, SIGNAL( valueChanged(int)), this, SLOT(ResliceImageCallback1(int)));
  connect(m_GUI.SagittalSlider, SIGNAL( valueChanged(int)), this, SLOT(ResliceImageCallback2(int)));
  connect(m_GUI.CoronalSlider, SIGNAL( valueChanged(int)), this, SLOT(ResliceImageCallback3(int)));

  connect(m_GUI.reference102Radio, SIGNAL(toggled(bool)), this, SLOT(reference102RadioChecked(bool)) );
  connect(m_GUI.reference403Radio, SIGNAL(toggled(bool)), this, SLOT(reference403RadioChecked(bool)) );
  connect(m_GUI.reference502Radio, SIGNAL(toggled(bool)), this, SLOT(reference502RadioChecked(bool)) );
  connect(m_GUI.reference201Radio, SIGNAL(toggled(bool)), this, SLOT(reference201RadioChecked(bool)) );
  connect(m_GUI.reference202Radio, SIGNAL(toggled(bool)), this, SLOT(reference202RadioChecked(bool)) );

  connect(m_GUI.pointer102Radio, SIGNAL(toggled(bool)), this, SLOT(pointer102RadioChecked(bool)) );
  connect(m_GUI.pointer403Radio, SIGNAL(toggled(bool)), this, SLOT(pointer403RadioChecked(bool)) );
  connect(m_GUI.pointer502Radio, SIGNAL(toggled(bool)), this, SLOT(pointer502RadioChecked(bool)) );
  connect(m_GUI.pointer201Radio, SIGNAL(toggled(bool)), this, SLOT(pointer201RadioChecked(bool)) );
  connect(m_GUI.pointer202Radio, SIGNAL(toggled(bool)), this, SLOT(pointer202RadioChecked(bool)) );

  connect(m_GUI.firstNextButton,SIGNAL( clicked()), this, SLOT( NextTab()));
  connect(m_GUI.secondNextButton,SIGNAL( clicked()), this, SLOT( NextTab()));
  connect(m_GUI.secondPreviousButton,SIGNAL( clicked()), this, SLOT( PreviousTab()));
  connect(m_GUI.thirdPreviousButton,SIGNAL( clicked()), this, SLOT( PreviousTab()));
  
  connect(m_GUI.helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
  connect(m_WindowLevelSlider, SIGNAL(valuesChanged(int,int)), this, SLOT(SetWindowLevelThroughRange(int, int)));
}

void IGIPlanningGUI::NextTab()
{
  m_GUI.WizardTab->setCurrentIndex(m_GUI.WizardTab->currentIndex()+1);
}

void IGIPlanningGUI::PreviousTab()
{
  m_GUI.WizardTab->setCurrentIndex(m_GUI.WizardTab->currentIndex()-1);
}


void IGIPlanningGUI::SetupView()
{
  //Create views
  m_ViewAxial = View2DType::New();
  m_ViewSagittal = View2DType::New();
  m_ViewCoronal = View2DType::New();
  m_View3D = View3DType::New();

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

void IGIPlanningGUI::LoadImageAction()
{
  handleMessage("IGIPlanningGUI::LoadImageProcessing called...\n", 0);

  QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
  QString directory = QFileDialog::getExistingDirectory(this,
                                                        "Select DICOM directory.",
                                                        "../Data/CT",
                                                        options);
  if(directory.isNull())
    return;
  if (directory.isEmpty())
  {
    handleMessage("No directory was selected.\n",1);
    return;
  }

  m_ImageDir = directory.toStdString();

  //check for slash
  if (itksys::SystemTools::StringEndsWith(m_ImageDir.c_str(),"/") )
  {
    m_ImageDir = m_ImageDir.substr (0,m_ImageDir.length()-1);
  }
  handleMessage( std::string("Set image directory: " + m_ImageDir + "\n"), 0 );

  /** Setup image reader  */
  CTImageReaderType::Pointer CTImageReader        = CTImageReaderType::New();
  CTImageReader->SetLogger( this->GetLogger() );

  /** Build itk progress command to assess image load progress */
  itk::SmartPointer<ProgressCommandType>            progressCommand;
  progressCommand = ProgressCommandType::New();

  /** Set the callback to the itk progress command */
  progressCommand->SetCallbackFunction( this, &IGIPlanningGUI::OnITKProgressEvent );

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
    return;
  }

  if( dndcb->GotDICOMImageReaderNonExistingDirectoryError() )
  {
    handleMessage("Image directory does not exist.\n", 1 );
    return;
  }

  if( decb->GotDICOMImageReaderEmptyDirectoryError() )
  {
    handleMessage("Image directory is empty.\n", 1 );
    return;
  }

  if( ddhefcb->GotDICOMImageDirectoryNameDoesNotHaveEnoughFilesError() )
  {
    handleMessage("Image directory does not have enough files.\n", 1 );
    return;
  }

  if( disgcb->GotDICOMImageDirectoryDoesNotContainValidDICOMSeriesError() )
  {
    handleMessage("Image directory does not contain valid DICOM series.\n", 1 );
    return;
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
    return;
  }
  if ( CTImageReader->FileSuccessfullyRead() )
  {
    m_GUI.PatientNameLabel->setText( CTImageReader->GetPatientName().c_str() );

    if ( CTImageObserver.IsNotNull() )
    {
      m_CTImageSpatialObject = CTImageObserver->GetCTImage();
      this->ConnectImageRepresentation();
      m_GUI.AxialSlider->setEnabled(true);
      m_GUI.CoronalSlider->setEnabled(true);
      m_GUI.SagittalSlider->setEnabled(true);
      this->ResetCamera();
      m_GUI.imagePathEdit->setText(QString(m_ImageDir.c_str()));

      m_GUI.newFiducialSetButton->setEnabled(true);
      m_GUI.LoadFiducialsButton->setEnabled(true);
    }
  }
}

/** -----------------------------------------------------------------
* Set ImageSpatialObjects to
* ImageRepresentations, sets image orientations, adds ImageSpatialObjects
* to Views and connects the scene graph
*---------------------------------------------------------------------
*/
void IGIPlanningGUI::ConnectImageRepresentation()
{
  handleMessage("IGIPlanningGUI::ConnectImageRepresentation called...\n", 0 );

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
  m_AxialPlaneRepresentation->SetFrameColor(1,0,0);
  m_AxialPlaneRepresentation->RequestSetImageSpatialObject( m_CTImageSpatialObject );
  m_AxialPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_AxialPlaneSpatialObject );

  // create reslice plane representation for sagittal view
  m_SagittalPlaneRepresentation = CTImageRepresentationType::New();
  m_SagittalPlaneRepresentation->SetFrameColor(0,1,0);
  m_SagittalPlaneRepresentation->RequestSetImageSpatialObject( m_CTImageSpatialObject );
  m_SagittalPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_SagittalPlaneSpatialObject );

  // create reslice plane representation for coronal view
  m_CoronalPlaneRepresentation = CTImageRepresentationType::New();
  m_CoronalPlaneRepresentation->SetFrameColor(0,0,1);
  m_CoronalPlaneRepresentation->RequestSetImageSpatialObject( m_CTImageSpatialObject );
  m_CoronalPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_CoronalPlaneSpatialObject );

  /** SCENE GRAPH: World, image, reslicer planes and multimodal views */
  m_AxialPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
  m_SagittalPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
  m_CoronalPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );

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
  
  m_PointCoordsAnnotation->RequestSetFontColor( 0, 1.0, 0.0, 0.0 ); 

  m_View3D->RequestAddAnnotation2D(m_PointCoordsAnnotation);
  
  /** Set up cross hairs */
  m_CrossHair = CrossHairType::New();
  m_CrossHair->RequestSetBoundingBoxProviderSpatialObject( m_CTImageSpatialObject );

  // build the cross hair representations
  m_AxialCrossHairRepresentation = CrossHairRepresentationType::New();
  //m_AxialCrossHairRepresentation->SetColor(0,1,0);
  m_AxialCrossHairRepresentation->SetOpacity(1);
  m_AxialCrossHairRepresentation->SetLineWidth(2);
  m_AxialCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );

  m_SagittalCrossHairRepresentation = CrossHairRepresentationType::New();
  //m_SagittalCrossHairRepresentation->SetColor(0,1,0);
  m_SagittalCrossHairRepresentation->SetOpacity(1);
  m_SagittalCrossHairRepresentation->SetLineWidth(2);
  m_SagittalCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );

  m_CoronalCrossHairRepresentation = CrossHairRepresentationType::New();
  //m_CoronalCrossHairRepresentation->SetColor(0,1,0);
  m_CoronalCrossHairRepresentation->SetOpacity(1);
  m_CoronalCrossHairRepresentation->SetLineWidth(2);
  m_CoronalCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );
  //delete 3D cross hair
  m_3DViewCrossHairRepresentation = CrossHairRepresentationType::New();
  //m_3DViewCrossHairRepresentation->SetColor(1,1,0);
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
  
  //** Add observer for picking events in the Axial view */
  m_AxialViewPickerObserver = LoadedObserverType::New();
  m_AxialViewPickerObserver->SetCallbackFunction( this, &IGIPlanningGUI::AxialViewPickingCallback );

  m_ViewAxial->AddObserver(
  igstk::CoordinateSystemTransformToEvent(), m_AxialViewPickerObserver );
  
  /** Add observer for picking events in the Axial view */
  m_SagittalViewPickerObserver = LoadedObserverType::New();
  m_SagittalViewPickerObserver->SetCallbackFunction( this, &IGIPlanningGUI::SagittalViewPickingCallback );

  m_ViewSagittal->AddObserver(
  igstk::CoordinateSystemTransformToEvent(), m_SagittalViewPickerObserver );

  /** Add observer for picking events in the Axial view */
  m_CoronalViewPickerObserver = LoadedObserverType::New();
  m_CoronalViewPickerObserver->SetCallbackFunction( this, &IGIPlanningGUI::CoronalViewPickingCallback );

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
  m_ImageLoaded = true;
  SetWindowLevelThroughRange(m_Level - m_Window / 2, m_Level + m_Window / 2);
  m_WindowLevelSlider->setEnabled(true);
}

/** -----------------------------------------------------------------
* Set window and level values of the CT image
*---------------------------------------------------------------------
*/
void IGIPlanningGUI::SetWindowLevelThroughRange(int min, int max)
{
  if(m_ImageLoaded)
  {
    m_Window = max - min;
    m_Level = min + m_Window/2;
    m_AxialPlaneRepresentation->SetWindowLevel( m_Window, m_Level );
    m_SagittalPlaneRepresentation->SetWindowLevel( m_Window, m_Level );
    m_CoronalPlaneRepresentation->SetWindowLevel( m_Window, m_Level );

    m_AxialPlaneRepresentation2->SetWindowLevel( m_Window, m_Level );
    m_SagittalPlaneRepresentation2->SetWindowLevel( m_Window, m_Level );
    m_CoronalPlaneRepresentation2->SetWindowLevel( m_Window, m_Level );

    m_GUI.WindowTextfield->setText(QString::number(m_Window));
    m_GUI.LevelTextfield->setText(QString::number(m_Level));
  }
}
/** -----------------------------------------------------------------
* Load tool spatial object mesh. This method asks for a file with the
* spatial object mesh in the .msh format (see mesh SpatialObject in ITK)
*  -----------------------------------------------------------------
*/
void IGIPlanningGUI::LoadToolSpatialObjectAction()
{
  handleMessage(
     "IGIPlanningGUI::LoadToolSpatialObjectAction called...\n", 0 );

  QString fName = QFileDialog::getOpenFileName(this,
    "Select 3D Model for Pointer Tool.",
    "../Data/MeshModels",
    "3D Mesh Model File (*.msh)");

  if(fName.isNull())
    return;

  if (fName.isEmpty())
  {
    handleMessage("No mesh model file [*.msh] was selected.\n",1);
    return;
  }

  m_MeshFileName = fName;

  MeshReaderType::Pointer reader = MeshReaderType::New();
  reader->RequestSetFileName( m_MeshFileName.toStdString() );
  reader->RequestReadObject();

  MeshObjectObserver::Pointer observer = MeshObjectObserver::New();

  reader->AddObserver( igstk::MeshReader::MeshModifiedEvent(), observer);
  reader->RequestGetOutput();

  if( !observer->GotMeshObject() )
  {
    handleMessage("Cannot read 3D model.\n", 1 );
    return;
  }

  m_GUI.toolMeshPathEdit->setText(m_MeshFileName);
  m_ToolSpatialObject = observer->GetMeshObject();
  
  if ( m_ToolSpatialObject.IsNotNull() )
  {
    m_ToolRepresentation = MeshRepresentationType::New();
    m_ToolRepresentation->RequestSetMeshObject( m_ToolSpatialObject );
    m_ToolRepresentation->SetOpacity(1.0);
    m_ToolRepresentation->SetColor(0,0,1);
  }
  else
  {
    handleMessage("Tool visualization could not added to the views.", 1 ); 
  }
}

void IGIPlanningGUI::SelectCameraCalibrationFile()
{
  handleMessage("IGIPlanningGUI::SelectCameraCalibrationFile called...\n", 0);

  QString fName = QFileDialog::getOpenFileName(this,
      "Select Camera Calibration File.",
      "../Configuration",
      "Camera Calibration File (*.yml)");

  if(fName.isNull())
    return;
  if (fName.isEmpty())
  {
    handleMessage("No camera calibration file [*.yml] was selected.\n",1);
    return;
  }

  m_GUI.cameraCalibPathEdit->setText(fName);
  m_CameraCalibrationFile = fName.toStdString();
}

void IGIPlanningGUI::SelectPointerToolCalibrationFile()
{
  handleMessage("IGIPlanningGUI::SelectPointerToolCalibrationFile called...\n", 0);

  QString fName = QFileDialog::getOpenFileName(this,
      "Select Pointer Tool Calibration File.",
      "../Configuration",
      "Pointer Tool Calibration File (*.xml)");

  if(fName.isNull())
    return;
  if (fName.isEmpty())
  {
    handleMessage("No file was selected\n",1);
    return;
  }

  m_GUI.toolCalibPathEdit->setText(fName);
  m_PointerToolCalibrationFile = fName.toStdString();
}

void IGIPlanningGUI::SaveConfiguration()
{
  if(m_ImageDir.empty())
  {
    QMessageBox::information(this, "IGI Planning", "CT dataset directory not defined.");
    m_GUI.WizardTab->setCurrentIndex(0);
    return;
  }
  
  if(m_MeshFileName.isEmpty())
  {
    QMessageBox::information(this, "IGI Planning", "Pointer tool 3D model file name not defined.");
    m_GUI.WizardTab->setCurrentIndex(0);
    return;
  }

  if(m_PointerToolCalibrationFile.empty())
  {
    QMessageBox::information(this, "IGI Planning", "Pointer tool calibration file not defined.");
    m_GUI.WizardTab->setCurrentIndex(0);
    return;
  }

  if(m_FiducialSetFilename.isEmpty())
  {
    QMessageBox::information(this, "IGI Planning", "Fiducial point set file not defined.");
    m_GUI.WizardTab->setCurrentIndex(1);
    return;
  }

  if(!m_CurrentPointSetSaved)
  {
      QMessageBox::StandardButton value = 
    QMessageBox::information(this,
    "IGIPlanning:", "Unsaved point set exists. Proceed without saving current point set?",
    QMessageBox::Yes | QMessageBox::No );

    if( value == QMessageBox::Yes )
    {
      
    }
    else
    {
      m_GUI.WizardTab->setCurrentIndex(1);
      SaveFiducials();
      return;
    }
  }

  if(m_CameraCalibrationFile.empty())
  {
    QMessageBox::information(this, "IGI Planning", "Camera calibration file not defined.");
    m_GUI.WizardTab->setCurrentIndex(0);
    return;
  }
	
  QDir currentDir = QDir::current();
  currentDir.cdUp();
  QString configDir = currentDir.absolutePath();

  /** Configuration in XML */
  QSettings::Format XmlFormat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);
  QSettings::setPath(XmlFormat, QSettings::UserScope, configDir);
  QSettings::setDefaultFormat(XmlFormat);

  m_Settings = new QSettings(XmlFormat, 
                              QSettings::UserScope,
                              IGIConfigurationData::CONFIGURATION_FOLDER,
                              IGIConfigurationData::CONFIGURATION_NAME);
  m_Settings->setValue("ImageDataSetDir", QString(QDir::toNativeSeparators(m_ImageDir.c_str())));
  m_Settings->setValue("PointerToolMeshFile", QDir::toNativeSeparators(m_MeshFileName));
  m_Settings->setValue("CameraParametersFile", QDir::toNativeSeparators(QString(m_CameraCalibrationFile.c_str())));
  m_Settings->setValue("FiducialSetFile", QDir::toNativeSeparators(m_FiducialSetFilename)); 
  m_Settings->setValue("TargetSetFile", QDir::toNativeSeparators(m_TargetSetFilename));
  m_Settings->setValue("PointerToolCalibrationFile", QDir::toNativeSeparators(QString(m_PointerToolCalibrationFile.c_str())));
  m_Settings->setValue("ReferenceId", QString(m_ReferenceId.c_str())); 
  m_Settings->setValue("PointerId", QString(m_PointerId.c_str()));
  m_Settings->setValue("Window", QString::number(m_Window)); 
  m_Settings->setValue("Level", QString::number(m_Level));
 
  m_Settings->beginGroup("PointerTools");
    if(m_GUI.Message102Edit->text().length() > 0)
      m_Settings->setValue("id102", QDir::toNativeSeparators(m_GUI.Message102Edit->text()));

    if(m_GUI.Message403Edit->text().length() > 0)
      m_Settings->setValue("id403", QDir::toNativeSeparators(m_GUI.Message403Edit->text()));

    if(m_GUI.Message502Edit->text().length() > 0)
      m_Settings->setValue("id502", QDir::toNativeSeparators(m_GUI.Message502Edit->text()));

    if(m_GUI.Message201Edit->text().length() > 0)
      m_Settings->setValue("id201", QDir::toNativeSeparators(m_GUI.Message201Edit->text()));

    if(m_GUI.Message202Edit->text().length() > 0)
      m_Settings->setValue("id202", QDir::toNativeSeparators(m_GUI.Message202Edit->text()));
  m_Settings->endGroup();

  QMessageBox::information(this,windowTitle(), 
    QString("Configuration file saved as:\n") + configDir + "/" + 
            IGIConfigurationData::CONFIGURATION_FOLDER + "/" + 
            IGIConfigurationData::CONFIGURATION_NAME + ".xml");
}

void IGIPlanningGUI::showHelp()
{
  QDialog* helpBox = new QDialog(this);
  helpBox->setWindowTitle("Help");

  QTextBrowser* browser = new QTextBrowser(helpBox); 
  browser->setSource(*new QUrl("READMEPlanning.html"));
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

void IGIPlanningGUI::setMarkerDialogState(std::string oldSelection, std::string newSelection, int select)
{
  m_MarkerCheckBoxes[oldSelection]->setEnabled(true);
  m_MarkerCheckBoxes[newSelection]->setEnabled(false);
  
  if(select == 0)
  {  
    m_PointerRadioButtons[oldSelection]->setEnabled(true);
    m_PointerRadioButtons[newSelection]->setEnabled(false);
  }

  if(select == 1)
  {  
    m_ReferenceRadioButtons[oldSelection]->setEnabled(true);
    m_ReferenceRadioButtons[newSelection]->setEnabled(false);
  }
}

void IGIPlanningGUI::reference102RadioChecked(bool)
{
  std::string newId = "102";
  setMarkerDialogState(m_ReferenceId,newId,0); 
  m_ReferenceId = newId;
  m_GUI.Message102Edit->setText("");
}

void IGIPlanningGUI::reference403RadioChecked(bool)
{
  std::string newId = "403";
  setMarkerDialogState(m_ReferenceId,newId,0); 
  m_ReferenceId = newId;
  m_GUI.Message403Edit->setText("");
}

void IGIPlanningGUI::reference502RadioChecked(bool)
{
  std::string newId = "502";
  setMarkerDialogState(m_ReferenceId,newId,0); 
  m_ReferenceId = newId;
  m_GUI.Message502Edit->setText("");
}

void IGIPlanningGUI::reference201RadioChecked(bool)
{
  std::string newId = "201";
  setMarkerDialogState(m_ReferenceId,newId,0); 
  m_ReferenceId = newId;
  m_GUI.Message201Edit->setText("");
}

void IGIPlanningGUI::reference202RadioChecked(bool)
{
  std::string newId = "202";
  setMarkerDialogState(m_ReferenceId,newId,0); 
  m_ReferenceId = newId;
  m_GUI.Message202Edit->setText("");
}

void IGIPlanningGUI::pointer102RadioChecked(bool)
{
  std::string newId = "102";
  setMarkerDialogState(m_PointerId,newId,1); 
  m_PointerId = newId;
  m_GUI.Message102Edit->setText("");
}

void IGIPlanningGUI::pointer403RadioChecked(bool)
{
  std::string newId = "403";
  setMarkerDialogState(m_PointerId,newId,1); 
  m_PointerId = newId;
  m_GUI.Message403Edit->setText("");
}

void IGIPlanningGUI::pointer502RadioChecked(bool)
{
  std::string newId = "502";
  setMarkerDialogState(m_PointerId,newId,1); 
  m_PointerId = newId;
  m_GUI.Message502Edit->setText("");
}

void IGIPlanningGUI::pointer201RadioChecked(bool)
{
  std::string newId = "201";
  setMarkerDialogState(m_PointerId,newId,1); 
  m_PointerId = newId;
  m_GUI.Message201Edit->setText("");
}

void IGIPlanningGUI::pointer202RadioChecked(bool)
{
  std::string newId = "202";
  setMarkerDialogState(m_PointerId,newId,1); 
  m_PointerId = newId;
  m_GUI.Message202Edit->setText("");
}

void IGIPlanningGUI::OnCheckBoxToggled(QWidget * checkBox)
{
  handleMessage("IGIPlanningGUI::OnCheckBoxToggled called...\n", 0);

  QCheckBox *cbx = qobject_cast<QCheckBox*>(checkBox);
   if (cbx == 0)
      return;

  QString cbxName = cbx->objectName();
  char cbxInitial = cbxName.toStdString().c_str()[0];

  QString fName = "";
  if(cbx->isChecked())
  {
	  QFileDialog::Options options;
	  QString selectedFilter;
	  fName = QFileDialog::getOpenFileName(this,
		  "Select 3D model for tool.",
		  "../Data/MeshModels",
		  "3D model file (*.msh)",
		  &selectedFilter,
		  options);
  
	  if(fName.isNull())
	  {
		  cbx->setChecked(false);
		  return;
	  }
  }
	switch(cbxInitial)
	{
	case 'A': m_GUI.Message102Edit->setText(fName); break;
	case 'B': m_GUI.Message403Edit->setText(fName); break;
	case 'C': m_GUI.Message502Edit->setText(fName); break;
	case 'D': m_GUI.Message201Edit->setText(fName); break;
	case 'E': m_GUI.Message202Edit->setText(fName); break;
	}
}

void IGIPlanningGUI::OffButtonClicked(int offButton)
{
  switch(offButton)
  {
    case 0: m_GUI.Message102Edit->setText(""); break;
    case 1: m_GUI.Message403Edit->setText(""); break;
    case 2: m_GUI.Message502Edit->setText(""); break;
    case 3: m_GUI.Message201Edit->setText(""); break;
    case 4: m_GUI.Message202Edit->setText(""); break;
  }
}

void IGIPlanningGUI::keyPressEvent ( QKeyEvent * event)
{
  switch ( event->key() )
  {
    case Qt::Key_S: 
      SaveConfiguration();
      break;
    default:
      return;
  }
}

// Reslices the views to the picked position
void IGIPlanningGUI::SetImagePickingProcessing()
{
  handleMessage(
      "IGIPlanningGUI::SetImagePickingProcessing called...\n", 0 );

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
void IGIPlanningGUI::AxialViewPickingCallback( const itk::EventObject & event)
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
      "IGIPlanningGUI::AxialViewPickingCallback could not get coordinate system transform...\n", 0 );
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
void IGIPlanningGUI::SagittalViewPickingCallback( const itk::EventObject & event)
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
      "IGIPlanningGUI::SagittalViewPickingCallback could not get coordinate system transform...\n", 0 );
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
void IGIPlanningGUI::CoronalViewPickingCallback( const itk::EventObject & event)
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
        "IGIPlanningGUI::CoronalViewPickingCallback could not get coordinate system transform...\n", 0 );
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
void IGIPlanningGUI::OnITKProgressEvent(itk::Object *source, const itk::EventObject &)
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

void IGIPlanningGUI::ResetCamera()
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
void IGIPlanningGUI::ResliceImageCallback1( int value )
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

void IGIPlanningGUI::ResliceImageCallback2( int value )
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

void IGIPlanningGUI::ResliceImageCallback3( int value )
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
void IGIPlanningGUI::ResliceImage ( IndexType index )
{
  m_GUI.AxialSlider->setValue( index[2] );
  m_GUI.SagittalSlider->setValue( index[0] );
  m_GUI.CoronalSlider->setValue( index[1] );
}

void IGIPlanningGUI::NewFiducialSet()
{
  m_FiducialSet->Clear();
  // GUI
  m_GUI.AddFiducialButton->setEnabled(true);
  m_GUI.delFiducialButton->setEnabled(true);
  m_GUI.saveFiducialSetButton->setEnabled(true);
  m_GUI.RegPointsCheckBox->setEnabled(true);

  m_CurrentPointSetSaved = false;
}

/** -----------------------------------------------------------------
*  Load fiducial points from file
*---------------------------------------------------------------------
*/
void IGIPlanningGUI::LoadFiducials()
{
  handleMessage("IGIPlanningGUI::LoadFiducials called...\n", 0 );

  NewFiducialSet();
  m_FiducialSet->LoadFiducialsFromXMLfile(m_WorldReference);
}

void IGIPlanningGUI::AddNewFiducial()
{
  handleMessage("IGIPlanningGUI::SetImageFiducial called...\n", 0 );

  PointType point = TransformToPoint( m_PickingTransform );

  if( m_CTImageSpatialObject->IsInside( point ) )
  {
    m_FiducialSet->AddNewFiducial(point);
  }
  else
  {
    handleMessage( "Picked point outside image...\n", 0 );
  }
}

void IGIPlanningGUI::RemoveFiducial()
{
  m_FiducialSet->RemoveSelectedFiducial(); 
}

/** -----------------------------------------------------------------
*  Switches the currently active image fiducial
*---------------------------------------------------------------------
*/
void IGIPlanningGUI::SelectFiducial(QString buttonName)
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
  //ResetCamera();
}

/**
 *  Write the image fiducials to file
 */
void IGIPlanningGUI::SaveFiducials()
{
  handleMessage("IGIPlanningGUI::SaveFiducials called...\n", 0 );
  
  if(m_GUI.RegPointsCheckBox->isChecked())
  {
    if(m_FiducialSet->GetNumberOfElements() < 3 )
    {
      handleMessage("Define three or more fiducials for registration.", 1);
      return;
    }
    m_FiducialSetFilename = m_FiducialSet->SaveFiducials();
  }
  else
  {
    m_TargetSetFilename = m_FiducialSet->SaveFiducials();
  }
  m_CurrentPointSetSaved = true;
}

void IGIPlanningGUI::PulseTimerEvent()
{
  igstk::PulseGenerator::CheckTimeouts();
}

/**-----------------------------------------------------------------
*  Show quit dialog
*---------------------------------------------------------------------
*/
void IGIPlanningGUI::OnQuitAction()
{
  QMessageBox::StandardButton value = 
    QMessageBox::information(this,
    "IGIPlanning:", "Are you sure you want to quit ?",
    QMessageBox::Yes | QMessageBox::No );

  if( value == QMessageBox::Yes )
  {
    this->close();
    m_GUIQuit = true;
  }
}

bool IGIPlanningGUI::HasQuitted() 
{
  return m_GUIQuit;
}

/** -----------------------------------------------------------------
*     Destructor
*  -----------------------------------------------------------------
*/
IGIPlanningGUI::~IGIPlanningGUI()
{
  delete m_GUI.Display3D;
  delete m_GUI.DisplayAxial;
  delete m_GUI.DisplayCoronal;
  delete m_GUI.DisplaySagittal;
}


/** -----------------------------------------------------------------
*  Construct an error observer for all the possible errors that occur in 
*   the observed IGSTK components.
*---------------------------------------------------------------------
*/
IGIPlanningGUI::ErrorObserver::ErrorObserver() : m_ErrorOccured( false )
{
}

/** -----------------------------------------------------------------
*   When an error occurs in an IGSTK component it will invoke this method 
*   with the appropriate error event object as a parameter.
*---------------------------------------------------------------------
*/
void IGIPlanningGUI::ErrorObserver::Execute( const itk::Object * itkNotUsed(caller), 
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
void IGIPlanningGUI::ErrorObserver::Execute( itk::Object *caller, 
  const itk::EventObject & event ) throw (std::exception)
{
  const itk::Object * constCaller = caller;
  this->Execute(constCaller, event);
}

