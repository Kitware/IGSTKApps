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
  buttonContainer[name]=button;

  RearrangeGridLayout();
}

void ButtonSetWidget::RemoveButton(QString name)
{
  QPushButton *button = buttonContainer[name];
  signalMapper->removeMappings(button);
  delete button;
  buttonContainer.remove(name);

  RearrangeGridLayout();
}

void ButtonSetWidget::RearrangeGridLayout()
{
  iButton = 0;
  QMap<QString, QPushButton*>::Iterator iButtonContainer;
  for(iButtonContainer = buttonContainer.begin();
    iButtonContainer != buttonContainer.end();
    iButtonContainer++)
  {
    gridLayout->removeWidget(iButtonContainer.value());
  }

  for(iButtonContainer = buttonContainer.begin();
    iButtonContainer != buttonContainer.end();
    iButtonContainer++)
  {
    gridLayout->addWidget(iButtonContainer.value(), iButton / 3, iButton % 3);
    iButton++;
  }
}
