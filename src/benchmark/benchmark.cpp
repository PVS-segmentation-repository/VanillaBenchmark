#include "FrangiFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>

// Funciones para leer imágenes
using ImageType = itk::Image<float, 3>; // Ajustar el tipo de imagen según corresponda

ImageType::Pointer LeerImagen(const std::string& archivo) {
    using ReaderType = itk::ImageFileReader<ImageType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(archivo);
    reader->Update();
    return reader->GetOutput();
}

// Funciones para escribir imágenes
void GuardarImagen(const ImageType::Pointer& imagen, const std::string& nombreArchivo) {
    using WriterType = itk::ImageFileWriter<ImageType>;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(nombreArchivo);
    writer->SetInput(imagen);
    writer->Update();
}

// Funciones para calcular las métricas
void CalcularMetricas(const ImageType::Pointer& result, const ImageType::Pointer& referencia, 
                      int& TP, int& TN, int& FP, int& FN, double umbral) {
    itk::ImageRegionConstIterator<ImageType> itResult(result, result->GetRequestedRegion());
    itk::ImageRegionConstIterator<ImageType> itRef(referencia, referencia->GetRequestedRegion());

    TP = TN = FP = FN = 0;

    for (itResult.GoToBegin(), itRef.GoToBegin(); !itResult.IsAtEnd(); ++itResult, ++itRef) {
        bool valorResult = itResult.Get() > umbral;  // Umbral dinámico para binarizar
        bool valorRef = itRef.Get() > 0.5;

        if (valorResult && valorRef) TP++;
        else if (!valorResult && !valorRef) TN++;
        else if (valorResult && !valorRef) FP++;
        else if (!valorResult && valorRef) FN++;
    }
}
// Estructura para los parámetros de Frangi
struct FrangiParams {
    double sigmaMin;
    double sigmaMax;
    int nbSigmaSteps;
    double alpha;
    double beta;
    double gamma;
};
// Función para leer los parámetros desde un archivo
std::vector<FrangiParams> LeerParametrosFrangi(const std::string& archivoParametros) {
    std::vector<FrangiParams> parametros;
    std::ifstream archivo(archivoParametros);
    if (!archivo.is_open()) {
        throw std::runtime_error("Could not open the parameters file.");
    }

    FrangiParams params;
    while (archivo >> params.sigmaMin >> params.sigmaMax >> params.nbSigmaSteps 
                  >> params.alpha >> params.beta >> params.gamma) {
        parametros.push_back(params);
    }
    return parametros;
}

// Clase abstracta para los filtros
class Filtro {
public:
    virtual ImageType::Pointer AplicarFiltro(const ImageType::Pointer& inputImage) = 0;
};

// Clase para el filtro Frangi
class FiltroFrangi : public Filtro {
private:
    FrangiParams params;
    
public:
    FiltroFrangi(const FrangiParams& parametros) : params(parametros) {}

    ImageType::Pointer AplicarFiltro(const ImageType::Pointer& inputImage) override {
        FrangiFilter frangiFilter(params.sigmaMin, params.sigmaMax, params.nbSigmaSteps, 
                                  params.alpha, params.beta, params.gamma);
        return frangiFilter.ApplyFrangiFilter(inputImage);
    }
};

// Función que devuelve un puntero a la clase Filtro correspondiente
std::unique_ptr<Filtro> CrearFiltro(const std::string& nombreFiltro, const FrangiParams& params) {
    if (nombreFiltro == "Frangi") {
        return std::make_unique<FiltroFrangi>(params);
    } else {
        throw std::invalid_argument("Filter not recognized: " + nombreFiltro);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Use: " << argv[0] << " <image_list.txt> <frangi_parameters.txt> <filter_name> <num_thresholds>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string archivoLista = argv[1];
    std::string archivoParametrosFrangi = argv[2];
    std::string nombreFiltro = argv[3];
    int numThresholds = std::stoi(argv[4]);

    // Leer los parámetros del filtro Frangi desde el archivo
    std::vector<FrangiParams> parametrosFrangi;
    try {
        
        parametrosFrangi = LeerParametrosFrangi(archivoParametrosFrangi);
        
    } catch (const std::exception& e) {
        std::cerr << "Error reading Frangi parameters: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Abrir el archivo .txt que contiene las rutas de las imágenes
    std::ifstream archivo(archivoLista);
    if (!archivo.is_open()) {
        std::cerr << "Error opening file " << archivoLista << std::endl;
        return EXIT_FAILURE;
    }
    
    

    // Generar los thresholds distribuidos uniformemente entre 0 y 1
    std::vector<double> thresholds;
    for (int i = 0; i < numThresholds; ++i) {
        thresholds.push_back(static_cast<double>(i) / (numThresholds - 1)); // Umbrales de 0 a 1
    }

    std::string imagenOriginal, imagenReferencia;
    while (archivo >> imagenOriginal >> imagenReferencia) {
        std::cout << "Processing " << imagenOriginal << " with reference " << imagenReferencia << std::endl;

        // Leer la imagen original
        ImageType::Pointer inputImage = LeerImagen(imagenOriginal);
        ImageType::Pointer referencia = LeerImagen(imagenReferencia);

        // Obtener el directorio de la imagen original para guardar el CSV
        std::filesystem::path outputDirectory = std::filesystem::path(imagenOriginal).parent_path();
    
        // Ruta del archivo CSV de resultados
        std::filesystem::path outputPath = outputDirectory / "benchmark_results.csv";
        // Abrir el archivo CSV y escribir el encabezado si aún no se ha hecho
        std::ofstream fileResults(outputPath, std::ios::app);
        if (fileResults.tellp() == 0) {
            fileResults<< "Original_Image,Reference_Image,SigmaMin,SigmaMax,NbSigmaSteps,Alpha,Beta,Gamma,Threshold,TP,TN,FP,FN,Precision,Sensitivity\n";
        }
        
        

        // Aplicar cada conjunto de parámetros del filtro Frangi
        for (const auto& params : parametrosFrangi) {
            

            std::cout << "Applying Frangi with parameters: "
                      << "sigmaMin=" << params.sigmaMin << ", sigmaMax=" << params.sigmaMax
                      << ", nbSigmaSteps=" << params.nbSigmaSteps << ", alpha=" << params.alpha
                      << ", beta=" << params.beta << ", gamma=" << params.gamma << std::endl;

            // Crear el filtro Frangi con los parámetros actuales
            FiltroFrangi filtro(params);
    
            // Aplicar el filtro Frangi a la imagen original
            ImageType::Pointer outputImage = filtro.AplicarFiltro(inputImage);

            // Acumuladores para los resultados después de aplicar todos los umbrales
            ImageType::Pointer imagenFinal = outputImage; // Aquí puedes ajustar si necesitas postprocesamiento


            // Calcular y guardar métricas para cada umbral
            for (const double& threshold : thresholds) {
                int TP, TN, FP, FN;
                CalcularMetricas(outputImage, referencia, TP, TN, FP, FN, threshold);

                double precision = static_cast<double>(TP) / (TP + FP);
                double sensibilidad = static_cast<double>(TP) / (TP + FN);

               std::cout << "Threshold: " << threshold << " | TP: " << TP << " | TN: " << TN 
                          << " | FP: " << FP << " | FN: " << FN 
                          << " | Precision: " << precision << " | Sensitivity: " << sensibilidad << std::endl;

                // Guardar los resultados en el archivo CSV
                fileResults << imagenOriginal << "," << imagenReferencia << ","
                                  << params.sigmaMin << "," << params.sigmaMax << ","
                                  << params.nbSigmaSteps << "," << params.alpha << ","
                                  << params.beta << "," << params.gamma << ","
                                  << threshold << "," << TP << "," << TN << "," << FP << "," << FN << ","
                                  << precision << "," << sensibilidad << "\n";
            }
            // Después de aplicar todos los umbrales, guardar la imagen final resultante
            std::stringstream nombreArchivo;
                nombreArchivo << outputDirectory.string() << "/frangi_sigmaMin" << params.sigmaMin
                  << "_sigmaMax" << params.sigmaMax << "_alpha" << params.alpha
                  << "_beta" << params.beta << "_gamma" << params.gamma
                  << "_final.nii.gz";
    
            GuardarImagen(imagenFinal, nombreArchivo.str());
        }
    }

    return EXIT_SUCCESS;
}

