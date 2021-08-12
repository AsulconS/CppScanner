#include <image.hpp>
#include <kernel.hpp>

#include <Eigen/SVD>

#include <cmath>

#include <iostream>

void homography(Image& img, const float x[4], const float y[4], const float xd[4], const float yd[4])
{
    Eigen::Matrix3f H {};
    Eigen::Matrix<float, 8, 9> A {};
    for(int i {0}; i < 4; ++i)
    {
        A(2 * i, Eigen::all) = Eigen::RowVector<float, 9>{
            x[i], y[i], 1.0f, 0.0f, 0.0f, 0.0f, -xd[i] * x[i], -xd[i] * y[i], -xd[i]
        };
        A(2 * i + 1, Eigen::all) = Eigen::RowVector<float, 9>{
            0.0f, 0.0f, 0.0f, x[i], y[i], 1.0f, -yd[i] * x[i], -yd[i] * y[i], -yd[i]
        };
    }
    Eigen::JacobiSVD<Eigen::Matrix<float, 8, 9>> SVD {A, Eigen::ComputeThinV};
    H = SVD.matrixV()(Eigen::all, 8).reshaped(3, 3).transpose();

    Image clippedImg {ftoi(xd[2]), ftoi(yd[2]), RGB};
    Eigen::MatrixXf& imgDataR {img.matrix(0)};
    Eigen::MatrixXf& imgDataG {img.matrix(1)};
    Eigen::MatrixXf& imgDataB {img.matrix(2)};
    Eigen::MatrixXf& clippedImgDataR {clippedImg.matrix(0)};
    Eigen::MatrixXf& clippedImgDataG {clippedImg.matrix(1)};
    Eigen::MatrixXf& clippedImgDataB {clippedImg.matrix(2)};
    for(int i {0}; i < img.rows(); ++i)
    {
        for(int j {0}; j < img.cols(); ++j)
        {
            Eigen::Vector3f cPos {itof(j), itof(i), 1.0f};
            Eigen::Vector3f nPos {(H * cPos)};
            nPos /= nPos[2];
            if(clippedImg.boundaries(ftoi(nPos[1]), ftoi(nPos[0])))
            {
                clippedImgDataR(ftoi(nPos[1]), ftoi(nPos[0])) = imgDataR(i, j) + 0.1f;
                clippedImgDataG(ftoi(nPos[1]), ftoi(nPos[0])) = imgDataG(i, j) + 0.1f;
                clippedImgDataB(ftoi(nPos[1]), ftoi(nPos[0])) = imgDataB(i, j) + 0.1f;
            }
        }
    }

    Kernel<3, 3> kernel {};
    kernelFromFile(kernel, "box_3");
    for(int i {0}; i < clippedImg.rows(); ++i)
    {
        for(int j {0}; j < clippedImg.cols(); ++j)
        {
            if(clippedImgDataR(i, j) == 0.0f && clippedImgDataG(i, j) == 0.0f && clippedImgDataB(i, j) == 0.0f)
            {
                int truePixels {0};
                for(int ki {0}; ki < 3; ++ki)
                    for(int kj {0}; kj < 3; ++kj)
                        if(clippedImg.boundaries(i - 1 + ki, j - 1 + kj))
                            if( clippedImgDataR(i - 1 + ki, j - 1 + kj) != 0.0f ||
                                clippedImgDataG(i - 1 + ki, j - 1 + kj) != 0.0f ||
                                clippedImgDataB(i - 1 + ki, j - 1 + kj) != 0.0f )
                                ++truePixels;
                kernel.factor = 1.0f / itof(truePixels);
                clippedImgDataR(i, j) = clippedImg.weighRegion(i, j, 0, kernel);
                clippedImgDataG(i, j) = clippedImg.weighRegion(i, j, 1, kernel);
                clippedImgDataB(i, j) = clippedImg.weighRegion(i, j, 2, kernel);
            }
        }
    }
    clippedImgDataR.array() -= 0.1f;
    clippedImgDataG.array() -= 0.1f;
    clippedImgDataB.array() -= 0.1f;
    img = clippedImg;
}

int main(int argc, char** argv)
{
    Image img(argv[1], RGB);

    float x[4] {std::stof(argv[2]), std::stof(argv[4]), std::stof(argv[6]), std::stof(argv[8])};
    float y[4] {std::stof(argv[3]), std::stof(argv[5]), std::stof(argv[7]), std::stof(argv[9])};

    float fFactor   {std::stof(argv[10])};
    float minWidth  {fFactor * std::min(std::abs(x[0] - x[3]), std::abs(x[1] - x[2]))};
    float minHeight {fFactor * std::min(std::abs(y[0] - y[1]), std::abs(y[2] - y[3]))};

    float xd[4] {0.0f, 0.0f, minWidth, minWidth};
    float yd[4] {0.0f, minHeight, minHeight, 0.0f};

    homography(img, x, y, xd, yd);

    img.write(argv[1]);

    return 0;
}
