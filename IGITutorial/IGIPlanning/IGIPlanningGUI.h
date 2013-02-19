#ifndef __IGIPlanningGUI_h
#define __IGIPlanningGUI_h

/** ITK */
#include "itkCommand.h"

/** Qt */
#include <QtGui>
#include <QtXml/qdom.h>
#include "ui_IGIPlanningGUI.h"

/** IGSTK */
#include "igstkObject.h"
#include "igstkView.h"
#include "igstkView2D.h"
#include "igstkView3D.h"
#include "igstkCTImageReader.h"
#include "igstkCTImageSpatialObjectRepresentation.h"
#include "igstkImageResliceObjectRepresentation.h"
#include "igstkCrossHairSpatialObject.h"
#include "igstkCrossHairObjectRepresentation.h"
#include "igstkAxesObject.h"
#include "igstkAxesObjectRepresentation.h"
#include "igstkTransform.h"
#include "igstkMeshReader.h"
#include "igstkMeshObjectRepresentation.h"
#include "igstkCoordinateSystemTransformToResult.h"
#include "igstkAnnotation2D.h"

#include <map>
#include "ErrorManager.h"
#include "XMLSettings.h"
#include "FiducialSet.h"
#include "ctkRangeSlider.h"

/** \class IGIPlanningGUI
*
* \brief Implementation class for IGIPlanningGUI.
*
* This class implements the main application.
*
*/
class IGIPlanningGUI : public QMainWindow, ErrorManager
{
  Q_OBJECT

  /** typedef for View */
  typedef igstk::View         ViewType;
  typedef igstk::View2D       View2DType;
  typedef igstk::View3D       View3DType;
   
  /** typedef for CT image reader */
  typedef igstk::CTImageReader                        CTImageReaderType;

  /** typedef for CT image spatial objects */
  typedef CTImageReaderType::ImageSpatialObjectType   CTImageSpatialObjectType;

  typedef itk::MemberCommand<IGIPlanningGUI>          ProgressCommandType;

  /** Image reslice representation */
  typedef igstk::ImageResliceObjectRepresentation< CTImageSpatialObjectType >
                                                      CTImageRepresentationType;

  /** Cross hair spatial object and representation */
  typedef igstk::CrossHairSpatialObject               CrossHairType;
  typedef igstk::CrossHairObjectRepresentation        CrossHairRepresentationType;

  /** Reslicer plane spatial object */
  typedef igstk::ReslicerPlaneSpatialObject           ReslicerPlaneType;

  /** Typedef world coordinate system */
  typedef igstk::AxesObject                           AxesObjectType;
  typedef igstk::AxesObjectRepresentation             AxesObjectRepresentationType;

  /** typedef for an image voxel index and world coordinates */
  typedef CTImageSpatialObjectType::IndexType         IndexType;
  typedef CTImageSpatialObjectType::PointType         PointType;

  /** typedef for mesh readers */
  typedef igstk::MeshReader                           MeshReaderType;

  /** typedef for mesh spatial objects */
  typedef MeshReaderType::MeshObjectType              MeshType;

  /** tool representation */
  typedef igstk::MeshObjectRepresentation             MeshRepresentationType;

  /** Observer type for loaded event,
   *  the callback can be set to a member function. */
  typedef itk::ReceptorMemberCommand < IGIPlanningGUI >         LoadedObserverType;

  typedef std::map<std::string, QCheckBox*>				   ButtonContainerType;
  typedef std::map<std::string, QRadioButton*>             RadioButtonContainerType;

public:
  IGIPlanningGUI();
  virtual ~IGIPlanningGUI();
  void SetupView();
  bool HasQuit();

public slots:
  void PulseTimerEvent();
  void OnQuitAction();
  void LoadImageAction();
  void ResliceImageCallback1(int value);
  void ResliceImageCallback2(int value);
  void ResliceImageCallback3(int value);
  void ResetCamera();
  void SelectFiducial(QString buttonName);
  void LoadFiducials();
  void LoadToolSpatialObjectAction();
  void AddNewFiducial();
  void SelectPointerToolCalibrationFile();
  void SelectCameraCalibrationFile();
  void SaveConfiguration();
  void SaveFiducials();
  void NextTab();
  void PreviousTab();
  void showHelp();

  void reference102RadioChecked(bool);
  void reference403RadioChecked(bool);
  void reference502RadioChecked(bool);
  void reference201RadioChecked(bool);
  void reference202RadioChecked(bool);

  void pointer102RadioChecked(bool);
  void pointer403RadioChecked(bool);
  void pointer502RadioChecked(bool);
  void pointer201RadioChecked(bool);
  void pointer202RadioChecked(bool);
  void OnCheckBoxToggled(QWidget * checkBox);
  void OffButtonClicked(int);
  void NewFiducialSet();
  void RemoveFiducial();
  void SetWindowLevelThroughRange(int min, int max);

protected:
  void keyPressEvent(QKeyEvent * event );

private:
  bool                                                m_GUIQuit;
  Ui::MainWindow                                      m_GUI;
  AxesObjectType::Pointer                             m_WorldReference;
  AxesObjectRepresentationType::Pointer               m_WorldReferenceRepresentation;
  igstk::Annotation2D::Pointer                        m_PointCoordsAnnotation;
  ctkRangeSlider*                                     m_WindowLevelSlider;
  int                                                 m_Window;
  int                                                 m_Level;
  
  QString                                             m_CurrentPath;
  QString                                             m_ConfigDir;
  QString                                             m_DatatDir;
  QString                                             m_TutorialDir;
  
  /** Point set definition */
  FiducialSet*                                        m_FiducialSet;
  QString                                             m_FiducialSetFilename;
  QString                                             m_TargetSetFilename;
  bool                                                m_CurrentPointSetSaved;

  /** Load and visualize image part */
  std::string                                         m_ImageDir;
  bool                                                m_ImageLoaded;
  View2DType::Pointer                                 m_ViewAxial;
  View2DType::Pointer                                 m_ViewSagittal;
  View2DType::Pointer                                 m_ViewCoronal;
  View3DType::Pointer                                 m_View3D;

  CTImageRepresentationType::Pointer                  m_AxialPlaneRepresentation;
  CTImageRepresentationType::Pointer                  m_SagittalPlaneRepresentation;
  CTImageRepresentationType::Pointer                  m_CoronalPlaneRepresentation;

  CTImageRepresentationType::Pointer                  m_AxialPlaneRepresentation2;
  CTImageRepresentationType::Pointer                  m_SagittalPlaneRepresentation2;
  CTImageRepresentationType::Pointer                  m_CoronalPlaneRepresentation2;
  
  ReslicerPlaneType::Pointer                          m_AxialPlaneSpatialObject;
  ReslicerPlaneType::Pointer                          m_SagittalPlaneSpatialObject;
  ReslicerPlaneType::Pointer                          m_CoronalPlaneSpatialObject;

  /** cross hair spatial object */
  CrossHairType::Pointer                                m_CrossHair;

  /** Configuration */
  QSettings*                                            m_Settings;
  
  std::string                                           m_PointerToolCalibrationFile;
  std::string                                           m_CameraCalibrationFile;

  QString                                               m_MeshFileName;
  std::string                                           m_CameraParametersFile;

  /** tool spatial object and representation */
  MeshType::Pointer                                     m_ToolSpatialObject;
  MeshRepresentationType::Pointer                       m_ToolRepresentation;

  double                                                m_TrackerRMS;
  igstk::Transform                                      m_RegistrationTransform;
  igstk::Transform                                      m_PickingTransform;

  bool                                                  m_ReferenceNotAvailable;
  bool                                                  m_ToolNotAvailable;

  
  /** Marker settings */
  std::string                                           m_ReferenceId;
  std::string                                           m_PointerId;
  ButtonContainerType                                   m_MarkerCheckBoxes;
  RadioButtonContainerType                              m_ReferenceRadioButtons;
  RadioButtonContainerType                              m_PointerRadioButtons;
  
  igstkObserverObjectMacro( CTImage, 
                            igstk::CTImageReader::ImageModifiedEvent,
                            igstk::CTImageSpatialObject );
  
  /** DICOM image reader error observer */
  igstkObserverMacro( DICOMImageReaderInvalidDirectoryNameError,
                      igstk::DICOMImageDirectoryIsNotDirectoryErrorEvent,
                      std::string )

  igstkObserverMacro( DICOMImageReaderNonExistingDirectoryError,
                      igstk::DICOMImageDirectoryDoesNotExistErrorEvent,
                      igstk::EventHelperType::StringType )

  igstkObserverMacro( DICOMImageReaderEmptyDirectoryError,
                      igstk::DICOMImageDirectoryEmptyErrorEvent,
                      igstk::EventHelperType::StringType )

  igstkObserverMacro( DICOMImageDirectoryNameDoesNotHaveEnoughFilesError,
                      igstk::DICOMImageDirectoryDoesNotHaveEnoughFilesErrorEvent,
                      igstk::EventHelperType::StringType )

  igstkObserverMacro( DICOMImageDirectoryDoesNotContainValidDICOMSeriesError,
                      igstk::DICOMImageSeriesFileNamesGeneratingErrorEvent,
                      igstk::EventHelperType::StringType )

  igstkObserverMacro( DICOMImageInvalidError,
                      igstk::DICOMImageReadingErrorEvent,
                      igstk::EventHelperType::StringType )

  igstkObserverMacro( ImageExtent, igstk::ImageExtentEvent,
                      igstk::EventHelperType::ImageExtentType );

  igstkObserverObjectMacro( MeshObject, igstk::MeshReader::MeshModifiedEvent,
                                        igstk::MeshObject );

  igstkObserverMacro( Registration, igstk::CoordinateSystemTransformToEvent,
                                    igstk::CoordinateSystemTransformToResult );

  igstkObserverMacro( RegistrationError, igstk::DoubleTypeEvent, double );

  igstkObserverMacro( CoordinateSystemTransform,
                      igstk::CoordinateSystemTransformToEvent,
                      igstk::CoordinateSystemTransformToResult );

  LoadedObserverType::Pointer                           m_TrackerToolNotAvailableObserver;
  LoadedObserverType::Pointer                           m_TrackerToolAvailableObserver;
  LoadedObserverType::Pointer                           m_ReferenceNotAvailableObserver;
  LoadedObserverType::Pointer                           m_ReferenceAvailableObserver;
  LoadedObserverType::Pointer                           m_AxialViewPickerObserver;
  LoadedObserverType::Pointer                           m_SagittalViewPickerObserver;
  LoadedObserverType::Pointer                           m_CoronalViewPickerObserver;

  void AxialViewPickingCallback( const itk::EventObject & event );
  void SagittalViewPickingCallback( const itk::EventObject & event );
  void CoronalViewPickingCallback( const itk::EventObject & event );

  void SetImagePickingProcessing();

  /** CT image spatial object */
  CTImageSpatialObjectType::Pointer                     m_CTImageSpatialObject;

  /** cross hair representation */
  CrossHairRepresentationType::Pointer                  m_AxialCrossHairRepresentation;
  CrossHairRepresentationType::Pointer                  m_SagittalCrossHairRepresentation;
  CrossHairRepresentationType::Pointer                  m_CoronalCrossHairRepresentation;
  CrossHairRepresentationType::Pointer                  m_3DViewCrossHairRepresentation;

  /** Internal functions */
  void CreateActions();
  void ConnectImageRepresentation();
  void ResliceImage( IndexType index );
  void OnITKProgressEvent(itk::Object *source, const itk::EventObject &);
  void AcceptingRegistration();
  QDomElement PointToNode( QDomDocument &d, PointType point, QString id );

  void setMarkerDialogState(std::string oldSelection, std::string newSelection, int select);

  /** -----------------------------------------------------------------
  *  Construct an error observer for all the possible errors that occur in 
  *   the observed IGSTK components.
  *---------------------------------------------------------------------
  */
  class ErrorObserver : public itk::Command
  {
  public:
    typedef ErrorObserver                    Self;
    typedef ::itk::Command                   Superclass;
    typedef ::itk::SmartPointer<Self>        Pointer;
    typedef ::itk::SmartPointer<const Self>  ConstPointer;

    igstkNewMacro(Self)
    igstkTypeMacro(ErrorObserver, itk::Command)

    /** When an error occurs in an IGSTK component it will invoke this method 
      *  with the appropriate error event object as a parameter.*/
    virtual void Execute(itk::Object *caller, 
                          const itk::EventObject & event) throw (std::exception);

    /** When an error occurs in an IGSTK component it will invoke this method 
      *  with the appropriate error event object as a parameter.*/
    virtual void Execute(const itk::Object *caller, 
                          const itk::EventObject & event) throw (std::exception);

    /**Clear the current error.*/
    void ClearError() { this->m_ErrorOccured = false; 
                        this->m_ErrorMessage.clear(); }
    /**If an error occurs in one of the observed IGSTK components this method 
      * will return true.*/
    bool ErrorOccured() { return this->m_ErrorOccured; }
    /**Get the error message associated with the last IGSTK error.*/
    void GetErrorMessage(std::string &errorMessage) 
      {
      errorMessage = this->m_ErrorMessage;
      }

  protected:
    ErrorObserver();
    virtual ~ErrorObserver(){}
  private:
    bool                               m_ErrorOccured;
    std::string                        m_ErrorMessage;
    std::map<std::string,std::string>  m_ErrorEvent2ErrorMessage;

    //purposely not implemented
    ErrorObserver(const Self&);
    void operator=(const Self&); 
  };
   
  ErrorObserver::Pointer                m_ErrorObserver;
  std::string                           m_ErrorMessage;
  
  /** Utility functions, conversion between points and transform */
  inline
  igstk::Transform
  PointToTransform( PointType point)
  {
    igstk::Transform transform;
    igstk::Transform::VectorType translation;
    for (int i=0; i<3; i++)
      {
      translation[i] = point[i];
      }
    transform.SetTranslation( translation, 0.1,
                                igstk::TimeStamp::GetLongestPossibleTime() );
    return transform;
  }

  inline
  PointType
  TransformToPoint( igstk::Transform transform)
  {
    PointType point;
    for (int i=0; i<3; i++)
      {
      point[i] = transform.GetTranslation()[i];
      }
    return point;
  }
};

#endif
