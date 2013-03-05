#include "FiducialSet.h"

#include "../IGIConfigurationData.h"

FiducialSet::FiducialSet(QWidget *parent)
  :m_Parent(parent)
{
}

FiducialSet::~FiducialSet(){}

unsigned int FiducialSet::GetNrOfPoints()
{
  return m_Plan.size();
}

void FiducialSet::Clear()
{
  // clear all fiducial related containers
  m_Plan.clear();
}

QString FiducialSet::GetNextAvailableFiducialId()
{
  if(m_Plan.size() == 0)
    return QString("1");

  int startId;
  startId = ((m_Plan.begin())->first).toInt();
  std::map<QString,PointType>::iterator iFiducial;
  for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
  {
    if(m_Plan.count(QString::number(startId + 1)))
      startId++;
    else
      return QString::number(startId + 1);
  }
  return 0;
}

void FiducialSet::AddNewFiducial(PointType point,  igstk::AxesObject::Pointer m_WorldReference)
{
  QString fiducialId = GetNextAvailableFiducialId();

  // save each point position
  m_Plan[fiducialId] = point;

}

/**
 *  Write the image fiducials to file
 */
QString FiducialSet::SaveFiducials(QString path)
{  
  QString fileName = QFileDialog::getSaveFileName(m_Parent,
                                                  "Save Point Set as...",
                                                  path,
                                                  "Point Set (*.txt)" );
  m_FiducialSetFilename = fileName;
  
  std::ofstream  planFile;
  planFile.open( fileName.toStdString().c_str(), ios::trunc );
  
  if ( planFile.is_open())
  {
    std::map<QString,PointType>::iterator iFiducial;
    for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
    {
     
      FiducialSet::PointType p = m_Plan[iFiducial->first];
      planFile << p[0] << "\t" << p[1] << "\t" << p[2] << "\n";
                                
    }
    planFile.close();
    return m_FiducialSetFilename;;
  }
  else
  {
    return NULL;
  }  
}

void FiducialSet::SetOpacity(double value)
{
  std::map<QString,PointType>::iterator iFiducial;
  for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
  {
    m_3DViewFiducialRepresentationVector[iFiducial->first]->SetOpacity(value);
  } 
}
