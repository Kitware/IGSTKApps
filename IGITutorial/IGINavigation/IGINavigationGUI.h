#ifndef __IGINavigationGUI_h
#define __IGINavigationGUI_h

/** ITK */
#include "itkCommand.h"

/** Qt */
#include <QtGui>
#include "ui_IGINavigationGUI.h"
#include <QtXml/qdom.h>

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
#include "igstkLandmark3DRegistration.h"
#include "igstkEllipsoidObject.h"
#include "igstkEllipsoidObjectRepresentation.h"
#include "igstkTransform.h"
#include "igstkArucoTracker.h"
#include "igstkArucoTrackerTool.h"
#include "igstkMeshReader.h"
#include "igstkMeshObjectRepresentation.h"
#include "igstkCoordinateSystemTransformToResult.h"
#include "igstkAnnotation2D.h"
#include "igstkTransformFileReader.h"
#include "igstkTransformXMLFileReaderBase.h"
#include "igstkPrecomputedTransformData.h"
#include "igstkRigidTransformXMLFileReader.h"

#include "ErrorManager.h"
#include "XMLSettings.h"
#include "FiducialSet.h"

/** \class IGINavigationGUI
*
* \brief Implementation class for IGINavigationGUI.
*
* This class implements the main application.
*
*/
class IGINavigationGUI : public QMainWindow, ErrorManager
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

  typedef itk::MemberCommand<IGINavigationGUI>        ProgressCommandType;

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

 /** typedef for landmark registration types */
  typedef igstk::Landmark3DRegistration               RegistrationType;
  typedef RegistrationType::LandmarkPointContainerType
                                                      LandmarkPointContainerType;

  typedef std::map< unsigned int, bool >              AcceptedLandmarkPointContainerType;

  /** typedef for mesh readers */
  typedef igstk::MeshReader                           MeshReaderType;

  /** typedef for mesh spatial objects */
  typedef MeshReaderType::MeshObjectType              MeshType;

  /** tool representation */
  typedef igstk::MeshObjectRepresentation             MeshRepresentationType;

  /** Observer type for loaded event,
   *  the callback can be set to a member function. */
  typedef itk::ReceptorMemberCommand < IGINavigationGUI >         LoadedObserverType;

public:
  IGINavigationGUI();
  virtual ~IGINavigationGUI();
  void SetupView();
  bool HasQuit();


public slots:
  void PulseTimerEvent();
  void OnQuitAction();
  bool LoadImageAction();
  void ResliceImageCallback1(int value);
  void ResliceImageCallback2(int value);
  void ResliceImageCallback3(int value);
  void ResetCamera();
  void SelectFiducial(QString buttonName);
  void SelectTarget(QString buttonName);
  void LoadFiducials();
  void AcceptTrackerFiducialAction();
  bool LoadToolSpatialObjectAction(QString id, std::string meshFile);
  void CalculateRegistrationAction();
  void LoadConfiguration();
  void showHelp();
  void CheckMarkerPosition();

protected:
  void keyPressEvent(QKeyEvent * event );

private:
  bool                                                m_GUIQuit;
  Ui::MainWindow                                      m_GUI;
  AxesObjectType::Pointer                             m_WorldReference;
  AxesObjectRepresentationType::Pointer               m_WorldReferenceRepresentation;
  igstk::Annotation2D::Pointer                        m_PointCoordsAnnotation;

  QString                                             m_CurrentPath;
  QString                                             m_ConfigDir;
  QString                                             m_TutorialDir;

  QString                                             m_PointerToolCalibrationFile;
  QString                                             m_ReferenceId;
  QString                                             m_PointerId;
  QMap<QString,QString>                               m_MarkerIdMeshMap;
  int                                                 m_Window;
  int                                                 m_Level;
  
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
  CrossHairType::Pointer                              m_CrossHair;

  /** Configuration */
  QSettings*                                          m_Settings;
  
  FiducialSet*                                        m_FiducialSet;
  FiducialSet*                                        m_TargetSet;
  QString                                             m_FiducialSetFilename;
  QString                                             m_TargetSetFilename;
  
  std::string                                         m_MeshFileName;
  std::string                                         m_CameraParametersFile;

  QMap<QString,PointType>                               m_LandmarksContainer;
  QMap<QString,PointType>                               m_AcceptedLandmarksContainer;

  igstk::ArucoTracker::Pointer                          m_ArucoTracker;
  igstk::ArucoTrackerTool::Pointer                      m_ArucoTrackerTool;
  igstk::ArucoTrackerTool::Pointer                      m_ArucoReferenceTrackerTool;
  QMap<QString,igstk::ArucoTrackerTool::Pointer>        m_ArucoTrackerToolMap;
  igstk::Transform                                      m_ToolCalibrationTransform;

  int                                                   m_FiducialNr;

  /** tool spatial object and representation maps*/
  MeshType::Pointer                                     m_ToolSpatialObject;
  MeshRepresentationType::Pointer                       m_ToolRepresentation;
  QMap<QString,MeshType::Pointer>                       m_MeshMap;
  QMap<QString,MeshRepresentationType::Pointer>         m_MeshRepresentationMap;

  double                                                m_TrackerRMS;
  igstk::Transform                                      m_RegistrationTransform;
  igstk::Transform                                      m_PickingTransform;

  bool                                                  m_ReferenceNotAvailable;
  bool                                                  m_ToolNotAvailable;
  igstkObserverObjectMacro( CTImage,
                            igstk::CTImageReader::ImageModifiedEvent,
                            igstk::CTImageSpatialObject );
  void OnWriteFailureEvent( itk::Object *caller,
                              const itk::EventObject & event );

  //observer for write failure event
  typedef itk::MemberCommand<IGINavigationGUI>          WriteFailureObserverType;

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

  /**
   * This object observes the event generated when a RequestGetData() method of
   * the reader is invoked, gets the precomputed transform data object. 
   */
  igstkObserverMacro( TransformData,
                      igstk::TransformFileReader::TransformDataEvent,
                      igstk::PrecomputedTransformData::Pointer )

  igstkObserverMacro( TransformationDescription,
                      igstk::StringEvent,
                      std::string )

  igstkObserverMacro( TransformationDate,
                      igstk::PrecomputedTransformData::TransformDateTypeEvent,
                      std::string )
  typedef igstk::PrecomputedTransformData::TransformType *  TransformTypePointer;

  igstkObserverMacro( TransformRequest,
                      igstk::PrecomputedTransformData::TransformTypeEvent,
                      TransformTypePointer )

  igstkObserverMacro( TransformError,
                      igstk::PrecomputedTransformData::TransformErrorTypeEvent,
                      igstk::PrecomputedTransformData::ErrorType )

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
  void ToolNotAvailableCallback( const itk::EventObject & event );
  void ToolAvailableCallback( const itk::EventObject & event );
  void ReferenceNotAvailableCallback( const itk::EventObject & event );
  void ReferenceAvailableCallback( const itk::EventObject & event );
  bool InitializeTrackerAction();

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
  bool LoadToolCalibrationTransform(QString transformFile);
  void SceneObjectsVisibility(double opacity);
  void StoreTransformInXMLFormat(igstk::Transform transform);

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
