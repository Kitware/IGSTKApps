/*=========================================================================

  Program:   IGI Remote Control
  Module:  $RCSfile: RemoteControl.cxx,v $
  Language:  C++
  Date:    $Date: 2012-10-22 19:52:31 $
  Version:   $Revision: 1.0 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "RemoteControl.moc"
#include "RemoteControl.h"

#include "../IGIConfigurationData.h"

RemoteControl::RemoteControl()
{ 
  m_GUI.setupUi(this);
  this->CreateActions();

  m_GUIQuit  = false;
  
  // get current application path
  QString path = QApplication::applicationDirPath();
  QDir currentDir = QDir(path);
  m_CurrentPath = currentDir.absolutePath();
  
  currentDir.cdUp();
  m_ConfigDir = currentDir.absolutePath() + "/" + IGIConfigurationData::CONFIGURATION_FOLDER;  

  QTimer *pulseTimer = new QTimer();
  connect(pulseTimer, SIGNAL(timeout()), this, SLOT(PulseTimerEvent()));
  pulseTimer->start(10);
}

void RemoteControl::PulseTimerEvent()
{
  igstk::PulseGenerator::CheckTimeouts();
}

void RemoteControl::CreateActions()
{
  connect(m_GUI.QuitPushButton, SIGNAL(clicked()), this, SLOT(OnQuitAction()));
  connect(m_GUI.helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
  connect(m_GUI.CheckPositionButton, SIGNAL(clicked()), this, SLOT(CheckMarkerPosition()));
}

/**
 * Configure the ArucoTracker. Attach three tools corresponding to the forward
 * keypress, backward keypress, and exit actions. The former key events are sent to the 
 * window that has focus. The exit action will terminate this program after
 * shutting the tracker down.
 */
igstk::Tracker::Pointer RemoteControl::Setup()
{                   //camera calibration file should be in the same directory
	                //in which the program is run
  const std::string CAMERA_CALIBRATION_FILENAME = 
                           m_ConfigDir.toStdString()
	                         + "/"
	                         + IGIConfigurationData::CAMERA_CALIBRATION_FILENAME;

                  //marker size in mm
  const unsigned int MARKER_SIZE = 50;
                 //requested tracking refresh rate in Hz
  const double REFRESH_RATE = 5;

  const int FORWARD_MARKER_NAME = 202;
  const int BACKWARD_MARKER_NAME = 502;
  const int EXIT_MARKER_NAME = 201;

  igstk::Tracker::Pointer tracker;
  igstk::ArucoTrackerTool::Pointer forwardTool, backwardTool, terminateTool;

  m_ArucoTracker = igstk::ArucoTracker::New();
  m_ArucoTracker->SetCameraParametersFromYAMLFile( CAMERA_CALIBRATION_FILENAME );
  m_ArucoTracker->SetMarkerSize( MARKER_SIZE );
  m_ArucoTracker->RequestSetFrequency( REFRESH_RATE );
             //need to observe that request open worked
  TrackerOpenEventOccurredObserver::Pointer too = TrackerOpenEventOccurredObserver::New();
  m_ArucoTracker->AddObserver( igstk::TrackerOpenEvent(), too );
  m_ArucoTracker->RequestOpen();

                    //if the tracker open succeeded we add the tools and attempt to
                    //start tracking
  if( too->EventOccured() ) {
	           //setup tool for sending the page-down/forward event
    forwardTool = igstk::ArucoTrackerTool::New();
    forwardTool->RequestSetMarkerName( FORWARD_MARKER_NAME );
    forwardTool->RequestConfigure();
    forwardTool->RequestAttachToTracker( m_ArucoTracker );
  
    ToolUpdatedGenerateKeyEventObserver::Pointer forwardObserver = 
		ToolUpdatedGenerateKeyEventObserver::New();
    forwardObserver->Initialize( forwardTool.GetPointer(), VK_NEXT );
    forwardTool->AddObserver( igstk::TrackerToolTransformUpdateEvent(), forwardObserver );

	          //setup tool for sending the page-up/backward event
    backwardTool = igstk::ArucoTrackerTool::New();
    backwardTool->RequestSetMarkerName( BACKWARD_MARKER_NAME );
    backwardTool->RequestConfigure();
    backwardTool->RequestAttachToTracker( m_ArucoTracker );
  
    ToolUpdatedGenerateKeyEventObserver::Pointer backwardObserver = 
		ToolUpdatedGenerateKeyEventObserver::New();
    backwardObserver->Initialize( backwardTool.GetPointer(), VK_PRIOR );
    backwardTool->AddObserver( igstk::TrackerToolTransformUpdateEvent(), backwardObserver );
	
	          //setup tool for terminating the program
    terminateTool = igstk::ArucoTrackerTool::New();
    terminateTool->RequestSetMarkerName( EXIT_MARKER_NAME );
    terminateTool->RequestConfigure();
    terminateTool->RequestAttachToTracker( m_ArucoTracker );
  
    ToolUpdatedTerminateProgramObserver::Pointer terminateObserver = 
		ToolUpdatedTerminateProgramObserver::New();
	  terminateObserver->Initialize( terminateTool.GetPointer(), m_ArucoTracker.GetPointer() );
    terminateTool->AddObserver( igstk::TrackerToolTransformUpdateEvent(), terminateObserver );

            //attempt to start tracking
    StartTrackingEventOccurredObserver::Pointer sto = StartTrackingEventOccurredObserver::New();
    m_ArucoTracker->AddObserver( igstk::TrackerStartTrackingEvent(), sto );
    m_ArucoTracker->RequestStartTracking();
    if( sto->EventOccured() )
      tracker = m_ArucoTracker;
    else { 
      QMessageBox::information(this, "Remote Control", "Failed to open tracker [camera not connected, or calibration file not found].\n");
      return tracker;
    }
  }
  else {
    QMessageBox::information(this, "Remote Control", "Failed to open tracker [camera not connected, or calibration file not found].\n");
    return tracker;
  }
  return tracker;
}

void RemoteControl::CheckMarkerPosition()
{
  cv::Mat currentImage = m_ArucoTracker->GetCurrentVideoFrame();
  cv::imshow("video",currentImage);
  cv::waitKey(2000);
  cv::destroyWindow("video");
}

void RemoteControl::OnQuitAction()
{
  this->close();
  m_GUIQuit = true;
}

bool RemoteControl::HasQuit( )
{
  return m_GUIQuit;
}

void RemoteControl::showHelp()
{
  QDialog* helpBox = new QDialog(this);
  helpBox->setWindowTitle("Help");

  QTextBrowser* browser = new QTextBrowser(helpBox); 
  browser->setSource(QUrl::fromLocalFile(m_CurrentPath + "/READMERemoteControl.html"));
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
