# Distributed File I/O Benchmark

Simple I/O workload generator uing MPI-IO to benchmark distributed file systems.

## Build Instructions

### Dependencies

- openmpi-devel
- cmake
- gcc

Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib64/openmpi
make
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.