#if !defined(KERNEL_HPP)
#define KERNEL_HPP

#include <config.hpp>

#include <string>
#include <fstream>

template <int M, int N>
struct DLL_EXPORT Kernel
{
    float factor;
    int matrix[M][N];
};

template <int M, int N>
void DLL_EXPORT kernelFromFile(Kernel<M, N>& kernel, const char* name);

#include <kernel.inl>

#endif // KERNEL_HPP
