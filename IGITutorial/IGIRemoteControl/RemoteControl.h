/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: RemoteControl.h,v $
  Language:  C++
  Date:      $Date: 2012-10-23 16:26:16 $
  Version:   $Revision: 1.8 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/** \class RemoteControl
 *
 *  \brief Use of tracking system for HCI purposes, a power point presentation can be
 *        navigated by displaying different markers in front of a calibrated webcam.
 */

#ifndef __RemoteControl_h
#define __RemoteControl_h

#include <Windows.h>
#include <QtGui>

#include "igstkPulseGenerator.h"
#include "igstkArucoTracker.h"
#include "igstkTransformObserver.h"

#include "ui_RemoteControl.h"



#define igstkEventOccurredMacro( name, EventType ) \
class name##EventOccurredObserver : public ::itk::Command \
{ \
public: \
  typedef  name##EventOccurredObserver  Self; \
  typedef  ::itk::Command               Superclass;\
  typedef  ::itk::SmartPointer<Self>    Pointer;\
  itkNewMacro( Self );\
public:\
  void Execute(itk::Object *caller, const itk::EventObject & event)\
    {\
    const itk::Object * constCaller = caller;\
    this->Execute( constCaller, event );\
    }\
  void Execute(const itk::Object *caller, const itk::EventObject & event)\
    {\
    if( EventType().CheckEvent( &event ) )\
      {\
        m_EventOccurred = true;\
      }\
    }\
  bool EventOccured() const\
    {\
    return m_EventOccurred;\
    }\
  void Reset() \
    {\
    m_EventOccurred = false; \
    }\
protected:\
  name##EventOccurredObserver() \
    {\
    m_EventOccurred = false;\
    }\
  ~name##EventOccurredObserver() {}\
private:\
  bool m_EventOccurred;\
};


igstkEventOccurredMacro( TrackerOpen,  igstk::TrackerOpenEvent )
igstkEventOccurredMacro( StartTracking, igstk::TrackerStartTrackingEvent )

/**
 * Observer class for the ToolUpdated event which terminates the program, stops
 * the tracking, closes the connection to the tracker and exits.
 */
class ToolUpdatedTerminateProgramObserver  : public ::itk::Command
{
public:
  typedef  ToolUpdatedTerminateProgramObserver    Self;
  typedef  ::itk::Command                         Superclass;
  typedef  ::itk::SmartPointer<Self>              Pointer;
  itkNewMacro( Self );

protected:

  ToolUpdatedTerminateProgramObserver() 
  {
    this->m_TransformObserver = igstk::TransformObserver::New();    
  }

  ~ToolUpdatedTerminateProgramObserver() {}

public:
  void Initialize( igstk::TrackerTool::Pointer trackerTool, 
		           igstk::Tracker::Pointer tracker ) 
   {
     this->m_Tool = trackerTool;
     this->m_TransformObserver->ObserveTransformEventsFrom( this->m_Tool );
     this->m_LastValid = 0;
	 this->m_Tracker = tracker;
   }

  void Execute( itk::Object *caller, const itk::EventObject & event )
  {
    const itk::Object * constCaller = caller;
    this->Execute( constCaller, event );
  }
  
  void Execute( const itk::Object *caller, const itk::EventObject & event )
  {
    const double DURATION_VISIBLE = 2000.0; //time in milliseconds

                 //do something only for the correct tool
    if( this->m_Tool.GetPointer() == caller )
    {               //the tool transform has been updated, get it
      if( dynamic_cast<const
            igstk::TrackerToolTransformUpdateEvent  *>( &event) )
      {                 //request to get the transform
        this->m_Tool->RequestGetTransformToParent();
        if ( this->m_TransformObserver->GotTransform() )
        {
          igstk::Transform transform =
            this->m_TransformObserver->GetTransform();
          transform.GetStartTime();
          igstk::TransformBase::TimePeriodType curStart = 
            transform.GetStartTime();
          if( curStart - this->m_LastValid > DURATION_VISIBLE ) {
			this->m_Tracker->RequestStopTracking();
			this->m_Tracker->RequestClose();
			exit( EXIT_SUCCESS );
          }          
        }
      }
    }
  }

private:
  igstk::TransformObserver::Pointer m_TransformObserver; 
  igstk::TrackerTool::Pointer  m_Tool;
  igstk::TransformBase::TimePeriodType m_LastValid;
  igstk::Tracker::Pointer m_Tracker;
};

/**
 * Observer class for the ToolUpdated event which sends a specific key event
 * to the window that currently has focus.
 */
class ToolUpdatedGenerateKeyEventObserver : public ::itk::Command
{
public:
  typedef  ToolUpdatedGenerateKeyEventObserver    Self;
  typedef  ::itk::Command                         Superclass;
  typedef  ::itk::SmartPointer<Self>              Pointer;
  itkNewMacro( Self );

protected:

  ToolUpdatedGenerateKeyEventObserver() 
  {
    this->m_TransformObserver = igstk::TransformObserver::New();    
  }

  ~ToolUpdatedGenerateKeyEventObserver() {}

 public:
   void Initialize( igstk::TrackerTool::Pointer trackerTool, int vk ) 
   {
     this->m_Tool = trackerTool;
     this->m_TransformObserver->ObserveTransformEventsFrom( this->m_Tool );
     this->m_LastValid = 0;
	 this->m_vk = vk;
   }

   /*
   * The GenerateKey function was obtained here:
   * http://stackoverflow.com/questions/5202118/c-troubleshooting-sending-keystrokes
   *
   * Apparently this is not the original, and it is not clear who is the original 
   * author, so kudos go to the anonymous coder.
   *
   * Generate a key event which is sent to the window that currently has focus.
   * @param vk Virtual key code. These codes are defined in WinUser.h .
   */
  void GenerateKey( int vk, BOOL bExtended )
  {
    KEYBDINPUT  kb = {0};
    INPUT input = {0};

      //generate a "key down"
    if ( bExtended ) 
      kb.dwFlags  = KEYEVENTF_EXTENDEDKEY;
    kb.wVk  = vk;
    input.type  = INPUT_KEYBOARD;
    input.ki  = kb;
    SendInput( 1, &input, sizeof( INPUT ) );

      //generate a "key up"
    ZeroMemory( &kb, sizeof( KEYBDINPUT ) );
    ZeroMemory( &input, sizeof( INPUT ) );
    kb.dwFlags  =  KEYEVENTF_KEYUP;
    if ( bExtended ) 
      kb.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    kb.wVk = vk;
    input.type = INPUT_KEYBOARD;
    input.ki = kb;
    SendInput( 1, &input, sizeof( INPUT ) );

    return;
  }
  void Execute( itk::Object *caller, const itk::EventObject & event )
  {
    const itk::Object * constCaller = caller;
    this->Execute( constCaller, event );
  }
  
  void Execute( const itk::Object *caller, const itk::EventObject & event )
  {
    const double DURATION_VISIBLE = 1000.0; //time in milliseconds

                 //do something only for the correct tool
    if( this->m_Tool.GetPointer() == caller )
    {               //the tool transform has been updated, get it
      if( dynamic_cast<const
            igstk::TrackerToolTransformUpdateEvent  *>( &event) )
      {                 //request to get the transform
        this->m_Tool->RequestGetTransformToParent();
        if ( this->m_TransformObserver->GotTransform() )
        {
          igstk::Transform transform =
            this->m_TransformObserver->GetTransform();
          transform.GetStartTime();
          igstk::TransformBase::TimePeriodType curStart = 
            transform.GetStartTime();
          if( curStart - this->m_LastValid > DURATION_VISIBLE ) {
			GenerateKey( this->m_vk, false );
            this->m_LastValid = curStart;
          }          
        }
      }
    }
  }

private:
  igstk::TransformObserver::Pointer m_TransformObserver; 
  igstk::TrackerTool::Pointer  m_Tool;
  igstk::TransformBase::TimePeriodType m_LastValid;
  int m_vk;
};

class RemoteControl : public QMainWindow
{
  Q_OBJECT

public:
  RemoteControl();
  bool HasQuit();

  /**
   * This is where we setup the tracking and associate between tools and actions.
   * THIS IS THE HEART OF THE PROGRAM.
   */
  igstk::Tracker::Pointer Setup();

public slots:
  void PulseTimerEvent();
  void OnQuitAction();
  void showHelp();
  void CheckMarkerPosition();

private:
  void CreateActions();
  igstk::ArucoTracker::Pointer m_ArucoTracker;

private:
  Ui::MainWindow  m_GUI;
  bool            m_GUIQuit;
  QString         m_CurrentPath;
  QString         m_ConfigDir;
};

#endif
