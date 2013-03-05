#include "FiducialSet.h"

#include "../IGIConfigurationData.h"

FiducialSet::FiducialSet(QWidget *parent)
  :m_Parent(parent)
{
  m_Color[0] = 1.0;
  m_Color[1] = 1.0;
  m_Color[2] = 0.0;

  m_FiducialButtonWidget = new ButtonSetWidget(parent);
}

FiducialSet::~FiducialSet(){}

void FiducialSet::SetColor(double r, double g, double b)
{
  m_Color[0] = r;
  m_Color[1] = g;
  m_Color[2] = b;
}

void FiducialSet::AddView(QString name, igstk::View::Pointer view)
{
  m_Views[name]=view;
}

ButtonSetWidget* FiducialSet::GetButtonWidget()
{
  return m_FiducialButtonWidget;
}

void FiducialSet::Clear()
{
  // remove objects from UI and views 
  std::map<QString,PointType>::iterator iFiducial;
  for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
  {
    m_FiducialButtonWidget->RemoveButton(iFiducial->first);

    m_Views["axial"]->RequestRemoveObject( m_AxialFiducialRepresentationVector[iFiducial->first] );
    m_Views["sagittal"]->RequestRemoveObject( m_SagittalFiducialRepresentationVector[iFiducial->first] );
    m_Views["coronal"]->RequestRemoveObject( m_CoronalFiducialRepresentationVector[iFiducial->first] );
    m_Views["view3D"]->RequestRemoveObject( m_3DViewFiducialRepresentationVector[iFiducial->first] );
  }
 
  // clear all fiducial related containers
  m_Plan.clear();
  m_FiducialPointVector.clear();
  m_AxialFiducialRepresentationVector.clear();
  m_SagittalFiducialRepresentationVector.clear();
  m_CoronalFiducialRepresentationVector.clear();
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

void FiducialSet::LoadFiducialsFromXMLPath(QString path, igstk::AxesObject::Pointer m_WorldReference)
{
  QString fName = QFileDialog::getOpenFileName(m_Parent,
      "Load Fiducial Set",
      path,
      "Fiducials (*.xml)");
  
  if(fName.isNull())
    return;

  if( fName.isEmpty() )
  {
    handleMessage("No fiducial set file [.xml] was selected\n", 1 );
    return;
  }
  m_FiducialSetFilename = fName;

  LoadFiducialsFromXMLfile(m_FiducialSetFilename, m_WorldReference);
}

void FiducialSet::LoadFiducialsFromXMLfile(QString fName, igstk::AxesObject::Pointer m_WorldReference)
{
  this->Clear();
  QFile file( fName );

  if( !file.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( m_Parent, "Loading", "Failed to load file." );
    return;
  }

  QDomDocument doc( "FiducialSet" );
  if( !doc.setContent( &file ) )
  {
    QMessageBox::warning( m_Parent, "Loading", "Failed to import XML data." );
    file.close();
    return;
  }

  file.close();

  QDomElement root = doc.documentElement();
  if( root.tagName() != "Fiducials" )
  {
    QMessageBox::warning( m_Parent, "Loading", "Invalid file." );
    return;
  }
  
  QDomNode n = root.firstChild();
  while( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if( !e.isNull() )
    {
      if( e.tagName() == "Fiducial" )
      {
        PointType p;
        QString id =  e.attribute( "id", "" );
        p[0] = (e.attribute( "x", "" )).toDouble();
        p[1] = (e.attribute( "y", "" )).toDouble();
        p[2] = (e.attribute( "z", "" )).toDouble();

        m_Plan[id]=p;
      }
    }
    n = n.nextSibling();
  }

  // read fiducials from xml file into id-point container
  // set up fiducual representations
  std::map<QString,PointType>::iterator iFiducial;
  for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
  {
    m_FiducialButtonWidget->AddButton(iFiducial->first);
  
    m_FiducialPointVector[iFiducial->first] = EllipsoidType::New();
    m_FiducialPointVector[iFiducial->first]->SetRadius(
                                       IGIConfigurationData::SPHERE_DIAMETER,
                                       IGIConfigurationData::SPHERE_DIAMETER,
                                       IGIConfigurationData::SPHERE_DIAMETER );
    m_FiducialPointVector[iFiducial->first]->RequestSetTransformAndParent(
                                                    PointToTransform(iFiducial->second), 
                                                    m_WorldReference );
                                                    
    m_AxialFiducialRepresentationVector[iFiducial->first] = EllipsoidRepresentationType::New();
    m_AxialFiducialRepresentationVector[iFiducial->first]->RequestSetEllipsoidObject( m_FiducialPointVector[iFiducial->first] );
    m_AxialFiducialRepresentationVector[iFiducial->first]->SetOpacity( 0.60 );

    m_SagittalFiducialRepresentationVector[iFiducial->first] = EllipsoidRepresentationType::New();
    m_SagittalFiducialRepresentationVector[iFiducial->first]->RequestSetEllipsoidObject( m_FiducialPointVector[iFiducial->first] );
    m_SagittalFiducialRepresentationVector[iFiducial->first]->SetOpacity( 0.60 );

    m_CoronalFiducialRepresentationVector[iFiducial->first] = EllipsoidRepresentationType::New();
    m_CoronalFiducialRepresentationVector[iFiducial->first]->RequestSetEllipsoidObject( m_FiducialPointVector[iFiducial->first] );
    m_CoronalFiducialRepresentationVector[iFiducial->first]->SetOpacity( 0.60 );

    m_3DViewFiducialRepresentationVector[iFiducial->first] = EllipsoidRepresentationType::New();
    m_3DViewFiducialRepresentationVector[iFiducial->first]->RequestSetEllipsoidObject( m_FiducialPointVector[iFiducial->first] );
    m_3DViewFiducialRepresentationVector[iFiducial->first]->SetOpacity( 0.60 );

    m_AxialFiducialRepresentationVector[iFiducial->first]->SetColor( m_Color[0], m_Color[1], m_Color[2]);
    m_SagittalFiducialRepresentationVector[iFiducial->first]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);
    m_CoronalFiducialRepresentationVector[iFiducial->first]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);
    m_3DViewFiducialRepresentationVector[iFiducial->first]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);

    // add new fiducial representation to the view
    m_Views["axial"]->RequestAddObject( m_AxialFiducialRepresentationVector[iFiducial->first] );
    m_Views["sagittal"]->RequestAddObject( m_SagittalFiducialRepresentationVector[iFiducial->first] );
    m_Views["coronal"]->RequestAddObject( m_CoronalFiducialRepresentationVector[iFiducial->first] );
    m_Views["view3D"]->RequestAddObject( m_3DViewFiducialRepresentationVector[iFiducial->first] );
  }
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
  m_Views["axial"]->RequestAddObject( m_AxialFiducialRepresentationVector[fiducialId] );
  m_Views["sagittal"]->RequestAddObject( m_SagittalFiducialRepresentationVector[fiducialId] );
  m_Views["coronal"]->RequestAddObject( m_CoronalFiducialRepresentationVector[fiducialId] );
  m_Views["view3D"]->RequestAddObject( m_3DViewFiducialRepresentationVector[fiducialId] );
}

void FiducialSet::RemoveSelectedFiducial()
{
  // remove objects from UI and views 
  m_FiducialButtonWidget->RemoveButton(m_FiducialId);
  
  m_Views["axial"]->RequestRemoveObject( m_AxialFiducialRepresentationVector[m_FiducialId] );
  m_Views["sagittal"]->RequestRemoveObject( m_SagittalFiducialRepresentationVector[m_FiducialId] );
  m_Views["coronal"]->RequestRemoveObject( m_CoronalFiducialRepresentationVector[m_FiducialId] );
  m_Views["view3D"]->RequestRemoveObject( m_3DViewFiducialRepresentationVector[m_FiducialId] );
  
  // clear all fiducial related containers
  m_Plan.erase(m_FiducialId);
  m_FiducialPointVector.erase(m_FiducialId);
  m_AxialFiducialRepresentationVector.erase(m_FiducialId);
  m_SagittalFiducialRepresentationVector.erase(m_FiducialId);
  m_CoronalFiducialRepresentationVector.erase(m_FiducialId);
  m_3DViewFiducialRepresentationVector.erase(m_FiducialId);
}

void FiducialSet::AddNewFiducial(PointType point)
{
  QString fiducialId = GetNextAvailableFiducialId();

  m_Plan[fiducialId] = point;

  m_FiducialButtonWidget->AddButton(fiducialId);

  m_FiducialPointVector[fiducialId] = EllipsoidType::New();

  m_FiducialPointVector[fiducialId]->SetRadius(
                                    IGIConfigurationData::SPHERE_DIAMETER,
                                    IGIConfigurationData::SPHERE_DIAMETER,
                                    IGIConfigurationData::SPHERE_DIAMETER );

  m_AxialFiducialRepresentationVector[fiducialId] = EllipsoidRepresentationType::New();
  m_AxialFiducialRepresentationVector[fiducialId]->RequestSetEllipsoidObject( m_FiducialPointVector[fiducialId] );
  m_AxialFiducialRepresentationVector[fiducialId]->SetOpacity( 0.60 );

  m_SagittalFiducialRepresentationVector[fiducialId] = EllipsoidRepresentationType::New();
  m_SagittalFiducialRepresentationVector[fiducialId]->RequestSetEllipsoidObject( m_FiducialPointVector[fiducialId] );
  m_SagittalFiducialRepresentationVector[fiducialId]->SetOpacity( 0.60 );

  m_CoronalFiducialRepresentationVector[fiducialId] = EllipsoidRepresentationType::New();
  m_CoronalFiducialRepresentationVector[fiducialId]->RequestSetEllipsoidObject( m_FiducialPointVector[fiducialId] );
  m_CoronalFiducialRepresentationVector[fiducialId]->SetOpacity( 0.60 );

  m_3DViewFiducialRepresentationVector[fiducialId] = EllipsoidRepresentationType::New();
  m_3DViewFiducialRepresentationVector[fiducialId]->RequestSetEllipsoidObject( m_FiducialPointVector[fiducialId] );
  m_3DViewFiducialRepresentationVector[fiducialId]->SetOpacity( 0.60 );

  m_AxialFiducialRepresentationVector[fiducialId]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);
  m_SagittalFiducialRepresentationVector[fiducialId]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);
  m_CoronalFiducialRepresentationVector[fiducialId]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);
  m_3DViewFiducialRepresentationVector[fiducialId]->SetColor(  m_Color[0], m_Color[1], m_Color[2]);

  // add new fiducial representation to the view
  m_Views["axial"]->RequestAddObject( m_AxialFiducialRepresentationVector[fiducialId] );
  m_Views["sagittal"]->RequestAddObject( m_SagittalFiducialRepresentationVector[fiducialId] );
  m_Views["coronal"]->RequestAddObject( m_CoronalFiducialRepresentationVector[fiducialId] );
  m_Views["view3D"]->RequestAddObject( m_3DViewFiducialRepresentationVector[fiducialId] );
}

/**
 *  Write the image fiducials to file
 */
QString FiducialSet::SaveFiducials(QString path)
{  
  QString fileName = QFileDialog::getSaveFileName(
         m_Parent,
         "Save Fiducial Set as...",
         path,
         "Fiducial Set (*.xml)" );

  m_FiducialSetFilename = fileName;

  QDomDocument doc( "FiducialSet" );
  QDomElement root = doc.createElement( "Fiducials" );
  doc.appendChild( root );

  std::map<QString,PointType>::iterator iFiducial;
  for (iFiducial=m_Plan.begin(); iFiducial != m_Plan.end(); iFiducial++)
  {
    root.appendChild(PointToNode(doc, 
                                  m_Plan[iFiducial->first], 
                                  iFiducial->first));
  }

  QFile file( m_FiducialSetFilename );
  if( !file.open( QIODevice::WriteOnly ) )
    return NULL;

  QTextStream ts( &file );
  ts << doc.toString();
  file.close();

  return m_FiducialSetFilename;
}

QDomElement FiducialSet::PointToNode( QDomDocument &d, PointType point, QString id )
{
  QDomElement cn = d.createElement( "Fiducial" );

  cn.setAttribute( "id", id );
  cn.setAttribute( "x", point[0] );
  cn.setAttribute( "y", point[1] );
  cn.setAttribute( "z", point[2] );

  return cn;
}