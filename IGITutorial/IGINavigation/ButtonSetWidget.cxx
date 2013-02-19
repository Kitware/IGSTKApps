#include "ButtonSetWidget.h"

ButtonSetWidget::ButtonSetWidget(){}

ButtonSetWidget::~ButtonSetWidget(){}

ButtonSetWidget::ButtonSetWidget(QStringList texts, QWidget *parent)
: QWidget(parent)
{
  signalMapper = new QSignalMapper(this);

  QGridLayout *gridLayout = new QGridLayout;

  for (int i = 0; i < texts.size(); ++i)
  {
    QPushButton *button = new QPushButton(texts[i]);
    button->setCheckable(true);
    button->setAutoExclusive(true);
    connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(button, texts[i]);
    gridLayout->addWidget(button, i / 3, i % 3);
  }
  connect(signalMapper, SIGNAL(mapped(const QString &)),
      this, SIGNAL(clicked(const QString &)));

  setLayout(gridLayout);
}

ButtonSetWidget::ButtonSetWidget(QWidget *parent)
: QWidget(parent)
{
  signalMapper = new QSignalMapper(this);

  gridLayout = new QGridLayout;
  setLayout(gridLayout);
  iButton = 0;
  connect(signalMapper, SIGNAL(mapped(const QString &)),
      this, SIGNAL(clicked(const QString &)));
}

void ButtonSetWidget::AddButton(QString name)
{
  QPushButton *button = new QPushButton(name);
  button->setCheckable(true);
  button->setAutoExclusive(true);
  connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(button, name);
  gridLayout->addWidget(button, iButton / 3, iButton % 3);
  iButton++;
  buttonContainer[name]=button;
}

void ButtonSetWidget::RemoveButton(QString name)
{
  QPushButton *button = buttonContainer[name];
  signalMapper->removeMappings(button);
  gridLayout->removeWidget(button);
  delete button;
  buttonContainer.remove(name);

  if(buttonContainer.size()!=0)
  {
    QMap<QString, QPushButton*>::Iterator iButton;
    iButton = buttonContainer.begin();
    iButton.value()->setChecked(true);
  }
}

QString ButtonSetWidget::CheckButton(QString name)
{
  if(name.isEmpty())
  {
    if(buttonContainer.size()!=0)
    {
      QMap<QString, QPushButton*>::Iterator iButton;
      iButton = buttonContainer.begin();
      iButton.value()->setChecked(true);
      return iButton.key();
    }
  }
  else
  {
    if(buttonContainer.size()!=0)
    {
      buttonContainer[name]->setChecked(true);
      return name;
    }
  }
  return 0;
}

QString ButtonSetWidget::CheckNextButton(QString currentId)
{
  QMap<QString, QPushButton*>::Iterator iButton;
  iButton = buttonContainer.find(currentId);
  iButton++;
  if(iButton == buttonContainer.end())
  {
    return CheckButton("");
  }
  else
  {
    return CheckButton(iButton.key());
  }
}
