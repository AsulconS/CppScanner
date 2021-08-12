#include <image.hpp>
#include <kernel.hpp>

#include <cmath>

#define SQD(v) ((v) * (v))

void enhance(Image& img)
{
    Eigen::MatrixXf& imgDataR {img.matrix(0)};
    Eigen::MatrixXf& imgDataG {img.matrix(1)};
    Eigen::MatrixXf& imgDataB {img.matrix(2)};
    for(int i {0}; i < img.rows(); ++i)
    {
        for(int j {0}; j < img.cols(); ++j)
        {
            imgDataR(i, j) = log1pf(imgDataR(i, j));
            imgDataG(i, j) = log1pf(imgDataG(i, j));
            imgDataB(i, j) = log1pf(imgDataB(i, j));
        }
    }
    img.remap();
}

int main(int argc, char** argv)
{
    Image img(argv[1], RGB);

    enhance(img);

    img.write(argv[1]);

    return 0;
}
