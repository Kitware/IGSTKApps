#ifndef __ButtonSetWidget_h
#define __ButtonSetWidget_h

/** Qt */
#include <QtGui>

class ButtonSetWidget : public QWidget
{
  Q_OBJECT

  public:
  ButtonSetWidget();
  ~ButtonSetWidget();
  ButtonSetWidget(QStringList texts, QWidget *parent);

  ButtonSetWidget(QWidget *parent);

  void AddButton(QString name);

  void RemoveButton(QString name);

  QString CheckButton(QString name);
  QString CheckNextButton(QString selectedId);

  signals:
    void clicked(const QString &text);

  private:
    QSignalMapper *signalMapper;
    QGridLayout *gridLayout;
    QMap<QString,QPushButton*> buttonContainer;
    int iButton;
};
#endif

