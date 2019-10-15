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

        void InfereSegmentation(std::string node_name, int model_type, mitk::DataStorage::Pointer ds);
        /*Metodo para invocar desde un plugin y generar una inferencia de IA con el modelo model_type sobre la imagen
         * que figura como node_name en el DataStorage ds
            node_name: contiene data casteable a MITK image. Es la imagen que será segmentada.
            model_type: determina qué modelo IA utilizar en la segmentacion. Disponibles al momento:
                0 --> modelo de segmentacion abdominal
            ds: datastorage de MITK
        */

        Inference();
        /*El constructor va a setear las "variables de directorio" (ver mas abajo): cada tipo de modelo disponible deberá tener una subcarpeta en
         * la carpeta NiftyNet del Module. Dentro de esta subcarpeta habrá: una carpeta de inputs donde se guarda temporalmente las imagenes originales,
         * una carpeta de outputs de niftynet, y carpeta de segmentaciones donde se guardan las máscaras obtenidas por separado. Además deberá haber un archivo
         *  sh que ejecuta Python para la segmentacion. */
       ~Inference();
        /*El destructor se encarga de limpiar cada carpeta dentro de la subcarpeta del tipo de modelo utilizado. Sucede cuando se cierra el plugin.*/

        void CleanDir(std::string path);
        /* Elimina los archivos generados en esa corrida de plugin. */

        //---------------VARIABLES PROPIAS DE MODELO ABDOMINAL------------------------------//:

        std::vector<int> m_selected_labels = {0,0,0,0,0,0,0,0};
        //Indica qué órganos se pidió segmentar. Debe ser modificada desde el plugin.

        std::vector<std::string> organs_names = { "Bazo", "Rinon", "Vesicula","Esofago", "Higado", "Estomago", "Pancreas", "Duodeno" };
        //Nombres en español de los órganos. El plugin debe levantar de acá los nombres para los labels de frontend
        //----------------------------------------------------------------------------------//

    private:
        //DOS METODOS LLAMADOS POR EL CONSTRUCTOR:
        void SetLookUpTable();
        void SetPaths();
        //--------------------------------------//

        void SetModelSubdir(int model_type);
        //Inicializa la tabla de colores para que cada máscara tenga un color distinto en MITK

        void SaveInput(mitk::Image *image, std::string inputpath);
        //El primer paso de una segmentacion será convertir la imagen a formato Nifti y guardarla temporalmente en la carpeta de inputs.

        void ProcessNiftynet();       
        //Ejecuta el archivo sh que llama a niftynet con interprete Python. Guarda el nifti segmentado en la carpeta de outputs.

        void GetSegmentationMasks(mitk::DataStorage::Pointer ds, std::string node_name);
        //Gestiona la segmentacion de mascaras: cuáles fueron solicitadas, cuáles ya existen y cuáles deben construirse. Las guarda en la carpeta de segmentations

        void CreateMasks(std::vector<int> missingmasks, std::string NiftyNetOutputPath,mitk::DataStorage::Pointer ds);
        //Crea las máscaras a partir del nifty output de niftynet.

        void LoadImageFromPath(mitk::DataStorage::Pointer ds, std::string outputpath);
        //Cargar en el datastorage las mascaras generadas.


        //----------------VARIABLES DE DIRECTORIO----------------------//
        std::string NIFTYNETPATH;
        std::string INPUT;
        std::string INPUTNAME;
        std::string OUTPUT;
        std::string OUTPUTNAME;
        std::string SEGMENTATION;
        std::string SHFILE;
        std::string MODELSUBDIR;
        std::vector<std::string> MODELNAMES;
        //------------------------------------------------------------//

        mitk::LookupTable::Pointer m_lutUS;
        mitk::ColorSequenceRainbow m_Rainbow;
};


#endif
