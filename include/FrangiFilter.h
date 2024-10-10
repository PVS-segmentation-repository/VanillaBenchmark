#ifndef FRANGI_FILTER_H
#define FRANGI_FILTER_H

#include "itkImage.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"

constexpr unsigned int Dimension = 3; // Cambia a 2 si trabajas con imágenes 2D
using PixelType = float;
using ImageType = itk::Image<PixelType, Dimension>;
using HessianPixelType = itk::SymmetricSecondRankTensor<double, Dimension>;
using HessianImageType = itk::Image<HessianPixelType, Dimension>;

class FrangiFilter
{
public:
    // Constructor que recibe los parámetros necesarios
    FrangiFilter(double sigmaMin, double sigmaMax, int nbSigmaSteps, double alpha, double beta, double gamma);
    
    // Aplicar el filtro Frangi a una imagen de entrada
    ImageType::Pointer ApplyFrangiFilter(ImageType::Pointer inputImage);

private:
    // Cálculo de la medida de Frangi
    double ComputeFrangiMeasure(const std::array<double, Dimension>& eigenValues);

    // Parámetros del filtro
    double m_SigmaMin;
    double m_SigmaMax;
    int m_NbSigmaSteps;
    double m_Alpha;
    double m_Beta;
    double m_Gamma;
};

#endif // FRANGI_FILTER_H
