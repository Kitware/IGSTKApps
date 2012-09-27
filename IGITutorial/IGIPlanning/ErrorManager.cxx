#include "ErrorManager.h"

#include "itksys/SystemTools.hxx"
#include "itksys/Directory.hxx"

#include <QDialog>
#include <QErrorMessage>
#include <QMessageBox>

ErrorManager::ErrorManager()
{  
  /** Setup logger, for all igstk components. */
  m_Logger   = LoggerType::New();
  this->GetLogger()->SetTimeStampFormat( itk::LoggerBase::HUMANREADABLE );
  this->GetLogger()->SetHumanReadableFormat("%Y %b %d, %H:%M:%S");
  this->GetLogger()->SetPriorityLevel( LoggerType::DEBUG);

  /** Direct the application log message to the std::cout */
  //itk::StdStreamLogOutput::Pointer m_LogCoutOutput
  //                                         = itk::StdStreamLogOutput::New();
  //m_LogCoutOutput->SetStream( std::cout );
  //this->GetLogger()->AddLogOutput( m_LogCoutOutput );

  /** Direct the igstk components log message to the file. */
  //itk::StdStreamLogOutput::Pointer m_LogFileOutput
  //                                         = itk::StdStreamLogOutput::New();

  //m_LogFileName = "LogIGSdemonstrator" + itksys::SystemTools::GetCurrentDateTime( "_%Y_%m_%d_%H_%M_%S" ) + ".txt";
  //m_LogFile.open( m_LogFileName.c_str() );

  //if( m_LogFile.fail() )
  //{
  //  //Return if fail to open the log file
  //  igstkLogMacro2( m_Logger, DEBUG, 
  //    "Problem opening Log file:" << m_LogFileName << "\n" );
  //  return;
  //}

  //m_LogFileOutput->SetStream( m_LogFile );
  //this->GetLogger()->AddLogOutput( m_LogFileOutput );
  //igstkLogMacro2( m_Logger, DEBUG, 
  //  "Successfully opened Log file:" << m_LogFileName << "\n" );

}

void ErrorManager::handleMessage(std::string errorMessage, bool showDialog)
{
  igstkLogMacro2( m_Logger, DEBUG, 
    errorMessage.c_str() << "\n" );
  if(showDialog)
  {
    /*QErrorMessage *errorMessageDialog = new QErrorMessage(NULL);
    errorMessageDialog->showMessage(errorMessage.c_str());*/
    QMessageBox::warning(NULL,"Warning", 
      QString(errorMessage.c_str()));
  }
}