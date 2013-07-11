/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: PivotCalibration.h,v $
  Language:  C++
  Date:      $Date: 2009-03-30 18:26:16 $
  Version:   $Revision: 1.8 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/** \class PivotCalibration
 *
 *  \brief This class is a user interface (Qt-based) for the pivot calibration
 *         class.
 *
 *  This class provides a UI for performing pivot calibration (tool tip
 *  calibration). The class is responsible for acquisition of tracking data and
 *  computation of the pivot calibration. 
 */

#ifndef __PivotCalibration_h
#define __PivotCalibration_h

#include <QtGui>

#include "itkCommand.h"
#include "igstkArucoTracker.h"
#include "igstkArucoTrackerTool.h"
#include "igstkPivotCalibration.h"

#include "ui_IGIPivotCalibration.h"

class PivotCalibration : public QMainWindow
{
  Q_OBJECT

public:
  PivotCalibration();

  bool HasQuit();

public slots:
  void PulseTimerEvent();
  void OnQuitAction();
  void OnInitializationAction( );
  void OnCalibrationAction( );
  void setTransformationsValue(int  value);
  void showHelp();
  void CheckMarkerPosition();

private:
  void CreateActions();

  Ui::MainWindow              m_GUI;

private:
  //pivot calibration success/failure observer
  typedef itk::MemberCommand<PivotCalibration>
    CalibrationSuccessFailureObserverType;
  void OnCalibrationSuccessFailureEvent( itk::Object *caller, 
                              const itk::EventObject & event );
  CalibrationSuccessFailureObserverType::Pointer m_CalibrationSuccessFailureObserver;  

  //observer for write failure event
  typedef itk::MemberCommand<PivotCalibration>
                            WriteFailureObserverType;
  void OnWriteFailureEvent( itk::Object *caller, 
                              const itk::EventObject & event );

  /* Observe all tracking related errors, those generated by the tracker and
     the serial communication*/
  class TrackingErrorObserver : public itk::Command
    {
    public:
      typedef TrackingErrorObserver            Self;
      typedef ::itk::Command                   Superclass;
      typedef ::itk::SmartPointer<Self>        Pointer;
      typedef ::itk::SmartPointer<const Self>  ConstPointer;

      igstkNewMacro(Self)
      igstkTypeMacro(IGSTKErrorObserver, itk::Command)

      /*
       * When an error occurs in the tracker or serial communicaiton component 
       * it will invoke this method with the appropriate error event object as 
       * a parameter. */
      void Execute(itk::Object *caller, const itk::EventObject & event);

      /*
       * When an error occurs in the tracker or serial communicaiton component 
       * it will invoke this method with the appropriate error event object as 
       * a parameter. */
      void Execute(const itk::Object *caller, const itk::EventObject & event);

      /**
       * Clear the current error. */
      void ClearError();

      /**
      * If an error occured in the tracker or serial communication this method
      * will return true. */
      bool Error();

    protected:

      /**
       * Construct an error observer for all the possible errors that occur in
       * the tracker and serial communication classes. */
      TrackingErrorObserver();

      virtual ~TrackingErrorObserver(){}

    private:

      /**
       *  member variables
       */
      bool                                m_ErrorOccured;
      std::map<std::string,std::string>   m_ErrorEvent2ErrorMessage;

      //purposely not implemented
      TrackingErrorObserver(const Self&);
      void operator=(const Self&);
    };

  igstk::ArucoTracker::Pointer m_Tracker;
  igstk::ArucoTrackerTool::Pointer m_tool;
  TrackingErrorObserver::Pointer m_errorObserver;
  bool            m_initialized;
  bool            m_GUIQuit;

  int nrOfTransformations;

  igstkTypeMacro( PivotCalibration, Fl_Group );

    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()


    /** This method sets the number of transformations required for performing
     *  the pivot calibration, and the tool information.
     *  It is assumed that the tracker is already in tracking mode.
     *  If the initialization fails a message box will be displayed.
     *  The method generates two events: InitializationSuccessEvent and
     *  InitializationFailureEvent. */
    void RequestInitialize( unsigned int n,
      igstk::TrackerTool::Pointer trackerTool );

    /** This method sets the delay in seconds between the moment the
     *  "Calibrate" button is pressed and the beginning of data acquisition. */
    void RequestSetDelay( unsigned int delayInSeconds );

    /** This method is used to request the calibration transformation.
     *  The method should only be invoked after a successful calibration.
     *  It generates two events: CoordinateSystemTransformToEvent, and
     *  TransformNotAvailableEvent, respectively denoting that a calibration
     *  transform is and isn't available. */
    void RequestCalibrationTransform();

    /** This method is used to request the pivot point, given in the coordinate
     *  system in which the user supplied transforms were given. It generates two
     *  events: PointEvent, and InvalidRequestErrorEvent, respectively denoting
     *  that the pivot point is and isn't available. */
    void RequestPivotPoint();

    /** This method is used to request the Root Mean Square Error (RMSE) of the
     *  overdetermined equation system used to perform pivot calibration. It
     *  generates two events: DoubleTypeEvent, and InvalidRequestErrorEvent,
     *  respectively denoting that the RMSE is and isn't available.
     *  \sa PivotCalibrationAlgorithm */
    void RequestCalibrationRMSE();

  private:
    inline void RequestComputeCalibration();
    //the igstk class that actually does everything
    igstk::PivotCalibration::Pointer m_pivotCalibration;
    //delay before data acquisition starts [milliseconds]
    unsigned int m_delay;
    //description of the tool we want to calibrate
    std::string m_currentToolInformation;
    QString m_CurrentPath;
    QString m_ConfigDir;

    //accumulate the calibration information in this stream and
    //then display to the user
    std::ostringstream m_calibrationInformationStream;

    //pivot calibration initialization observer
    typedef itk::MemberCommand<PivotCalibration>
      InitializationObserverType;
    void OnInitializationEvent( itk::Object *caller,
                                const itk::EventObject & event );
    InitializationObserverType::Pointer m_InitializationObserver;

    //pivot calibration events (start, progress, end)
    //observer
    typedef itk::MemberCommand<PivotCalibration>
      CalibrationObserverType;
    void OnCalibrationEvent( itk::Object *caller,
                             const itk::EventObject & event );
    CalibrationObserverType::Pointer m_CalibrationObserver;

    //pivot calibration get transform observer
    igstkObserverMacro( TransformTo,
                        igstk::CoordinateSystemTransformToEvent,
                        igstk::CoordinateSystemTransformToResult )
    TransformToObserver::Pointer m_TransformToObserver;

    //pivot calibration get pivot point observer
    igstkObserverMacro( PivotPoint,
                        igstk::PointEvent,
                        igstk::EventHelperType::PointType )
    PivotPointObserver::Pointer m_PivotPointObserver;

    //pivot calibration get RMSE observer
    igstkObserverMacro( RMSE,
                        igstk::DoubleTypeEvent,
                        igstk::EventHelperType::DoubleType )
    RMSEObserver::Pointer m_RMSEObserver;

};

#endif //_PIVOT_CALIBRATION_QT_WIDGET__H_
