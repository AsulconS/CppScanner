#include <image.hpp>
#include <kernel.hpp>

#include <cmath>

#define SQD(v) ((v) * (v))

void sobelFilter(Image& img)
{
    Kernel<3, 3> xGradKernel {};
    Kernel<3, 3> yGradKernel {};
    Kernel<9, 9> gaussianKernel {};
    kernelFromFile(xGradKernel, "xgradient_3");
    kernelFromFile(yGradKernel, "ygradient_3");
    kernelFromFile(gaussianKernel, "gaussian2_9");

    img.convolve(0, [](float x)->float { return x; }, gaussianKernel);
    img.convolve(0, [](float gx, float gy)->float {
        return std::sqrtf(SQD(gx) + SQD(gy));
    }, xGradKernel, yGradKernel);
}

int main(int argc, char** argv)
{
    Image img(argv[1], GRAY);

    sobelFilter(img);

    img.write(argv[1]);

    return 0;
}
