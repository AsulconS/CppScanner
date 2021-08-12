#include <image.hpp>

#include <string>

bool threshold(float value, float a, float b)
{
    return value >= a && value <= b;
}

void thresholding(Image& img, float a, float b)
{
    Eigen::MatrixXf& imgData {img.matrix()};
    for(int i {0}; i < img.rows(); ++i)
        for(int j {0}; j < img.cols(); ++j)
            if(threshold(imgData(i, j), a, b))
                imgData(i, j) = 255.0f;
            else
                imgData(i, j) = 0.0f;
}

int main(int argc, char** argv)
{
    Image img(argv[1], GRAY);

    thresholding(img, std::stof(argv[2]), std::stof(argv[3]));

    img.write(argv[1]);

    return 0;
}
