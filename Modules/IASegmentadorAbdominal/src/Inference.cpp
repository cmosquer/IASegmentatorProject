

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
#include <string>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32 || _WIN64
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif


typedef itksys::SystemTools ist;

Inference::Inference()
{
   SetPaths();
}
Inference::~Inference()
{

}



inline bool exists (const std::string& filename) {
  struct stat buffer;
  return (stat (filename.c_str(), &buffer) == 0);
}


void Inference::SetPaths()
{
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
        {
        cout<< errno <<endl;
        }
    std::string s = cCurrentPath;
    std::string delimiter = "/";
    NIFTYNETPATH = s.substr(0, s.find_last_of(delimiter)) + "/Modules/IASegmentadorAbdominal/NiftyNet/";

    INPUT = "/inputs/";
    OUTPUT = "/outputs/";
    INPUTNAME = "_input.nii";
    OUTPUTNAME = "_input_niftynet_out.nii.gz";
    SHFILE = "/inference.sh";

    MODELNAMES = {"abdominal"};
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
    std::string bashfile = NIFTYNETPATH + MODELSUBDIR + SHFILE;
    QString bashfileQ(bashfile.c_str());
    params<<bashfileQ;
    p.start("bash",params);
    p.waitForFinished(-1);
    QString output(p.readAllStandardOutput());
    cout << output.toStdString() <<endl;
    p.close();

}

std::string Inference::GetOrgansList()
{
    std::string organsList = "";
    for(std::vector<int>::size_type i = 0; i != m_selected_labels.size(); i++) {
        //cout<<organs_names[i]<< ". " << m_selected_labels[i]<<endl;

        if (m_selected_labels[i]==1)        {
           organsList = organsList +  "-" + char(i);
        }
        }
    return organsList;
}

void Inference::ProcessNiftynet(){


    QProcess p;
    QStringList params;
    std::string bashfile = NIFTYNETPATH + MODELSUBDIR + SHFILE;
    cout<<"bashfile: "<<bashfile<<endl;
    QString bashfileQ(bashfile.c_str());
    QString labelsList(GetOrgansList().c_str());
    params<<bashfileQ<<labelsList;
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
    std::string command = "rm -f " + NIFTYNETPATH + MODELSUBDIR + OUTPUT + "*";
    QString commandQ(command.c_str());
    params<<commandQ;
    cout<<"Comando clean dir: "<<command<<endl;
    p.start(commandQ);
    p.waitForFinished(-1);

    QStringList params2;
    std::string command2 = "-f " + NIFTYNETPATH + MODELSUBDIR + INPUT + "*";
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
                /*
                std::string bashfile = NIFTYNETPATH + ABDOMINALSUBDIR + organs_names[i] + MASKS_SHFILE;
                cout <<"Bashfile: "<<bashfile <<endl;
                QString bashfileQ(bashfile.c_str());
                QStringList params;
                params<<bashfileQ;
                p.start("bash",params);
                p.waitForFinished(-1);
                QString output(p.readAllStandardOutput());
                cout << output.toStdString() <<endl;
                */
                outputpath = NIFTYNETPATH + MODELSUBDIR + OUTPUT + organs_names[i] + ".nii";
                LoadImageFromPath(ds, outputpath);
                ds->GetNamedNode(organs_names[i])->SetName(node_name+" "+organs_names[i]);
            }
        }

        p.close();

    }
}

void Inference::SetModelSubdir(int model_type)
{

    MODELSUBDIR = "/" + MODELNAMES[model_type] + "/";
}
void Inference::InfereSegmentation(mitk::Image *input_image, QString node_name,int model_type, mitk::DataStorage::Pointer ds)//mitk::DataNode *output_node)
{
    SetModelSubdir(model_type);
    std::string inputpath = NIFTYNETPATH + MODELSUBDIR +INPUT + node_name.toStdString() + INPUTNAME;
    SaveInput(input_image, inputpath);
    std::string outputpath = NIFTYNETPATH + MODELSUBDIR + OUTPUT + node_name.toStdString() + OUTPUTNAME;
    if (exists(outputpath)!=1)
    {
        ProcessNiftynet();
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

