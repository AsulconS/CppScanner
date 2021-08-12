#if !defined(IMAGE_HPP)
#define IMAGE_HPP

#include <config.hpp>
#include <kernel.hpp>

#include <Eigen/Core>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <vector>
#include <string>
#include <iostream>

#define itof(V) static_cast<float>(V)
#define ui8tof(V) static_cast<float>(V)
#define ftoi(V) static_cast<int>(std::lround(V))
#define ftoui8(V) static_cast<uint8_t>(std::lround(V))

#define MAX_CHANNEL_NUMBER 4

enum ImageType { INVALID, JPG, PNG };
enum ColorType { AUTO, GRAY, GRAY_ALPHA, RGB, RGB_ALPHA };

class DLL_EXPORT Image
{
public:
    Image();
    Image(int width, int height, ColorType colorType = GRAY);
    Image(const char* filename, ColorType colorType = AUTO);
    Image(Image& o);
    Image(Image&& o);
    virtual ~Image();

    Image& operator=(Image& o);
    Image& operator=(Image&& o);

    Eigen::MatrixXf& matrix(int channel = 0);

    void load(const char* filename, ColorType colorType = AUTO);
    void write(const char* filename);

    int rows();
    int cols();
    int width();
    int height();
    int channels();
    Eigen::Vector3i shape();

    void scale(float xScale, float yScale);
    void remap(float lowerBound = 0.0f, float upperBound = 255.0f);
    void threshold(float lowerBound = 0.0f, float upperBound = 255.0f);
    void binaryThreshold(float lowerBound = 0.0f, float upperBound = 255.0f);
    void calcHist(Eigen::MatrixXi& hist, int lowerBound = 0, int upperBound = 255);

    template <typename F, typename... KArgs>
    void convolve(int channel, F f, KArgs&&... kernels);
    template <int M, int N>
    float weighRegion(int row, int col, int channel, const Kernel<M, N>& kernel);

    bool boundaries(int row, int col);

    Image operator~();

    Image operator+(float scalar);
    Image operator-(float scalar);
    Image operator*(float scalar);
    Image operator/(float scalar);

    Image operator+(const Image& img);
    Image operator-(const Image& img);
    Image operator*(const Image& img);
    Image operator/(const Image& img);
    Image operator&(const Image& img);
    Image operator|(const Image& img);
    Image operator^(const Image& img);

    Image blend(const Image& img, float blendFactor = 0.5f);

    Image squared();
    Image squareRooted();

    Image crop(int row, int col, int xrad, int yrad);

private:
    void validateChannelNumber();
    ImageType getType(const char* filename);

    Eigen::Vector3i m_shape;
    Eigen::MatrixXf m_data[MAX_CHANNEL_NUMBER];
};

float DLL_EXPORT scaleValue(float x, float a, float b, float c, float d);

#include <image.inl>

#endif // IMAGE_HPP
