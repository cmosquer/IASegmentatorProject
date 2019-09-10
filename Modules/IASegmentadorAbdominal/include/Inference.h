//Because vtkPlane is strange, we will return as output a PolyData and provide Normal and Origin accessors.


#ifndef __Inference_h
#define __Inference_h

#include <mitkImage.h>
#include <mitkDataNode.h>

#include <stdio.h>
#include <QProcess>
#include <mitkDataStorage.h>
#include <IASegmentadorAbdominalExports.h>
#include <mitkColorSequenceRainbow.h>
# include <mitkLookupTable.h>


class IASegmentadorAbdominal_EXPORT Inference{
     public:
        Inference();
       ~Inference();
        void InfereSegmentation(mitk::Image *input_image, QString node_name, int model_type,mitk::DataStorage::Pointer ds);// mitk::DataNode *output_node);
        std::vector<int> m_selected_labels = {0,0,0,0,0,0,0,0};
        std::vector<std::string> organs_names = { "Bazo", "Rinon", "Vesicula","Esofago", "Higado", "Estomago", "Pancreas", "Duodeno" };
        void CleanDir(std::string path);

    private:
        void SetLookUpTable();
        void SaveInput(mitk::Image *image, std::string inputpath);
        void ProcessNiftynet();
        std::string GetOrgansList();
        void InstallPackages();
        void LoadImageFromPath(mitk::DataStorage::Pointer ds, std::string outputpath);
        void GetSegmentationMasks(mitk::DataStorage::Pointer ds, std::string node_name);
        void SetPaths();
        void SetModelSubdir(int model_type);
        void CreateMasks(std::vector<int> missingmasks, std::string NiftyNetOutputPath,mitk::DataStorage::Pointer ds);
        std::string NIFTYNETPATH;
        std::string INPUT;
        std::string INPUTNAME;
        std::string OUTPUT;
        std::string OUTPUTNAME;
        std::string SEGMENTATION;
        std::string SHFILE;
        std::string MASKS_SHFILE;
        std::string MODELSUBDIR;
        std::vector<std::string> MODELNAMES;
        mitk::LookupTable::Pointer m_lutUS;


        mitk::ColorSequenceRainbow m_Rainbow;
};


#endif
