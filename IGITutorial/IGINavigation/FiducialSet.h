#ifndef __FiducialSet_h
#define __FiducialSet_h

#include <QtGui>
#include <QtXml/qdom.h>

#include "igstkView.h"
#include "ButtonSetWidget.h"
#include "ErrorManager.h"

#include "igstkEllipsoidObject.h"
#include "igstkEllipsoidObjectRepresentation.h"
#include "igstkCTImageReader.h"
#include "igstkAxesObject.h"

class FiducialSet : ErrorManager
{
  /** typedef for CT image reader */
  typedef igstk::CTImageReader::ImageSpatialObjectType::PointType         PointType;
  /** Ellipsoid spatial object, used to represent the fiducial points*/
  typedef igstk::EllipsoidObject                  EllipsoidType;
  typedef igstk::EllipsoidObjectRepresentation    EllipsoidRepresentationType;

public:
  FiducialSet();
  FiducialSet(QWidget *parent);
  ~FiducialSet();
  void Clear();
  void AddView(QString name, igstk::View::Pointer view);
  ButtonSetWidget* GetButtonWidget();
  void LoadFiducialsFromXMLPath(QString path, igstk::AxesObject::Pointer m_WorldReference);
  bool LoadFiducialsFromXMLfile(QString fName, igstk::AxesObject::Pointer m_WorldReference);
  QString GetNextAvailableFiducialId();
  void RemoveSelectedFiducial();
  void AddNewFiducial(PointType point);
  QString SaveFiducials(QString path);
  PointType GetPositionOfFiducial(QString fiducialId);
  QDomElement PointToNode( QDomDocument &d, PointType point, QString id );
  void RepositionFiducial( QString fiducialId,
                                        igstk::Transform newPosition,
                                        igstk::AxesObject::Pointer m_WorldReference);
  void SetColor(double r, double g, double b);
  void SetOpacity(double value);

  QString GetSelectedId()
  {return m_FiducialId;}

private:
  QMap<QString,igstk::View::Pointer>  m_Views;
  ButtonSetWidget*                    m_FiducialButtonWidget;
  std::map<QString,PointType>         m_Plan;
  QWidget*                            m_Parent;
  QString                             m_FiducialSetFilename;
  QString                             m_FiducialId;
  double                              m_Color[3];

  /** fiducial visualization */
  std::map< QString, EllipsoidType::Pointer >                 m_FiducialPointVector;
  std::map< QString, EllipsoidRepresentationType::Pointer >   m_AxialFiducialRepresentationVector;
  std::map< QString, EllipsoidRepresentationType::Pointer >   m_SagittalFiducialRepresentationVector;
  std::map< QString, EllipsoidRepresentationType::Pointer >   m_CoronalFiducialRepresentationVector;
  std::map< QString, EllipsoidRepresentationType::Pointer >   m_3DViewFiducialRepresentationVector;

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
