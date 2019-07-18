/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef IASegmentadorAbdominalView_h
#define IASegmentadorAbdominalView_h

#include <berryISelectionListener.h>
#include "Inference.h"
#include <QmitkAbstractView.h>
#include <mitkImage.h>
#include "ui_IASegmentadorAbdominalViewControls.h"

/**
  \brief IASegmentadorAbdominalView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class IASegmentadorAbdominalView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;
  ~IASegmentadorAbdominalView();
protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing(); // called when the user press the button
  void toggleLabel(int lbl);
  bool checkIfOrganSegm(std::string nodeName);
  Ui::IASegmentadorAbdominalViewControls m_Controls;
  Inference* my_infer_process;

private slots:
  void onBazo();
  void onRinonIzquierdo();
  void onVesicula();
  void onEsofago();
  void onHigado();
  void onEstomago();
  void onPancreas();
  void onDuodeno();
  void onTodos();

};

#endif // IASegmentadorAbdominalView_h
