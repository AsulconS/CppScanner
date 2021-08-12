#include <image.hpp>

#include <Eigen/LU>

#include <cmath>

#include <string>

#define SQD(v) ((v) * (v))

void cornerDetection(Image& img, int hRadius, float k, float sf, float threshold)
{
    Kernel<3, 3> xGradKernel {};
    Kernel<3, 3> yGradKernel {};
    Kernel<9, 9> gaussianKernel {};
    kernelFromFile(xGradKernel, "xgradient_3");
    kernelFromFile(yGradKernel, "ygradient_3");
    kernelFromFile(gaussianKernel, "gaussian2_9");

    // Image Downscale and Blurring to detect corners
    Image gaussImg {img};
    gaussImg.scale(sf, sf);
    gaussImg.convolve(0, [](float x)->float { return x; }, gaussianKernel);

    // Gradient Calculations
    Image xGradImg {gaussImg};
    Image yGradImg {gaussImg};
    Eigen::MatrixXf& xGradImgData {xGradImg.matrix()};
    Eigen::MatrixXf& yGradImgData {yGradImg.matrix()};
    xGradImg.convolve(0, [](float gx)->float { return gx; }, xGradKernel);
    yGradImg.convolve(0, [](float gy)->float { return gy; }, yGradKernel);

    Image cornersImg {gaussImg.width(), gaussImg.height(), GRAY};
    Eigen::MatrixXf& cornersImgData {cornersImg.matrix()};

    // R-Score calculation for corners
    Eigen::Matrix2f M;
    Eigen::Matrix2f G;
    int mEdgeSize {2 * hRadius + 1};
    for(int i {0}; i < cornersImg.rows(); ++i)
    {
        for(int j {0}; j < cornersImg.cols(); ++j)
        {
            M.setZero();
            for(int wi {0}, oi {i - hRadius}; wi < mEdgeSize; ++wi, ++oi)
                for(int wj {0}, oj {j - hRadius}; wj < mEdgeSize; ++wj, ++oj)
                    if(cornersImg.boundaries(oi, oj))
                    {
                        G << SQD(xGradImgData(oi, oj)), (xGradImgData(oi, oj) * yGradImgData(oi, oj)),
                             (xGradImgData(oi, oj) * yGradImgData(oi, oj)), SQD(yGradImgData(oi, oj));
                        M += G;
                    }
            cornersImgData(i, j) = M.determinant() - k * SQD(M.trace());
        }
    }

    // Remap and threshold best R-Scores (corners)
    cornersImg.remap();
    cornersImg.threshold(threshold);

    // Non-maximum supression
    for(int i {0}; i < cornersImg.rows(); ++i)
    {
        for(int j {0}; j < cornersImg.cols(); ++j)
        {
            for(int wi {0}, oi {i - hRadius}; wi < mEdgeSize; ++wi, ++oi)
                for(int wj {0}, oj {j - hRadius}; wj < mEdgeSize; ++wj, ++oj)
                    if(cornersImg.boundaries(oi, oj))
                        if(cornersImgData(oi, oj) > cornersImgData(i, j))
                        {
                            cornersImgData(i, j) = 0.0f;
                            goto stop;
                        }
            stop: continue;
        }
    }

    // Rescale for corner coords
    cornersImg.scale(1.0f / sf, 1.0f / sf);
    cornersImg.binaryThreshold(threshold);

    // Output
    img = cornersImg;
}

int main(int argc, char** argv)
{
    Image img(argv[1], GRAY);

    cornerDetection(img, std::stoi(argv[2]), std::stof(argv[3]), std::stof(argv[4]), std::stof(argv[5]));

    img.write(argv[1]);

    return 0;
}
