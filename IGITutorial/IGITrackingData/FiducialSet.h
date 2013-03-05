#ifndef __FiducialSet_h
#define __FiducialSet_h

#include <QtGui>
#include "igstkView.h"
#include "ErrorManager.h"

#include "igstkEllipsoidObject.h"
#include "igstkEllipsoidObjectRepresentation.h"
#include "igstkCTImageReader.h"
#include "igstkAxesObject.h"

class FiducialSet : ErrorManager
{
  /** typedef for CT image reader */
  typedef igstk::CTImageReader::ImageSpatialObjectType::PointType         PointType;

public:
  FiducialSet();
  FiducialSet(QWidget *parent);
  ~FiducialSet();
  unsigned int GetNrOfPoints();
  void Clear();
  QString GetNextAvailableFiducialId();
  void AddNewFiducial(PointType point,  igstk::AxesObject::Pointer m_WorldReference);
  QString SaveFiducials(QString path);
 
private:
  QMap<QString,igstk::View::Pointer>  m_Views;
  std::map<QString,PointType>         m_Plan;
  QWidget*                            m_Parent;
  QString                             m_FiducialSetFilename;
  QString                             m_FiducialId;
  double                              m_Color[3];

  /** Utility functions, conversion between points and transform */
  inline
  igstk::Transform
  PointToTransform( PointType point)
  {
    igstk::Transform transform;
    igstk::Transform::VectorType translation;
    for (int i=0; i<3; i++)
      {
      translation[i] = point[i];
      }
    transform.SetTranslation( translation, 0.1,
                                igstk::TimeStamp::GetLongestPossibleTime() );
    return transform;
  }
};

#endif
