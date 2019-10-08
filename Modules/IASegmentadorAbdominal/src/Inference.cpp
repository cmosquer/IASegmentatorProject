

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
    MASKS_SHFILE = "masks.sh";
    MODELNAMES = {"abdominal"};

}

void Inference::SetLookUpTable()
{
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();

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

        if (m_selected_labels[i]==1)        {
           organsList = organsList +  "-" + std::to_string(i);
        }
        }
    return organsList;
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
    cout<<"NiftyPath: "<<NiftyNetOutputPath<<endl;
    constexpr unsigned int Dimension = 3;

    //using PixelType = unsigned char;

    //using ImageType = itk::Image< PixelType, Dimension >;
    //using OutputImageType = itk::Image<unsigned short, Dimension>;


    using PixelType = unsigned char;
    using LabelType = unsigned short;
    using ImageType = itk::Image<PixelType, Dimension>;
    //using OutputImageType = itk::Image<LabelType, Dimension>;
    //using ShapeLabelObjectType = itk::ShapeLabelObject<LabelType, Dimension>;
    //using ShapeLabelMapType = itk::LabelMap<ShapeLabelObjectType>;
    using LabelObjectType = itk::LabelObject< LabelType, Dimension >;
    using LabelMapType = itk::LabelMap< LabelObjectType >;


    //1)REader
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


    //7) Shaper: OutputImageType a ShapeLabelMap
    //using I2LType = itk::LabelImageToShapeLabelMapFilter<OutputImageType, ShapeLabelMapType>;
    //I2LType::Pointer image2ShapeLabel = I2LType::New();

    //Instanciar el selector de region mas grande
    /*ESTO DA ERROR:
    using LabelShapeKeepNObjectsImageFilterType = itk::LabelShapeKeepNObjectsImageFilter<OutputImageType>;
    LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter =
      LabelShapeKeepNObjectsImageFilterType::New();

    labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
    labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
    labelShapeKeepNObjectsImageFilter->SetAttribute(
      LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);*/


    /*ESTO DA ERROR:

    using BinaryImageToShapeLabelMapFilterType = itk::BinaryImageToShapeLabelMapFilter<ImageType>;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter =
      BinaryImageToShapeLabelMapFilterType::New();

     ESTO NO DA ERROR:
    using RescaleFilterType = itk::RescaleIntensityImageFilter<OutputImageType, ImageType>;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(itk::NumericTraits<PixelType>::max());
    */

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
        labelMapConverter2->SetInput(connected->GetOutput(0));
        labelMapConverter2->Update();
        LabelMapType *labelMap = labelMapConverter2->GetOutput();
        // Retrieve all attributes
        unsigned int maximumPixels = 0;
        for (unsigned int n = 0; n < labelMap->GetNumberOfLabelObjects(); ++n)
        {
          LabelObjectType * labelObject = labelMap->GetNthLabelObject(n);
          const unsigned int pixels = labelObject->Size();
          cout << "Label: " << itk::NumericTraits<LabelMapType::LabelType>::PrintType(labelObject->GetLabel())
                    << "Pixels: " << pixels << endl;
          if (pixels<maximumPixels)
          {
              labelMap->RemoveLabelObject(labelObject);
              cout<<"Se elimina este objeto"<<endl;
          }
          else {
              maximumPixels = pixels;
          }

        }
        label2image->SetInput(labelMap);
        label2image->Update();
        //Distingo las regiones conectadas
        /*connected->SetInput(label2image->GetOutput());
        connected->Update();
        image2ShapeLabel->SetInput(connected->GetOutput());
        image2ShapeLabel->SetComputePerimeter(true);
        image2ShapeLabel->Update();

        ShapeLabelMapType * labelMap = image2ShapeLabel->GetOutput();
        cout <<" has " << labelMap->GetNumberOfLabelObjects() << " labels." << std::endl;

        // Retrieve all attributes
        for (unsigned int n = 0; n < labelMap->GetNumberOfLabelObjects(); ++n)
        {
          ShapeLabelObjectType * labelObject = labelMap->GetNthLabelObject(n);
          std::cout << "Label: " << itk::NumericTraits<LabelMapType::LabelType>::PrintType(labelObject->GetLabel())
                    << std::endl;
          std::cout << "    BoundingBox: " << labelObject->GetBoundingBox() << std::endl;
          std::cout << "    NumberOfPixels: " << labelObject->GetNumberOfPixels() << std::endl;
          std::cout << "    PhysicalSize: " << labelObject->GetPhysicalSize() << std::endl;
          std::cout << "    Centroid: " << labelObject->GetCentroid() << std::endl;
          std::cout << "    NumberOfPixelsOnBorder: " << labelObject->GetNumberOfPixelsOnBorder() << std::endl;
          std::cout << "    PerimeterOnBorder: " << labelObject->GetPerimeterOnBorder() << std::endl;
          std::cout << "    FeretDiameter: " << labelObject->GetFeretDiameter() << std::endl;
          std::cout << "    PrincipalMoments: " << labelObject->GetPrincipalMoments() << std::endl;
          std::cout << "    PrincipalAxes: " << labelObject->GetPrincipalAxes() << std::endl;
          std::cout << "    Elongation: " << labelObject->GetElongation() << std::endl;
          std::cout << "    Perimeter: " << labelObject->GetPerimeter() << std::endl;
          std::cout << "    Roundness: " << labelObject->GetRoundness() << std::endl;
          std::cout << "    EquivalentSphericalRadius: " << labelObject->GetEquivalentSphericalRadius() << std::endl;
          std::cout << "    EquivalentSphericalPerimeter: " << labelObject->GetEquivalentSphericalPerimeter() << std::endl;
          std::cout << "    EquivalentEllipsoidDiameter: " << labelObject->GetEquivalentEllipsoidDiameter() << std::endl;
          std::cout << "    Flatness: " << labelObject->GetFlatness() << std::endl;
          std::cout << "    PerimeterOnBorderRatio: " << labelObject->GetPerimeterOnBorderRatio() << std::endl;
        }*/
        //Me quedo solo con la region más grande
        //labelShapeKeepNObjectsImageFilter->SetInput(connected->GetOutput());
        //labelShapeKeepNObjectsImageFilter->Update();


        //rescaleFilter->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());
        /*
        binaryImageToShapeLabelMapFilter->SetInput(label2image->GetOutput());
        binaryImageToShapeLabelMapFilter->Update();

        // The output of this filter is an itk::ShapeLabelMap, which contains itk::ShapeLabelObject's
        cout << "There are " << binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << " objects."
                  << endl;

        for (unsigned int i = 0; i < binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
        {
          BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType * labelObject =
            binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
          // Output the bounding box (an example of one possible property) of the ith region
            cout << "Object " << i << " has pixels " << labelObject->GetNumberOfPixels() << endl;

        }_*/

        mitk::Image::Pointer m_ResultImage = mitk::Image::New();
        mitk::CastToMitkImage(label2image->GetOutput(), m_ResultImage);
        mitk::DataNode::Pointer m_Node = mitk::DataNode::New();
        m_Node->SetData(m_ResultImage);
        m_Node->SetBoolProperty("binary",false);
        m_Node->SetColor(m_Rainbow.GetNextColor());
        mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
        mitkLutProp->SetLookupTable(m_lutUS);
        m_Node->SetProperty( "LookupTable", mitkLutProp );
        m_Node->SetProperty( "Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));



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
                    mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
                    mitkLutProp->SetLookupTable(m_lutUS);
                    ds->GetNamedNode(organs_names[i])->SetProperty( "LookupTable", mitkLutProp );
                    ds->GetNamedNode(organs_names[i])->SetProperty( "Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));


                    ds->GetNamedNode(organs_names[i])->SetColor(m_Rainbow.GetNextColor());
                    ds->GetNamedNode(organs_names[i])->SetBoolProperty("binary",false);
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
void Inference::InfereSegmentation(mitk::Image *input_image, QString node_name,int model_type, mitk::DataStorage::Pointer ds)//mitk::DataNode *output_node)
{
    cout<<"Inicia segmentacion"<<endl;
    SetModelSubdir(model_type);
    std::string inputpath = NIFTYNETPATH + MODELSUBDIR +INPUT + node_name.toStdString() + INPUTNAME;
    SaveInput(input_image, inputpath);
    std::string outputpath = NIFTYNETPATH + MODELSUBDIR + OUTPUT + node_name.toStdString() + OUTPUTNAME;
    if (exists(outputpath)!=1)
    {
        ProcessNiftynet();
        GetSegmentationMasks(ds,node_name.toStdString());
    }
    else
    {
        cout<<"Ya existe la segmentacion de Niftynet en Outputs"<<endl;
        GetSegmentationMasks(ds,node_name.toStdString());


    }


}

