#include "FiducialSet.h"

#include "../IGIConfigurationData.h"

#define SPHERE_DIAMETER 3
#define SKIPVISNR 7

FiducialSet::FiducialSet(QWidget *parent)
  :m_Parent(parent)
{
  m_Color[0] = 1.0;
  m_Color[1] = 0.0;
  m_Color[2] = 0.0;
}

FiducialSet::~FiducialSet(){}

void FiducialSet::SetColor(double r, double g, double b)
{
  m_Color[0] = r;
  m_Color[1] = g;
  m_Color[2] = b;
}

unsigned int FiducialSet::GetNrOfPoints()
{
  return m_Plan.size();
}

unsigned int FiducialSet::GetNrOfVisualizedPoints()
{
  return m_FiducialPointVector.size();
}

void FiducialSet::AddView(QString name, igstk::View::Pointer view)
{
  m_Views[name]=view;
}

void FiducialSet::Clear()
{
  // remove objects from UI and views 
  std::map<QString,PointType>::iterator iFiducial;
  for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
  {
    m_Views["view3D"]->RequestRemoveObject( m_3DViewFiducialRepresentationVector[iFiducial->first] );
  }
 
  // clear all fiducial related containers
  m_Plan.clear();
  m_FiducialPointVector.clear();
  m_3DViewFiducialRepresentationVector.clear();
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

void FiducialSet::LoadFiducialsFromTXTPath(QString path, igstk::AxesObject::Pointer m_WorldReference)
{
  QString fName = QFileDialog::getOpenFileName(m_Parent,
      "Load Point Set",
      path,
      "3D Points (*.txt)");
  
  if(fName.isNull())
    return;

  if ( fName.isEmpty() )
  {
    handleMessage("No point set file [.txt] was selected\n", 1 );
    return;
  }
  m_FiducialSetFilename = fName;
  LoadFiducialsFromTXTfile(m_FiducialSetFilename, m_WorldReference);
}

bool FiducialSet::LoadFiducialsFromTXTfile(QString fName, igstk::AxesObject::Pointer m_WorldReference)
{
  this->Clear();
  
  std::ifstream planFile( fName.toStdString().c_str() );
  if ( planFile.is_open())
    {
      std::string line;
      float p[3];
      unsigned int id = 0;
      while ( std::getline( planFile, line ) )
      {
        if( sscanf( line.c_str(), "%f %f %f", &p[0], &p[1], &p[2]) != 3 )
        {
          std::cerr << "Incorrect file format!\n";
          planFile.close();
          return 0;
        }
        FiducialSet::PointType fp;
        fp[0] = p[0];
        fp[1] = p[1];
        fp[2] = p[2];
        m_Plan[QString::number(id)]=fp;
        id++;
      }
      planFile.close();
  }
  else
  {
    QMessageBox::warning( m_Parent, "Loading", "Failed to load point set file." );
    return false;
  }  
          
  // set up fiducual representations
  int loadedFidNr = 0;
  std::map<QString,PointType>::iterator iFiducial;
  for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
  {
    if( (loadedFidNr % SKIPVISNR) == 0)
    {
      m_FiducialPointVector[iFiducial->first] = EllipsoidType::New();
      m_FiducialPointVector[iFiducial->first]->SetRadius(
                                          SPHERE_DIAMETER,
                                          SPHERE_DIAMETER,
                                          SPHERE_DIAMETER );
      m_FiducialPointVector[iFiducial->first]->RequestSetTransformAndParent(
                                              PointToTransform(iFiducial->second), 
                                              m_WorldReference );

      m_3DViewFiducialRepresentationVector[iFiducial->first] = EllipsoidRepresentationType::New();
      m_3DViewFiducialRepresentationVector[iFiducial->first]->RequestSetEllipsoidObject( m_FiducialPointVector[iFiducial->first] );
      m_3DViewFiducialRepresentationVector[iFiducial->first]->SetOpacity( 0.60 );

      m_3DViewFiducialRepresentationVector[iFiducial->first]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);

      // add new fiducial representation to the view
      m_Views["view3D"]->RequestAddObject( m_3DViewFiducialRepresentationVector[iFiducial->first] );
    }
    loadedFidNr++;
  }
  return true;
}

/** -----------------------------------------------------------------
*  Switches the currently active image fiducial
*---------------------------------------------------------------------
*/
FiducialSet::PointType FiducialSet::GetPositionOfFiducial(QString fiducialId)
{
  // get fiducial coordinates
  PointType point;
  return m_Plan[ fiducialId ];
}

void FiducialSet::RepositionFiducial(QString fiducialId,igstk::Transform newPosition, igstk::AxesObject::Pointer m_WorldReference)
{
  // get currently selecetd fiducial id
  m_FiducialId = fiducialId;

  m_FiducialPointVector[fiducialId]->RequestSetTransformAndParent(
  newPosition, m_WorldReference );

  // add new fiducial representation to the view
  m_Views["view3D"]->RequestAddObject( m_3DViewFiducialRepresentationVector[fiducialId] );
}

void FiducialSet::RemoveSelectedFiducial()
{
  m_Views["view3D"]->RequestRemoveObject( m_3DViewFiducialRepresentationVector[m_FiducialId] );
  
  // clear all fiducial related containers
  m_Plan.erase(m_FiducialId);
  m_FiducialPointVector.erase(m_FiducialId);
  m_3DViewFiducialRepresentationVector.erase(m_FiducialId);
}

void FiducialSet::AddNewFiducial(PointType point,  igstk::AxesObject::Pointer m_WorldReference)
{
  QString fiducialId = GetNextAvailableFiducialId();

  // save each point position
  m_Plan[fiducialId] = point;

  // add each n^th point visualization to the view  
  int fiducialIdInt = fiducialId.toInt();
  if( (fiducialIdInt % SKIPVISNR) == 0 )
  {
    m_FiducialPointVector[fiducialId] = EllipsoidType::New();

    m_FiducialPointVector[fiducialId]->SetRadius(
                                          SPHERE_DIAMETER,
                                          SPHERE_DIAMETER,
                                          SPHERE_DIAMETER );

    m_FiducialPointVector[fiducialId]->RequestSetTransformAndParent(
    PointToTransform(point), m_WorldReference );

    m_3DViewFiducialRepresentationVector[fiducialId] = EllipsoidRepresentationType::New();
    m_3DViewFiducialRepresentationVector[fiducialId]->RequestSetEllipsoidObject( m_FiducialPointVector[fiducialId] );
    m_3DViewFiducialRepresentationVector[fiducialId]->SetOpacity( 1 );

    m_3DViewFiducialRepresentationVector[fiducialId]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);

    // add new fiducial representation to the view
    m_Views["view3D"]->RequestAddObject( m_3DViewFiducialRepresentationVector[fiducialId] );
  }
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
