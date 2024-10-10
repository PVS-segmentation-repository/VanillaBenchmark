cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(FrangiFilterITK)

# Buscar ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Incluir directorios
include_directories(include)

# Archivos fuente del filtro Frangi
set(FRANGI_SOURCES
    src/Filters/main.cpp
    src/Filters/FrangiFilter.cpp
)

# Archivos fuente del benchmark
set(BENCHMARK_SOURCES
    src/benchmark/benchmark.cpp
    src/Filters/FrangiFilter.cpp
)

# Ejecutable del filtro Frangi
add_executable(itk_frangi_filter ${FRANGI_SOURCES})

# Ejecutable del benchmark
add_executable(itk_frangi_benchmark ${BENCHMARK_SOURCES})

# Linkear ITK
target_link_libraries(itk_frangi_filter ${ITK_LIBRARIES})
target_link_libraries(itk_frangi_benchmark ${ITK_LIBRARIES})
