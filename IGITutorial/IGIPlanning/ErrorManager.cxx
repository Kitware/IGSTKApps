#include "ErrorManager.h"

#include "itksys/SystemTools.hxx"
#include "itksys/Directory.hxx"

#include <QDialog>
#include <QErrorMessage>
#include <QMessageBox>

ErrorManager::ErrorManager()
{}

void ErrorManager::handleMessage(std::string errorMessage, bool showDialog)
{
  if(showDialog)
  {
    QMessageBox::warning(NULL,"Warning", 
      QString(errorMessage.c_str()));
  }
}