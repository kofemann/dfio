# Distributed File I/O Benchmark

Simple I/O workload generator uing MPI-IO to benchmark distributed file systems.

## Build Instructions

### Dependencies

- openmpi-devel
- cmake
- gcc

### Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib64/openmpi
make
```

## Usage

Run the benchmark using `mpirun` or `mpiexec`. For example:

```bash
export PATH=$PATH:/usr/lib64/openmpi/bin/
mpirun [mpi_options] ./dfio <path_to_data_directory>
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.