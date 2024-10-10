# Utilizar una imagen base con C++ y herramientas de construcción
FROM ubuntu:22.04


# Actualizar los repositorios e instalar dependencias esenciales
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libx11-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxt-dev \
    && rm -rf /var/lib/apt/lists/*

# Instalar ITK desde los repositorios de Ubuntu
RUN apt-get update && apt-get install -y \
    libinsighttoolkit5-dev \
    && rm -rf /var/lib/apt/lists/*

# Opción alternativa: Clonar e instalar ITK desde la fuente
# RUN git clone https://github.com/InsightSoftwareConsortium/ITK.git \
#     && cd ITK \
#     && mkdir build \
#     && cd build \
#     && cmake .. \
#     && make -j$(nproc) \
#     && make install

# Crear el directorio de trabajo
WORKDIR /usr/src/app

# Copiar los archivos del proyecto al contenedor
COPY . .

# Construir el proyecto con CMake
RUN mkdir build && cd build \
    && cmake .. \
    && make

# Comando por defecto al ejecutar el contenedor
CMD ["./build/tu-ejecutable"]
