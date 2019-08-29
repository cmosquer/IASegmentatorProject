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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "IASegmentadorAbdominalView.h"

// Qt
#include <QMessageBox>
#include "Inference.h"
#define ABDOMINAL 0

const std::string IASegmentadorAbdominalView::VIEW_ID = "org.mitk.views.iasegmentadorabdominalview";

void IASegmentadorAbdominalView::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void IASegmentadorAbdominalView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  //connect(m_Controls.cbHigado, SIGNAL(activated(int)), this, SLOT(onHigado()));
  connect(m_Controls.cbTodos, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onTodos);

  connect(m_Controls.cbBazo, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onBazo);
  connect(m_Controls.cbRinonIzquierdo, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onRinonIzquierdo);
  connect(m_Controls.cbVesicula, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onVesicula);
  connect(m_Controls.cbEsofago, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onEsofago);
  connect(m_Controls.cbHigado, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onHigado);
  connect(m_Controls.cbEstomago, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onEstomago);
  connect(m_Controls.cbPancreas, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onPancreas);
  connect(m_Controls.cbDuodeno, &QCheckBox::clicked,this,&IASegmentadorAbdominalView::onDuodeno);

  connect(m_Controls.buttonPerformImageProcessing, &QPushButton::clicked, this, &IASegmentadorAbdominalView::DoImageProcessing);



  my_infer_process = new Inference;

}

void IASegmentadorAbdominalView::toggleLabel(int lbl)
{
    if (my_infer_process->m_selected_labels[lbl]==0)
    {
        my_infer_process->m_selected_labels[lbl]=1;

    }
    else if (my_infer_process->m_selected_labels[lbl]==1) {
        my_infer_process->m_selected_labels[lbl] =0;

    }
}

void IASegmentadorAbdominalView::onTodos()
{
    int new_lbl;
    if (m_Controls.cbTodos->checkState()) //Lo marcaron
    {
        new_lbl = 1;
        cout<<"Marcado"<<endl;
        m_Controls.cbBazo->setChecked(true);
        m_Controls.cbRinonIzquierdo->setChecked(true);
        m_Controls.cbVesicula->setChecked(true);
        m_Controls.cbEsofago->setChecked(true);
        m_Controls.cbEstomago->setChecked(true);
        m_Controls.cbHigado->setChecked(true);
        m_Controls.cbDuodeno->setChecked(true);
        m_Controls.cbPancreas->setChecked(true);
    }
    else //Lo desmarcaron
    {
        new_lbl = 0;
        cout<<"Desmarcado"<<endl;
        m_Controls.cbBazo->setChecked(false);
        m_Controls.cbRinonIzquierdo->setChecked(false);
        m_Controls.cbVesicula->setChecked(false);
        m_Controls.cbEsofago->setChecked(false);
        m_Controls.cbEstomago->setChecked(false);
        m_Controls.cbHigado->setChecked(false);
        m_Controls.cbDuodeno->setChecked(false);
        m_Controls.cbPancreas->setChecked(false);
    }
    for(unsigned long i=0; i<8;i++)
    {
        my_infer_process->m_selected_labels[i] = new_lbl;

    }
}
void IASegmentadorAbdominalView::onBazo()
{
    toggleLabel(0);
}

void IASegmentadorAbdominalView::onRinonIzquierdo()
{
    toggleLabel(1);
}

void IASegmentadorAbdominalView::onVesicula()
{
    toggleLabel(2);
}

void IASegmentadorAbdominalView::onEsofago()
{
    toggleLabel(3);
}

void IASegmentadorAbdominalView::onHigado()
{
    toggleLabel(4);
}

void IASegmentadorAbdominalView::onEstomago()
{
    toggleLabel(5);
}

void IASegmentadorAbdominalView::onPancreas()
{
    toggleLabel(6);
}

void IASegmentadorAbdominalView::onDuodeno()
{
    toggleLabel(7);
}

void IASegmentadorAbdominalView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach (mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull() && dynamic_cast<mitk::Image *>(node->GetData()))
    {
      m_Controls.labelWarning->setVisible(false);
      m_Controls.buttonPerformImageProcessing->setEnabled(true);
      m_Controls.cbBazo->setEnabled(true);
      m_Controls.cbRinonIzquierdo->setEnabled(true);
      m_Controls.cbVesicula->setEnabled(true);
      m_Controls.cbEsofago->setEnabled(true);
      m_Controls.cbEstomago->setEnabled(true);
      m_Controls.cbHigado->setEnabled(true);
      m_Controls.cbDuodeno->setEnabled(true);
      m_Controls.cbPancreas->setEnabled(true);
      m_Controls.cbTodos->setEnabled(true);

      return;
    }
  }

  m_Controls.labelWarning->setVisible(true);
  m_Controls.cbBazo->setEnabled(false);
  m_Controls.cbRinonIzquierdo->setEnabled(false);
  m_Controls.cbVesicula->setEnabled(false);
  m_Controls.cbEsofago->setEnabled(false);
  m_Controls.cbEstomago->setEnabled(false);
  m_Controls.cbHigado->setEnabled(false);
  m_Controls.cbDuodeno->setEnabled(false);
  m_Controls.cbPancreas->setEnabled(false);
  m_Controls.cbTodos->setEnabled(false);

  m_Controls.buttonPerformImageProcessing->setEnabled(false);
}

bool IASegmentadorAbdominalView::checkIfOrganSegm(std::string nodeName)
{
    for(std::vector<int>::size_type i = 0; i != my_infer_process->organs_names.size(); i++)
    {

        if (nodeName.find(my_infer_process->organs_names[i]) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}


void IASegmentadorAbdominalView::DoImageProcessing()
{
  cout<<CLOCKS_PER_SEC<<endl;
  return;
  clock_t tStart = clock();

  cout<<"Botón presionado"<<endl;
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty())
    return;
  std::string model_type = "abdominal";
  mitk::DataNode *node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(nullptr, "Template", "Por favor cargue y seleccione una imagen de TC abdominal.");

    return;
  }
  if (checkIfOrganSegm(node->GetName()))
  {
    QMessageBox::information(nullptr, "Template", "Por favor seleccione una imagen válida.");
    return;
  }
  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData *data = node->GetData();
  if (data)
  {

    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image *image = dynamic_cast<mitk::Image *>(data);
    if (image)
    {
      //std::stringstream message;
      std::string name;
      //message << "Performing image processing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        cout << "'" << name << "'"<<endl;
      }
      //message << ".";
      //MITK_INFO << message.str();
    //mitk::DataNode::Pointer output_node = mitk::DataNode::New();
      QString nodename(node->GetName().c_str());
      m_Controls.infolabel->setText("Segmentación en proceso...");
      m_Controls.buttonPerformImageProcessing->setEnabled(false);
      my_infer_process->InfereSegmentation(image,nodename,ABDOMINAL,GetDataStorage());
      m_Controls.infolabel->setText("");
      m_Controls.buttonPerformImageProcessing->setEnabled(true);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    //GetDataStorage()->Add(output_node);
    }

  }
  double timetaken = clock() - tStart;
  cout<< "Clocks taken: "<<timetaken<<endl;
}

IASegmentadorAbdominalView::~IASegmentadorAbdominalView()
{
    cout<<"Destructor"<<endl;
    my_infer_process->CleanDir();
}
