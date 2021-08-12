template <typename F, typename... KArgs>
inline void Image::convolve(int channel, F f, KArgs&&... kernels)
{
    Image temp {*this};
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            m_data[channel](i, j) = f(temp.weighRegion(i, j, channel, std::forward<KArgs>(kernels))...);
}

template <int M, int N>
inline float Image::weighRegion(int row, int col, int channel, const Kernel<M, N>& kernel)
{
    static_assert(M & 0x1 && N & 0x1, "Kernel Dimensions must be odd");
    float accum {0.0f};
    for(int m {0}, oi {row - M / 2}; m < M; ++m, ++oi)
        for(int n {0}, oj {col - N / 2}; n < N; ++n, ++oj)
            if(boundaries(oi, oj))
                accum += itof(kernel.matrix[m][n]) * m_data[channel](oi, oj);
            else if(boundaries(row + row - oi, oj))
                accum += itof(kernel.matrix[m][n]) * m_data[channel](row + row - oi, oj);
            else if(boundaries(oi, col + col - oj))
                accum += itof(kernel.matrix[m][n]) * m_data[channel](oi, col + col - oj);
            else if(boundaries(row + row - oi, col + col - oj))
                accum += itof(kernel.matrix[m][n]) * m_data[channel](row + row - oi, col + col - oj);
            else
                accum += itof(kernel.matrix[m][n]) * m_data[channel](row, col);
    return accum * kernel.factor;
}
