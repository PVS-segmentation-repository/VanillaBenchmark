#include "FrangiFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <vector>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 14)
    {
        std::cerr << "Uso: " << argv[0] << " --input <archivo_entrada> --output <archivo_salida> --sigmaMin <min> --sigmaMax <max> --nbSigmaSteps <steps> --alpha <alpha> --beta <beta> --gamma <gamma>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string inputFilename, outputFilename;
    double sigmaMin = 0.5, sigmaMax = 2.0, alpha = 0.5, beta = 0.5, gamma = 5.0;
    int nbSigmaSteps = 4;

    // Extraer los argumentos de la línea de comandos
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--input")
        {
            inputFilename = argv[++i];
        }
        else if (std::string(argv[i]) == "--output")
        {
            outputFilename = argv[++i];
        }
        else if (std::string(argv[i]) == "--sigmaMin")
        {
            sigmaMin = std::stod(argv[++i]);
        }
        else if (std::string(argv[i]) == "--sigmaMax")
        {
            sigmaMax = std::stod(argv[++i]);
        }
        else if (std::string(argv[i]) == "--nbSigmaSteps")
        {
            nbSigmaSteps = std::stoi(argv[++i]);
        }
        else if (std::string(argv[i]) == "--alpha")
        {
            alpha = std::stod(argv[++i]);
        }
        else if (std::string(argv[i]) == "--beta")
        {
            beta = std::stod(argv[++i]);
        }
        else if (std::string(argv[i]) == "--gamma")
        {
            gamma = std::stod(argv[++i]);
        }
    }

    // Leer la imagen de entrada
    using ReaderType = itk::ImageFileReader<ImageType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputFilename);

    try {
        reader->Update();
    } catch (itk::ExceptionObject &error) {
        std::cerr << "Error al leer la imagen: " << error << std::endl;
        return EXIT_FAILURE;
    }

    // Crear la instancia del filtro Frangi con los parámetros extraídos de los argumentos
    FrangiFilter frangiFilter(sigmaMin, sigmaMax, nbSigmaSteps, alpha, beta, gamma);

    // Aplicar el filtro de Frangi
    ImageType::Pointer outputImage = frangiFilter.ApplyFrangiFilter(reader->GetOutput());

    // Guardar la imagen resultante
    using WriterType = itk::ImageFileWriter<ImageType>;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(outputFilename);
    writer->SetInput(outputImage);

    try {
        writer->Update();
    } catch (itk::ExceptionObject &error) {
        std::cerr << "Error al escribir la imagen: " << error << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
