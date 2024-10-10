# PVSBenchmark
This repository contains a Docker image that runs a benchmark for the Frangi filter applied to medical images. The image processes NIfTI files and provides multiple thresholds to evaluate performance.

## Usage
Follow the steps below to pull the Docker image and run the benchmark and Frangi filter.

### Step 1: Pull the Docker Image

First, download the Docker image from the repository by running the following command:

```bash
docker pull camijaramillo/benchmark_pvs:Thresholds
```

### Step 2: Run the Docker Container

Next, start the Docker container, mounting your local folder to share files between your computer and Docker. Use this command:

```bash
docker run --rm -it -v "filelocation:/mnt/data" camijaramillo/benchmark_pvs:Thresholds /bin/bash
```
#### Explanation:

- `--rm`: Automatically removes the container after it stops.
- `-it`: Opens an interactive terminal.
- `-v "filelocation:/mnt/data"`: Mounts your local directory (`C:\Users\filelocation`) to the `/mnt/data` directory inside the container, enabling access to files.

### Step 3: Navigate to the Build Directory
Once inside the container, navigate to the build directory where the benchmark executable is located:

```bash
cd build
```
### Step 4: Run the Benchmark
Run the benchmark using a list of images and parameters stored in text files (imagenes.txt and params.txt), along with the number of thresholds to apply:

```bash
./itk_frangi_benchmark /mnt/data/imagenes.txt /mnt/data/params.txt Frangi 10
```
#### Explanation:

- /mnt/data/imagenes.txt: Path to the text file containing image file paths.
- /mnt/data/params.txt: Path to the text file containing the parameters for the benchmark.
- Frangi: Specifies the Frangi filter as the method to use.
- 10: The number of thresholds to apply.

### Step 5: Run the Frangi Filter
To apply the Frangi filter to a specific image, use the following command:

```bash
./itk_frangi_filter --input /mnt/data/AAA417_56_hm_tse_ac-pc_512_p2_reg2ref_Warped.nii --output /mnt/data/salida2.nii --sigmaMin 0.2 --sigmaMax 0.4 --nbSigmaSteps 4 --alpha 3.0 --beta 0.1 --gamma 1000
```
#### Explanation:

- input: Path to the input NIfTI image (.nii format).
- output: Path to the output image where the result will be saved.
- sigmaMin and --sigmaMax: Define the range of scales (sigma) to apply to the filter.
- nbSigmaSteps: Number of steps between sigmaMin and sigmaMax.
- alpha, --beta, and --gamma: Parameters that adjust the filter’s response to tubular structures.