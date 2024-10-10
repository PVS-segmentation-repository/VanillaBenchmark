#include "FrangiFilter.h"
#include <cmath>
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkCastImageFilter.h"  // Incluido para el casting de tipo de imagen

FrangiFilter::FrangiFilter(double sigmaMin, double sigmaMax, int nbSigmaSteps, double alpha, double beta, double gamma)
    : m_SigmaMin(sigmaMin), m_SigmaMax(sigmaMax), m_NbSigmaSteps(nbSigmaSteps), m_Alpha(alpha), m_Beta(beta), m_Gamma(gamma) {}

ImageType::Pointer FrangiFilter::ApplyFrangiFilter(ImageType::Pointer inputImage)
{
    // Paso 1: Calcular la matriz Hessiana
    using HessianFilterType = itk::HessianRecursiveGaussianImageFilter<ImageType, HessianImageType>;
    HessianFilterType::Pointer hessianFilter = HessianFilterType::New();
    hessianFilter->SetInput(inputImage);
    
    // Crear una imagen para almacenar el resultado final
    using OutputImageType = itk::Image<float, Dimension>;  // Definir el tipo de imagen de salida como float
    OutputImageType::Pointer outputImage = OutputImageType::New();
    
    // Copiar información de la imagen original a la imagen de salida
    outputImage->SetRegions(inputImage->GetLargestPossibleRegion());
    outputImage->SetOrigin(inputImage->GetOrigin());
    outputImage->SetSpacing(inputImage->GetSpacing());
    outputImage->SetDirection(inputImage->GetDirection());
    
    // Reservar memoria para los píxeles en la imagen de salida
    outputImage->Allocate();
    outputImage->FillBuffer(0);  // Inicializar la imagen con ceros

    // Iterar sobre los thresholds (umbral mínimo y máximo)
    for (int i = 0; i < m_NbSigmaSteps; ++i)
    {
        double sigma = m_SigmaMin + i * ((m_SigmaMax - m_SigmaMin) / (m_NbSigmaSteps - 1));
        hessianFilter->SetSigma(sigma);
        hessianFilter->Update();
    
        HessianImageType::Pointer hessianImage = hessianFilter->GetOutput();

        // Paso 2: Calcular los valores propios
        using EigenValueArrayType = itk::FixedArray<double, Dimension>;
        using EigenValueImageType = itk::Image<EigenValueArrayType, Dimension>;
        using EigenAnalysisFilterType = itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType>;
    
        EigenAnalysisFilterType::Pointer eigenAnalysisFilter = EigenAnalysisFilterType::New();
        eigenAnalysisFilter->SetInput(hessianImage);
        eigenAnalysisFilter->Update();

        EigenValueImageType::Pointer eigenValuesImage = eigenAnalysisFilter->GetOutput();

        // Paso 3: Iterar sobre los píxeles y aplicar la medida de Frangi
        itk::ImageRegionIterator<EigenValueImageType> eigenIterator(eigenValuesImage, eigenValuesImage->GetRequestedRegion());
        itk::ImageRegionIterator<OutputImageType> outputIterator(outputImage, outputImage->GetRequestedRegion());

        for (eigenIterator.GoToBegin(), outputIterator.GoToBegin(); !eigenIterator.IsAtEnd(); ++eigenIterator, ++outputIterator)
        {
            EigenValueArrayType eigenValues = eigenIterator.Get();
            double vesselness = ComputeFrangiMeasure({eigenValues[0], eigenValues[1], eigenValues[2]});

            // Actualizar el valor del píxel de salida con el máximo vesselness encontrado hasta el momento
            if (vesselness > outputIterator.Get()) {
                outputIterator.Set(vesselness);
            }
        }
    }

    // Asegurarse de copiar todas las propiedades relevantes (dirección, origen, espaciado y región)
    outputImage->SetOrigin(inputImage->GetOrigin());
    outputImage->SetSpacing(inputImage->GetSpacing());
    outputImage->SetDirection(inputImage->GetDirection());
    outputImage->SetRegions(inputImage->GetLargestPossibleRegion());

    // Retornar la imagen procesada
    return outputImage;
}

double FrangiFilter::ComputeFrangiMeasure(const std::array<double, Dimension>& eigenValues)
{
    double lambda1 = eigenValues[0];
    double lambda2 = eigenValues[1];
    double lambda3 = eigenValues[2];

    // Evitar casos donde lambda1 es 0
    if (lambda1 == 0) {
        return 0;
    }

    // Calcular R_A, R_B y S
    double RA = std::fabs(lambda2) / std::fabs(lambda1);
    double RB = std::fabs(lambda3) / std::sqrt(std::fabs(lambda1 * lambda2));
    double S = std::sqrt(lambda1 * lambda1 + lambda2 * lambda2 + lambda3 * lambda3);

    // Calcular la medida de Frangi
    double vesselness = std::exp(-RA * RA / (2 * m_Alpha * m_Alpha)) *
                        std::exp(-RB * RB / (2 * m_Beta * m_Beta)) *
                        (1 - std::exp(-S * S / (2 * m_Gamma * m_Gamma)));

    return vesselness;
}
