template <int M, int N>
inline void kernelFromFile(Kernel<M, N>& kernel, const char* name)
{
    std::ifstream file;
    file.open(std::string {"library/res/kernels/"} + name);
    file >> kernel.factor;
    kernel.factor = 1.0f / kernel.factor;
    for(int i {0}; i < M; ++i)
        for(int j {0}; j < N; ++j)
            file >> kernel.matrix[i][j];
    file.close();
}
