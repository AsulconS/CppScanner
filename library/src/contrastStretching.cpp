#include <image.hpp>

#include <cmath>

#include <vector>
#include <string>

void contrastStretching(Image& img, float lowerBound = 0.0f, float upperBound = 1.0f)
{
    Eigen::MatrixXi histogram;
    img.calcHist(histogram);

    int sum {0};
    for(int i {0}; i < 256; ++i)
        sum += histogram(0, i);

    std::vector<int> indices;
    for(int i {0}; i < 256; ++i)
        for(int j {0}; j < histogram(0, i); ++j)
            indices.push_back(i);

    float minIndex {static_cast<float>(indices[std::ceil((indices.size() - 1) * lowerBound)])};
    float maxIndex {static_cast<float>(indices[std::ceil((indices.size() - 1) * upperBound)])};

    Eigen::MatrixXf& imgData {img.matrix()};
    for(int i {0}; i < img.rows(); ++i)
        for(int j {0}; j < img.cols(); ++j)
            imgData(i, j) = scaleValue(imgData(i, j), minIndex, maxIndex, 0.0f, 255.0f);
}

int main(int argc, char** argv)
{
    Image img(argv[1], GRAY);

    contrastStretching(img, std::stof(argv[2]), std::stof(argv[3]));

    img.write(argv[1]);

    return 0;
}
