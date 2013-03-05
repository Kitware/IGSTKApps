#ifndef __IGI3DDigitizerGUI_h
#define __IGI3DDigitizerGUI_h

/** ITK */
#include "itkCommand.h"

/** Qt */
#include <QtGui>
#include "ui_IGI3DDigitizerGUI.h"

/** IGSTK */
#include "igstkObject.h"
#include "igstkView.h"
#include "igstkView3D.h"
#include "igstkAxesObject.h"
#include "igstkAxesObjectRepresentation.h"
#include "igstkEllipsoidObject.h"
#include "igstkEllipsoidObjectRepresentation.h"
#include "igstkTransform.h"
#include "igstkArucoTracker.h"
#include "igstkArucoTrackerTool.h"
#include "igstkMeshReader.h"
#include "igstkMeshObjectRepresentation.h"
#include "igstkCoordinateSystemTransformToResult.h"
#include "igstkTransformFileReader.h"
#include "igstkTransformXMLFileReaderBase.h"
#include "igstkPrecomputedTransformData.h"
#include "igstkRigidTransformXMLFileReader.h"

#include "ErrorManager.h"
#include "FiducialSet.h"

/** \class IGI3DDigitizerGUI
*
* \brief Implementation class for IGI3DDigitizerGUI.
*
* This class implements the main application.
*
*/
class IGI3DDigitizerGUI : public QMainWindow, ErrorManager
{
  Q_OBJECT

  /** typedef for View */
  typedef igstk::View         ViewType;
  typedef igstk::View3D       View3DType;

  /** typedef for CT image reader */
  typedef igstk::CTImageReader                        CTImageReaderType;

  /** typedef for CT image spatial objects */
  typedef CTImageReaderType::ImageSpatialObjectType   CTImageSpatialObjectType;
  typedef CTImageSpatialObjectType::PointType         PointType;

  /** Typedef world coordinate system */
  typedef igstk::AxesObject                           AxesObjectType;
  typedef igstk::AxesObjectRepresentation             AxesObjectRepresentationType;

  /** typedef for mesh readers */
  typedef igstk::MeshReader                           MeshReaderType;

  /** typedef for mesh spatial objects */
  typedef MeshReaderType::MeshObjectType              MeshType;

  /** tool representation */
  typedef igstk::MeshObjectRepresentation             MeshRepresentationType;

  /** Observer type for loaded event,
   *  the callback can be set to a member function. */
  typedef itk::ReceptorMemberCommand < IGI3DDigitizerGUI >         LoadedObserverType;

public:
  IGI3DDigitizerGUI();
  virtual ~IGI3DDigitizerGUI();
  void SetupView();
  bool HasQuit();

public slots:
  void PulseTimerEvent();
  void OnQuitAction();
  void ResetCamera();
  void StartInitialization();
  void showHelp();
  void CheckMarkerPosition();
  void TogglePointAquisition();
  void ClearScene();
  void PointAquisition();
  void SavePoints();
  void LoadPoints();
  void HandlePointerIDChanged(int ID);
  void HandleRefIDChanged(int ID);
  void SelectCameraCalibrationFile();
  void SelectPointerToolCalibrationFile();

protected:
  void keyPressEvent(QKeyEvent * event );

private:
  bool                                                m_GUIQuit;
  Ui::MainWindow                                      m_GUI;
  AxesObjectType::Pointer                             m_WorldReference;
  AxesObjectRepresentationType::Pointer               m_WorldReferenceRepresentation;
  igstk::Annotation2D::Pointer                        m_PointCoordsAnnotation;
  bool                                                m_PointAquisitionStarted;
  QTimer*                                             m_PointAquisitionPulseTimer;
  QString                                             m_PointSetFilename;
  bool                                                m_CurrentPointSetSaved;
  
  View3DType::Pointer                                 m_View3D;

  QString                                             m_CurrentPath;
  QString                                             m_ConfigDir;

  QString                                             m_PointerToolCalibrationFile;
  QString                                             m_ReferenceId;
  QString                                             m_PointerId;
  QMap<QString,QString>                               m_MarkerIdMeshMap;
 
  FiducialSet*                                        m_FiducialSet;
  QString                                             m_FiducialSetFilename;
  
  std::string                                         m_CameraParametersFile;

  igstk::ArucoTracker::Pointer                          m_ArucoTracker;
  igstk::ArucoTrackerTool::Pointer                      m_ArucoTrackerTool;
  igstk::ArucoTrackerTool::Pointer                      m_ArucoReferenceTrackerTool;
  QMap<QString,igstk::ArucoTrackerTool::Pointer>        m_ArucoTrackerToolMap;
  igstk::Transform                                      m_ToolCalibrationTransform;

  /** tool spatial object and representation maps*/
  MeshType::Pointer                                     m_ToolSpatialObject;
  MeshRepresentationType::Pointer                       m_ToolRepresentation;

  bool                                                  m_ReferenceNotAvailable;
  bool                                                  m_ToolNotAvailable;

  //observer for write failure event
  typedef itk::MemberCommand<IGI3DDigitizerGUI>          WriteFailureObserverType;

  igstkObserverObjectMacro( MeshObject, igstk::MeshReader::MeshModifiedEvent,
                                        igstk::MeshObject );

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

  void ToolNotAvailableCallback( const itk::EventObject & event );
  void ToolAvailableCallback( const itk::EventObject & event );
  void ReferenceNotAvailableCallback( const itk::EventObject & event );
  void ReferenceAvailableCallback( const itk::EventObject & event );
  bool InitializeTrackerAction();

  /** Internal functions */
  void CreateActions();
  void ConnectImageRepresentation();
  bool LoadToolCalibrationTransform(QString transformFile);
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
