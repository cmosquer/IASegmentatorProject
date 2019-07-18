

#include "Inference.h"
#include <stdio.h>
// mitk image
#include <mitkImage.h>
//#include <fileutils.h>
#include <mitkIPythonService.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
//include <Python.h>
#include <unistd.h>
#include <itkImage.h>
#include <itkThresholdImageFilter.h>
#include <itkImageFileReader.h>
#include "mitkImageCast.h"


#include <QMessageBox>

typedef itksys::SystemTools ist;
static std::string NIFTYNETPATH = "/home/cineot/Documents/CANDE/work/bin/IASegmentador/Modules/IASegmentadorAbdominal/NiftyNet/";
static std::string ABDOMINALSUBDIR = "abdominal_inference/";
static std::string INPUT = "inputs/";
static std::string INPUTNAME = "_input.nii";
static std::string OUTPUT = "outputs/";
static std::string OUTPUTNAME = "_input_niftynet_out.nii.gz";
static std::string SHFILE = "segmentation.sh";
static std::string MASKS_SHFILE = "_mask.sh";


Inference::Inference()
{

}
Inference::~Inference()
{

}
inline bool exists (const std::string& filename) {
  struct stat buffer;
  return (stat (filename.c_str(), &buffer) == 0);
}
void Inference::SaveInput(mitk::Image *image, std::string inputpath)
{
    cout<<"Existencia: "<<exists(inputpath)<<endl;
    if (exists(inputpath)!=1)
    {
        mitk::IOUtil::Save(image, inputpath);
    }
    else
    {
        cout<<"ya existe la imagen en inputs"<<endl;
    }
}

void Inference::LoadImageFromPath(mitk::DataStorage::Pointer ds, std::string outputpath)
{
    mitk::IOUtil::Load(outputpath, *ds);
}

void Inference::InstallPackages()
{
    QProcess p;
    QStringList params;
    std::string bashfile = NIFTYNETPATH + ABDOMINALSUBDIR + SHFILE;
    QString bashfileQ(bashfile.c_str());
    params<<bashfileQ;
    p.start("bash",params);
    p.waitForFinished(-1);
    QString output(p.readAllStandardOutput());
    cout << output.toStdString() <<endl;
    p.close();

}

void Inference::ProcessNiftynet(int model_type)
{


    QProcess p;
    QStringList params;
    std::string bashfile = NIFTYNETPATH + ABDOMINALSUBDIR + SHFILE;
    QString bashfileQ(bashfile.c_str());
    params<<bashfileQ;
    p.start("bash",params);
    p.waitForFinished(-1);
    QString output(p.readAllStandardOutput());
    cout << output.toStdString() <<endl;
    p.close();

}

void Inference::CleanDir()
{

    QProcess p;

    QStringList params;
    std::string command = "rm -f " + NIFTYNETPATH + ABDOMINALSUBDIR + OUTPUT + "*";
    QString commandQ(command.c_str());
    params<<commandQ;
    cout<<"Comando clean dir: "<<command<<endl;
    p.start(commandQ);
    p.waitForFinished(-1);

    QStringList params2;
    std::string command2 = "-f " + NIFTYNETPATH + ABDOMINALSUBDIR + INPUT + "*";
    QString commandQ2(command2.c_str());
    params<<commandQ2;
    p.start("rm",params2);
    p.waitForFinished(-1);

    QString output = p.readAllStandardOutput();

    cout << output.toStdString() <<endl;

    p.close();


}
void Inference::CreateSegmentationMasks(mitk::DataStorage::Pointer ds, std::string node_name)
{


    QProcess p;

    std::string outputpath;
    for(std::vector<int>::size_type i = 0; i != m_selected_labels.size(); i++) {
        //cout<<organs_names[i]<< ". " << m_selected_labels[i]<<endl;

        if (m_selected_labels[i]==1)        {
            mitk::DataNode::Pointer current_node = ds->GetNamedNode(node_name+" "+organs_names[i]);
            if (current_node)
            {
                std::string msg = "Ya existe una segmentación de " + organs_names[i] + " para " +node_name + ".\n Si por algún motivo desea realizar la segmentación nuevamente, cambie el nombre del nodo " +node_name+" "+organs_names[i]+ ".";
                QString message(msg.c_str());
                QMessageBox::warning(NULL, "Segmentación ya existente",message);

            }
            else
            {
                std::string bashfile = NIFTYNETPATH + ABDOMINALSUBDIR + organs_names[i] + MASKS_SHFILE;
                cout <<"Bashfile: "<<bashfile <<endl;
                QString bashfileQ(bashfile.c_str());
                QStringList params;
                params<<bashfileQ;
                p.start("bash",params);
                p.waitForFinished(-1);
                QString output(p.readAllStandardOutput());
                cout << output.toStdString() <<endl;
                outputpath = NIFTYNETPATH + ABDOMINALSUBDIR + OUTPUT + organs_names[i] + ".nii";
                LoadImageFromPath(ds, outputpath);
                ds->GetNamedNode(organs_names[i])->SetName(node_name+" "+organs_names[i]);
            }
        }

        p.close();

    }
}

void Inference::InfereSegmentation(mitk::Image *input_image, QString node_name,int model_type, mitk::DataStorage::Pointer ds)//mitk::DataNode *output_node)
{

    std::string inputpath = NIFTYNETPATH + ABDOMINALSUBDIR +INPUT + node_name.toStdString() + INPUTNAME;
    SaveInput(input_image, inputpath);//,saving_path);
    std::string outputpath = NIFTYNETPATH + ABDOMINALSUBDIR + OUTPUT + node_name.toStdString() + OUTPUTNAME;
    if (exists(outputpath)!=1)
    {
        ProcessNiftynet(model_type);
        LoadImageFromPath(ds, outputpath);
    }
    else
    {
        cout<<"ya existe la segmentacion"<<endl;
    }
        //mitk::Image::Pointer output_image;
    //output_node->SetData(output_image);
    //output_node->SetName("SegmentacionIA");
    CreateSegmentationMasks(ds,node_name.toStdString());

    //ds->GetNamedNode(outputpath)->SetName("Todos");

    //CleanDir();


}

