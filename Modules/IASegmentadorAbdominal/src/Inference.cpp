

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
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkImageFileWriter.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkImageFileReader.h>
#include <itkLabelSelectionLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkShapeOpeningLabelMapFilter.h"
#include "itkLabelMapToLabelImageFilter.h"
#include "itkScalarToRGBColormapImageFilter.h"
#include <itkConnectedComponentImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkLabelShapeKeepNObjectsImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>

#include <mitkImageCast.h>
#include <mitkColorSequenceRainbow.h>
#include <mitkLookupTableProperty.h>
#include <mitkRenderingModeProperty.h>
#include <QMessageBox>
#include <QDir>
#include <QString>
#include <string>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32
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
    SetLookUpTable();
}
Inference::~Inference()
{

    CleanDir(NIFTYNETPATH + MODELSUBDIR +INPUT);
    CleanDir(NIFTYNETPATH + MODELSUBDIR +OUTPUT);
    CleanDir(NIFTYNETPATH + MODELSUBDIR +SEGMENTATION);


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
    OUTPUT = "outputs/";
    SEGMENTATION = "/segmentations/";
    INPUTNAME = "_input.nii.gz";
    OUTPUTNAME = "_input_niftynet_out.nii.gz";
    SHFILE = "inference.sh";
    MODELNAMES = {"abdominal"};

}

void Inference::SetLookUpTable()
{
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    /*
    lookupTable->SetNumberOfTableValues(9);
    lookupTable->SetTableValue(1, 255.0, 255.0, 0.0, 1.);
    lookupTable->SetTableValue(2, 255.0, 0.0, 255.0, 1.);
    lookupTable->SetTableValue(3, 0.0, 255.0, 255.0, 1.);
    lookupTable->SetTableValue(4, 255.0, 0.0, 0.0, 1.);
    lookupTable->SetTableValue(5, 0.0, 255.0, 0.0, 1.);
    lookupTable->SetTableValue(6, 0.0, 0.0, 255.0, 1.);
    lookupTable->SetTableValue(7, 255.0, 100.0, 100.0, 1.);
    lookupTable->SetTableValue(8, 255.0, 255.0, 100.0, 1.);

    lookupTable->SetTableValue(0, 0.,0.,0.,0.);
    lookupTable->Build();*/
    lookupTable->SetNumberOfTableValues(2);
    lookupTable->SetRange(0.0,1.0);
    lookupTable->SetTableValue( 0, 0.0, 0.0, 0.0, 0.0 ); //label 0 is transparent
    lookupTable->SetTableValue( 1, 1.0, 1.0, 0.0, 1.0 );

    // generate mitk lookup table


    m_lutUS = mitk::LookupTable::New(); // mitk US lookup table for black transparency

    m_lutUS->SetVtkLookupTable(lookupTable);


}

void Inference::SaveInput(mitk::Image *image, std::string inputpath)
{
    if (exists(inputpath)!=1)
    {
        mitk::IOUtil::Save(image, inputpath);
    }
    else
    {
        cout<<"Ya existe la imagen Nifti en Inputs"<<endl;
    }
}

void Inference::LoadImageFromPath(mitk::DataStorage::Pointer ds, std::string outputpath)
{
    mitk::IOUtil::Load(outputpath, *ds);
}


void Inference::ProcessNiftynet(){

    std::string bashfile = NIFTYNETPATH + MODELSUBDIR + SHFILE;
    QProcess p;
    QStringList params;

    cout<<"INICIA NIFTYNET"<<endl;
    params<<"-i"<<bashfile.c_str();
    p.start("bash",params); // ,params);
    p.waitForFinished(-1);
    QString output(p.readAllStandardOutput());
    cout << output.toStdString() <<endl;
    p.close();

}

void Inference::CleanDir(std::string path)
{

    QDir dir(path.c_str());
    dir.setNameFilters(QStringList() << "*.*");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }

}

void Inference::CreateMasks(std::vector<int> missingmasks, std::string node_name, mitk::DataStorage::Pointer ds)
{
    std::string NiftyNetOutputPath =  NIFTYNETPATH + MODELSUBDIR + OUTPUT + node_name + OUTPUTNAME;

    constexpr unsigned int Dimension = 3;
    using PixelType = unsigned char;
    using LabelType = unsigned short;
    using ImageType = itk::Image<PixelType, Dimension>;
    using LabelObjectType = itk::LabelObject< LabelType, Dimension >;
    using LabelMapType = itk::LabelMap< LabelObjectType >;


    //1)Reader
    using ReaderType = itk::ImageFileReader< ImageType >;
    ReaderType::Pointer reader1 = ReaderType::New();
    reader1->SetFileName(NiftyNetOutputPath);

    //2) ImageType a LabelMap
    using LabelImageToLabelMapFilterType = itk::LabelImageToLabelMapFilter< ImageType, LabelMapType >;
    LabelImageToLabelMapFilterType::Pointer labelMapConverter = LabelImageToLabelMapFilterType::New();
    labelMapConverter->SetInput( reader1->GetOutput() );
    labelMapConverter->SetBackgroundValue( itk::NumericTraits< PixelType >::Zero );
    //3) Selector: LabelMap a LabelMap
    using SelectorType = itk::LabelSelectionLabelMapFilter< LabelMapType >;
    SelectorType::Pointer selector = SelectorType::New();
    selector->SetInput( labelMapConverter->GetOutput() );

    //4) LabelMap a ImageType
    typedef itk::LabelMapToLabelImageFilter<LabelMapType, ImageType> LabelMap2ImageType;
    LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();


    //5) Calculador min y max: ImageType a scalar
    using ImageCalculatorFilterType = itk::MinimumMaximumImageCalculator <ImageType>;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter
            = ImageCalculatorFilterType::New ();

    //6) ConnectedRegion: ImageType a OutputImageType
    using ConnectedComponentImageFilterType = itk::ConnectedComponentImageFilter<ImageType, ImageType>;
    ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();

    //7) ImageType a LabelMap
    LabelImageToLabelMapFilterType::Pointer labelMapConverter2 = LabelImageToLabelMapFilterType::New();

    //8) Writer
    using WriterType = itk::ImageFileWriter<ImageType>;
    WriterType::Pointer writer = WriterType::New();


    std::string mess = "El algoritmo no logró segmentar el órgano \n";
    bool messFlag = false;

    for(std::vector<int>::size_type j = 0; j != missingmasks.size(); j++)
    {

        int l = missingmasks[j];
        cout<<"Procesando máscara de "<<organs_names[l]<<endl;
        const auto label = static_cast< PixelType >(l+1);

        //Selecciono solo los pixeles de la imagen correspondiente al organo j
        selector->SetLabel(label);

        label2image->SetInput(selector->GetOutput(0));
        label2image->Update();
        imageCalculatorFilter->SetImage(label2image->GetOutput());
        imageCalculatorFilter->Compute();
        if(imageCalculatorFilter->GetMaximum() ==imageCalculatorFilter->GetMinimum())
        {
            cout<<"vacia"<<endl;
            mess = mess + organs_names[l] + " \n";
            messFlag = true;
            continue;
        }
        connected->SetInput(label2image->GetOutput());
        labelMapConverter2->SetBackgroundValue( itk::NumericTraits< PixelType >::Zero );

        labelMapConverter2->SetInput(connected->GetOutput(0));
        labelMapConverter2->Update();
        LabelMapType *labelMap = labelMapConverter2->GetOutput();
        unsigned long maximumPixels = 0;
        unsigned int maximumLabel;
        for (unsigned int n = 0; n < labelMap->GetNumberOfLabelObjects(); ++n)
        {
          LabelObjectType *labelObject = labelMap->GetNthLabelObject(n);
          const unsigned long pixels = labelObject->Size();
          //cout << "Label: " << itk::NumericTraits<LabelMapType::LabelType>::PrintType(labelObject->GetLabel())
          //          << ". Pixels: " << pixels << endl;
          //cout<<"n: "<<n<<endl;
          if (pixels>maximumPixels)
          {
              maximumPixels = pixels;
              maximumLabel = labelObject->GetLabel();
          }

        }
        //cout<<"El maximo es "<<maximumLabel<<" con "<<maximumPixels<<" pixels."<<endl;
        LabelMapType::SizeValueType N_total = labelMap->GetNumberOfLabelObjects();
        LabelMapType::SizeValueType new_n = 0;
        for (unsigned int n = 0; n < N_total; ++n)
        {

          LabelObjectType *labelObject = labelMap->GetNthLabelObject(new_n);
          //cout<<"n: "<<n<<endl;

          if (labelObject->Size()<maximumPixels)
          {
              //cout<<"Se elimina el label "<<itk::NumericTraits<LabelMapType::LabelType>::PrintType(labelObject->GetLabel())<<endl;
              labelMap->RemoveLabelObject(labelObject);


          }
          else
          {
            new_n++;
          }

        }

        label2image->SetInput(labelMap);
        label2image->Update();

        mitk::Image::Pointer m_ResultImage = mitk::Image::New();
        mitk::CastToMitkImage(label2image->GetOutput(), m_ResultImage);
        mitk::DataNode::Pointer m_Node = mitk::DataNode::New();
        m_Node->SetData(m_ResultImage);
        m_Node->SetColor(m_Rainbow.GetNextColor());

        mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
        mitkLutProp->SetLookupTable(m_lutUS);
        m_Node->SetProperty( "LookupTable", mitkLutProp );
        m_Node->SetProperty( "Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));
        m_Node->SetBoolProperty("binary",true);
        m_Node->SetOpacity(1.0);
        m_Node->SetName(node_name+" "+organs_names[l]);
        ds->Add(m_Node);

        //Guardar como archivo la imagen ITK
        std::string outputpath = NIFTYNETPATH + MODELSUBDIR + SEGMENTATION + organs_names[l] + ".mha";
        writer->SetFileName(outputpath);
        writer->SetInput(label2image->GetOutput());
        writer->Update();


    }
    if (messFlag)
        QMessageBox::information(nullptr, "Template", QString(mess.c_str()));
}

void Inference::GetSegmentationMasks(mitk::DataStorage::Pointer ds, std::string node_name)
{
    bool executeMasks=false;
    std::string outputpath;
    std::vector<int> missingMasksIdx;
    for(std::vector<int>::size_type i = 0; i != m_selected_labels.size(); i++)
    {

        if (m_selected_labels[i]==1)
        {
            mitk::DataNode::Pointer current_node = ds->GetNamedNode(node_name+" "+organs_names[i]);
            if (!current_node)
            {
                outputpath = NIFTYNETPATH + MODELSUBDIR + SEGMENTATION + organs_names[i] + ".mha";
                if(exists(outputpath))
                {
                    LoadImageFromPath(ds, outputpath);
                    ds->GetNamedNode(organs_names[i])->SetColor(m_Rainbow.GetNextColor());
                    mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
                    mitkLutProp->SetLookupTable(m_lutUS);
                    ds->GetNamedNode(organs_names[i])->SetProperty( "LookupTable", mitkLutProp );
                    ds->GetNamedNode(organs_names[i])->SetProperty( "Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));
                    //mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
                    //mitkLutProp->SetLookupTable(m_lutUS);
                    //ds->GetNamedNode(organs_names[i])->SetProperty( "LookupTable", mitkLutProp );
                    //ds->GetNamedNode(organs_names[i])->SetProperty( "Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));



                    ds->GetNamedNode(organs_names[i])->SetBoolProperty("binary",true);
                    ds->GetNamedNode(organs_names[i])->SetName(node_name+" "+organs_names[i]);
                }
                else
                {
                    cout<<"No existe mascara de "<<organs_names[i]<<endl;
                    executeMasks = true;
                    missingMasksIdx.push_back(i);
                }

            }

        }
    }
    if (executeMasks)
    {
        CreateMasks(missingMasksIdx,node_name,ds);
    }

}

void Inference::SetModelSubdir(int model_type)
{

    MODELSUBDIR = MODELNAMES[model_type] + "/";
}
void Inference::InfereSegmentation(std::string node_name,int model_type, mitk::DataStorage::Pointer ds)
{

    mitk::Image *input_image = dynamic_cast<mitk::Image *>(ds->GetNamedNode(node_name)->GetData());
    cout<<"Inicia segmentacion"<<endl;
    SetModelSubdir(model_type);
    std::string inputpath = NIFTYNETPATH + MODELSUBDIR +INPUT + node_name + INPUTNAME;
    SaveInput(input_image, inputpath);
    std::string outputpath = NIFTYNETPATH + MODELSUBDIR + OUTPUT + node_name + OUTPUTNAME;
    if (exists(outputpath)!=1)
    {
        ProcessNiftynet();
        GetSegmentationMasks(ds,node_name);
    }
    else
    {
        cout<<"Ya existe la segmentacion de Niftynet en Outputs"<<endl;
        GetSegmentationMasks(ds,node_name);


    }


}

