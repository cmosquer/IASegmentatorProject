

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
    CleanDir();
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
    SEGMENTATION = "/segmentations/";
    INPUTNAME = "_input.nii";
    OUTPUTNAME = "_input_niftynet_out.nii.gz";
    SHFILE = "inference.sh";
    MASKS_SHFILE = "masks.sh";
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
           organsList = organsList +  "-" + std::to_string(i);
        }
        }
    cout<<organsList<<endl;
    return organsList;
}

void Inference::ProcessNiftynet(){

    std::string bashfile = NIFTYNETPATH + MODELSUBDIR + SHFILE;
    QProcess p;
    QStringList params;

    //cout<<"bashfile: "<<bashfile<<endl;
    //QString bashfileQ(bashfile.c_str());
    //QString labelsList(GetOrgansList().c_str());
    //params<<"-i"<<bashfileQ;//<<labelsList;
    params<<"-i"<<bashfile.c_str()<<GetOrgansList().c_str();
    p.start("bash",params); // ,params);
    p.waitForFinished(-1);
    QString output(p.readAllStandardOutput());
    cout << output.toStdString() <<endl;
    p.close();

}

void Inference::CleanDir()
{

    QProcess p;

    QStringList params;
    std::string command = NIFTYNETPATH + MODELSUBDIR + OUTPUT + "*";
    params<<"rm"<<"-f"<<command.c_str();
    cout<<"Comando clean dir: "<<command<<endl;
    p.start("bash",params);
    p.waitForFinished(-1);
    QString output = p.readAllStandardOutput();

    p.close();

    QProcess p2;

    QStringList params2;
    std::string command2 = NIFTYNETPATH + MODELSUBDIR + INPUT + "*";
    params2<<"-i"<<"rm"<<"-f"<<command2.c_str();
    cout<<"Comando clean dir: "<<command2<<endl;
    p2.start("bash",params2);
    p2.waitForFinished(-1);

    output = p2.readAllStandardOutput();

    cout << output.toStdString() <<endl;

    p2.close();
    QProcess p3;

    QStringList params3;
    std::string command3 = NIFTYNETPATH + MODELSUBDIR + SEGMENTATION + "*";
    params3<<"rm"<<"-f"<<command3.c_str();
    cout<<"Comando clean dir: "<<command3<<endl;
    p3.start("bash",params3);
    p3.waitForFinished(-1);

    output = p3.readAllStandardOutput();

    cout << output.toStdString() <<endl;

    p2.close();


}
void Inference::LoadSegmentationMasks(mitk::DataStorage::Pointer ds, std::string node_name)
{


    bool executeMasks=false;
    std::string missingMasks="";
    std::string outputpath;
    std::vector<int> missingMasksIdx;
    for(std::vector<int>::size_type i = 0; i != m_selected_labels.size(); i++)
    {
        //cout<<organs_names[i]<< ". " << m_selected_labels[i]<<endl;

        if (m_selected_labels[i]==1)
        {
            mitk::DataNode::Pointer current_node = ds->GetNamedNode(node_name+" "+organs_names[i]);
            if (!current_node)
            {
                outputpath = NIFTYNETPATH + MODELSUBDIR + SEGMENTATION + organs_names[i] + ".mha";
                if(exists(outputpath))
                {
                    LoadImageFromPath(ds, outputpath);
                    ds->GetNamedNode(organs_names[i])->SetName(node_name+" "+organs_names[i]);
                }
                else
                {
                    cout<<"Falta la mascara de "<<organs_names[i]<<endl;
                    executeMasks = true;
                    missingMasks = missingMasks +  "-" + std::to_string(i);
                    missingMasksIdx.push_back(i);
                }

            }

        }
    }
    if (executeMasks)
    {
        QProcess p;

        std::string bashfile = NIFTYNETPATH + MODELSUBDIR + MASKS_SHFILE;
        cout <<"Bashfile: "<<bashfile <<endl;
        QString bashfileQ(bashfile.c_str());
        QStringList params;
        cout<<"Missing masks: "<<missingMasks<<endl;
        params<<"-i"<<bashfile.c_str()<<missingMasks.c_str();
        p.start("bash",params);
        p.waitForFinished(-1);
        QString output(p.readAllStandardOutput());
        cout << output.toStdString() <<endl;
        p.close();

        for(std::vector<int>::size_type j = 0; j != missingMasksIdx.size(); j++)
        {
            outputpath = NIFTYNETPATH + MODELSUBDIR + SEGMENTATION + organs_names[missingMasksIdx[j]] + ".mha";
            cout<<outputpath<<endl;
            if(exists(outputpath))
            {
                cout<<"Cargando la mascara de "<<organs_names[missingMasksIdx[j]]<<endl;
                LoadImageFromPath(ds, outputpath);
                ds->GetNamedNode(organs_names[missingMasksIdx[j]])->SetName(node_name+" "+organs_names[missingMasksIdx[j]]);
            }

         }

      }

}



void Inference::SetModelSubdir(int model_type)
{

    MODELSUBDIR = MODELNAMES[model_type] + "/";
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
        LoadSegmentationMasks(ds,node_name.toStdString());
    }
    else
    {
        cout<<"ya existe la segmentacion"<<endl;
        LoadSegmentationMasks(ds,node_name.toStdString());

    }
        //mitk::Image::Pointer output_image;
    //output_node->SetData(output_image);
    //output_node->SetName("SegmentacionIA");


    //ds->GetNamedNode(outputpath)->SetName("Todos");

    //


}

