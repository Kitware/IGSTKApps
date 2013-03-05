#ifndef __IGITrackingDataGUI_h
#define __IGITrackingDataGUI_h

/** ITK */
#include "itkCommand.h"

/** Qt */
#include <QtGui>
#include "ui_IGITrackingDataGUI.h"

/** IGSTK */
#include "igstkObject.h"
#include "igstkAxesObject.h"
#include "igstkAxesObjectRepresentation.h"
#include "igstkTransform.h"
#include "igstkArucoTracker.h"
#include "igstkArucoTrackerTool.h"
#include "igstkCoordinateSystemTransformToResult.h"
#include "igstkPrecomputedTransformData.h"
#include "igstkCTImageReader.h"

#include "ErrorManager.h"

/** \class IGITrackingDataGUI
*
* \brief Implementation class for IGITrackingDataGUI.
*
* This class implements the main application.
*
*/
class IGITrackingDataGUI : public QMainWindow, ErrorManager
{
  Q_OBJECT

    /** typedef for CT image reader */
  typedef igstk::CTImageReader                        CTImageReaderType;

  /** typedef for CT image spatial objects */
  typedef CTImageReaderType::ImageSpatialObjectType   CTImageSpatialObjectType;
  typedef CTImageSpatialObjectType::PointType         PointType;

  /** Typedef world coordinate system */
  typedef igstk::AxesObject                           AxesObjectType;
  typedef igstk::AxesObjectRepresentation             AxesObjectRepresentationType;

  /** Observer type for loaded event,
   *  the callback can be set to a member function. */
  typedef itk::ReceptorMemberCommand < IGITrackingDataGUI >         LoadedObserverType;

public:
  IGITrackingDataGUI();
  virtual ~IGITrackingDataGUI();
  bool HasQuit();

public slots:
  void PulseTimerEvent();
  void OnQuitAction();
  void StartInitialization();
  void showHelp();
  void CheckMarkerPosition();
  void TogglePointAquisition();
  void PointAquisition();
  void SelectCameraCalibrationFile();

protected:
  void keyPressEvent(QKeyEvent * event );

private:
  bool                                                m_GUIQuit;
  Ui::MainWindow                                      m_GUI;
  AxesObjectType::Pointer                             m_WorldReference;
  AxesObjectRepresentationType::Pointer               m_WorldReferenceRepresentation;
  bool                                                m_PointAquisitionStarted;
  QTimer*                                             m_PointAquisitionPulseTimer;
  QString                                             m_PointSetFilename;

  QString                                             m_CurrentPath;
  QString                                             m_ConfigDir;
  unsigned int                                        m_PositionIterator;
  
  std::string                                         m_CameraParametersFile;

  igstk::ArucoTracker::Pointer                          m_ArucoTracker;
  QMap<std::string,igstk::ArucoTrackerTool::Pointer>    m_ArucoTrackerToolMap;
  std::vector<std::string> m_MarkerIDs;
  QMap<std::string,std::ofstream*>                      m_Logfiles;
  igstk::Transform                                      m_ToolCalibrationTransform;

  bool                                                  m_ReferenceNotAvailable;
  bool                                                  m_ToolNotAvailable;

  //observer for write failure event
  typedef itk::MemberCommand<IGITrackingDataGUI>          WriteFailureObserverType;

  igstkObserverMacro( CoordinateSystemTransform,
                      igstk::CoordinateSystemTransformToEvent,
                      igstk::CoordinateSystemTransformToResult );

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

  bool InitializeTrackerAction();

  /** Internal functions */
  void CreateActions();
  void ConnectImageRepresentation();
  bool LoadToolCalibrationTransform(QString transformFile);
  void SetMarkerLED( std::string markerID, bool state);

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
