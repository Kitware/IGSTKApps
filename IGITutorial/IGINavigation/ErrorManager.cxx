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
}

void ErrorManager::handleMessage(std::string errorMessage, bool showDialog)
{
  igstkLogMacro2( m_Logger, DEBUG,
    errorMessage.c_str() << "\n" );
  if(showDialog)
  {
    QMessageBox::critical(NULL,"Error",
      QString(errorMessage.c_str()));
  }
}
