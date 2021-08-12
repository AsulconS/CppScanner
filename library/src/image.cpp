#include <image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

Image::Image()
    : m_shape   {0, 0, 0},
      m_data    {}
{
}

Image::Image(int width, int height, ColorType colorType)
    : m_shape   {height, width, static_cast<int>(colorType)},
      m_data    {}
{
    validateChannelNumber();
    for(int c {0}; c < m_shape(2); ++c)
    {
        m_data[c].resize(m_shape(0), m_shape(1));
        m_data[c].setZero();
    }
}

Image::Image(const char* filename, ColorType colorType)
    : m_shape   {0, 0, 0},
      m_data    {}
{
    load(filename, colorType);
    validateChannelNumber();
}

Image::Image(Image& o)
    : m_shape   {o.m_shape},
      m_data    {}
{
    for(int c {0}; c < m_shape(2); ++c)
        m_data[c] = o.m_data[c];
}

Image::Image(Image&& o)
    : m_shape   {std::move(o.m_shape)},
      m_data    {}
{
    for(int c {0}; c < m_shape(2); ++c)
        m_data[c] = std::move(o.m_data[c]);
}

Image::~Image()
{
}

Image& Image::operator=(Image& o)
{
    m_shape = o.m_shape;
    for(int c {0}; c < m_shape(2); ++c)
        m_data[c] = o.m_data[c];
    return *this;
}

Image& Image::operator=(Image&& o)
{
    m_shape = std::move(o.m_shape);
    for(int c {0}; c < m_shape(2); ++c)
        m_data[c] = std::move(o.m_data[c]);
    return *this;
}

Eigen::MatrixXf& Image::matrix(int channel)
{
    return m_data[channel];
}

void Image::load(const char* filename, ColorType colorType)
{
    uint8_t* imgData {static_cast<uint8_t*>(
        stbi_load(filename, &m_shape(1), &m_shape(0), &m_shape(2), static_cast<int>(colorType))
    )};
    if(imgData == nullptr)
    {
        std::cerr << "Error when loading the image: " << filename << std::endl;
        return;
    }
    if(colorType != AUTO)
        m_shape(2) = static_cast<int>(colorType);
    for(int c {0}; c < m_shape(2); ++c)
    {
        m_data[c].resize(m_shape(0), m_shape(1));
        for(int i {0}; i < m_shape(0); ++i)
            for(int j {0}; j < m_shape(1); ++j)
                m_data[c](i, j) = ui8tof(imgData[(i * m_shape(1) + j) * m_shape(2) + c]);
    }
    stbi_image_free(imgData);
}

void Image::write(const char* filename)
{
    uint8_t* imgData {static_cast<uint8_t*>(
        stbi__malloc(m_shape(0) * m_shape(1) * m_shape(2))
    )};
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            for(int c {0}; c < m_shape(2); ++c)
                imgData[(i * m_shape(1) + j) * m_shape(2) + c] = ftoui8(std::max(0.0f, std::min(m_data[c](i, j), 255.0f)));

    ImageType type {getType(filename)};
    switch(type)
    {
        case JPG:
            stbi_write_jpg(filename, m_shape(1), m_shape(0), m_shape(2), imgData, 100);
            break;
        case PNG:
            stbi_write_png(filename, m_shape(1), m_shape(0), m_shape(2), imgData, m_shape(1) * m_shape(2));
            break;
        default:
            stbi_write_bmp(filename, m_shape(1), m_shape(0), m_shape(2), imgData);
            break;
    }
    stbi_image_free(imgData);
}

int Image::rows()
{
    return m_shape(0);
}

int Image::cols()
{
    return m_shape(1);
}

int Image::width()
{
    return m_shape(1);
}

int Image::height()
{
    return m_shape(0);
}

int Image::channels()
{
    return m_shape(2);
}

Eigen::Vector3i Image::shape()
{
    return m_shape;
}

void Image::scale(float xScale, float yScale)
{
    Eigen::Vector3i oldShape {m_shape};
    m_shape << ftoi(yScale * itof(m_shape(0))), ftoi(xScale * itof(m_shape(1))), oldShape(2);
    if(m_shape(0) > oldShape(0))
        for(int c {0}; c < m_shape(2); ++c)
            m_data[c].conservativeResize(m_shape(0), Eigen::NoChange);
    if(m_shape(1) > oldShape(1))
        for(int c {0}; c < m_shape(2); ++c)
            m_data[c].conservativeResize(Eigen::NoChange, m_shape(1));

    Eigen::Matrix3f M {};
    M << xScale,   0.0f, 0.0f,
           0.0f, yScale, 0.0f,
           0.0f,   0.0f, 1.0f;
    for(int i {0}; i < oldShape(0); ++i)
    {
        for(int j {0}; j < oldShape(1); ++j)
        {
            Eigen::Vector3f cPos {itof(j), itof(i), 1.0f};
            Eigen::Vector3f nPos {(M * cPos)};
            nPos /= nPos[2];
            if(boundaries(ftoi(nPos[1]), ftoi(nPos[0])))
                for(int c {0}; c < m_shape(2); ++c)
                    m_data[c](ftoi(nPos[1]), ftoi(nPos[0])) = m_data[c](i, j);
        }
    }
}

void Image::remap(float lowerBound, float upperBound)
{
    float channelMinCoeff, channelMaxCoeff;
    float minCoeff {m_data[0].minCoeff()};
    float maxCoeff {m_data[0].maxCoeff()};
    for(int c {1}; c < m_shape(2); ++c)
    {
        channelMinCoeff = m_data[c].minCoeff();
        channelMaxCoeff = m_data[c].maxCoeff();
        if(channelMinCoeff < minCoeff)
            minCoeff = channelMinCoeff;
        if(channelMaxCoeff > maxCoeff)
            maxCoeff = channelMaxCoeff;
    }
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            for(int c {0}; c < m_shape(2); ++c)
                m_data[c](i, j) = scaleValue(m_data[c](i, j), minCoeff, maxCoeff, lowerBound, upperBound);
}

void Image::invert(float lowerBound, float upperBound)
{
    for(int c {0}; c < m_shape(2); ++c)
        m_data[c].array() = upperBound - (m_data[c].array() - lowerBound);
}

void Image::threshold(float lowerBound, float upperBound)
{
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            for(int c {0}; c < m_shape(2); ++c)
                if(m_data[c](i, j) < lowerBound || m_data[c](i, j) > upperBound)
                    m_data[c](i, j) = 0.0f;
}

void Image::binaryThreshold(float lowerBound, float upperBound)
{
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            for(int c {0}; c < m_shape(2); ++c)
                if(m_data[c](i, j) < lowerBound || m_data[c](i, j) > upperBound)
                    m_data[c](i, j) = 0.0f;
                else
                    m_data[c](i, j) = 255.0f;
}

void Image::calcHist(Eigen::MatrixXi& hist, int lowerBound, int upperBound)
{
    if(m_shape(2) == 1 || m_shape(2) == 2)
    {
        hist.resize(1, upperBound - lowerBound + 1);
        hist.setZero();
        for(int i {0}; i < m_shape(0); ++i)
            for(int j {0}; j < m_shape(1); ++j)
                ++hist(0, ftoi(m_data[0](i, j)));
    }
    else if(m_shape(2) == 3 || m_shape(2) == 4)
    {
        hist.resize(3, upperBound - lowerBound + 1);
        hist.setZero();
        for(int i {0}; i < m_shape(0); ++i)
            for(int j {0}; j < m_shape(1); ++j)
                for(int c {0}; c < m_shape(2); ++c)
                    ++hist(c, ftoi(m_data[c](i, j)));
    }
}

bool Image::boundaries(int row, int col)
{
    return (row >= 0 && row < m_shape(0)) && (col >=0 && col < m_shape(1));
}

Image Image::operator~()
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() = 255.0f - m_data[c].array();
    return res;
}

Image Image::operator+(float scalar)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() += scalar;
    return res;
}

Image Image::operator-(float scalar)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() -= scalar;
    return res;
}

Image Image::operator*(float scalar)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() *= scalar;
    return res;
}

Image Image::operator/(float scalar)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() /= scalar;
    return res;
}

Image Image::operator+(const Image& img)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() += img.m_data[c].array();
    return res;
}

Image Image::operator-(const Image& img)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() -= img.m_data[c].array();
    return res;
}

Image Image::operator*(const Image& img)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() *= img.m_data[c].array();
    return res;
}

Image Image::operator/(const Image& img)
{
    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
        res.m_data[c].array() /= img.m_data[c].array();
    return res;
}

Image Image::operator&(const Image& img)
{
    Image res {*this};
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            for(int c {0}; c < m_shape(2); ++c)
                res.m_data[c](i, j) = itof(ftoi(res.m_data[c](i, j)) & ftoi(img.m_data[c](i, j)));
    return res;
}

Image Image::operator|(const Image& img)
{
    Image res {*this};
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            for(int c {0}; c < m_shape(2); ++c)
                res.m_data[c](i, j) = itof(ftoi(res.m_data[c](i, j)) | ftoi(img.m_data[c](i, j)));
    return res;
}

Image Image::operator^(const Image& img)
{
    Image res {*this};
    for(int i {0}; i < m_shape(0); ++i)
        for(int j {0}; j < m_shape(1); ++j)
            for(int c {0}; c < m_shape(2); ++c)
                res.m_data[c](i, j) = itof(ftoi(res.m_data[c](i, j)) ^ ftoi(img.m_data[c](i, j)));
    return res;
}

Image Image::blend(const Image& img, float blendFactor)
{
    if(blendFactor < 0.0f || blendFactor > 1.0f)
        return *this;

    Image res {*this};
    for(int c {0}; c < m_shape(2); ++c)
    {
        res.m_data[c].array() *= blendFactor;
        res.m_data[c].array() += ((1.0f - blendFactor) * img.m_data[c].array());
    }
    return res;
}

Image Image::squared()
{
    Image res {*this};
    for(int c {0}; c < res.m_shape(2); ++c)
        res.m_data[c].array() = m_data[c].array().square();
    return res;
}

Image Image::squareRooted()
{
    Image res {*this};
    for(int c {0}; c < res.m_shape(2); ++c)
        res.m_data[c].array() = res.m_data[c].array().sqrt();
    return res;
}

Image Image::crop(int row, int col, int xrad, int yrad)
{
    Image res {xrad * 2, yrad * 2, static_cast<ColorType>(m_shape(2))};
    for(int i {0}, oi {row - yrad}; i < res.m_shape(0); ++i, ++oi)
        for(int j {0}, oj {col - xrad}; j < res.m_shape(1); ++j, ++oj)
            for(int c {0}; c < res.m_shape(2); ++c)
                if(boundaries(oi, oj))
                    res.m_data[c](i, j) = m_data[c](oi, oj);
    return res;
}

void Image::validateChannelNumber()
{
    if(m_shape(2) < 1 || m_shape(2) > MAX_CHANNEL_NUMBER)
        m_shape << 0, 0, 0;
}

ImageType Image::getType(const char* filename)
{
    std::string name {filename};
    std::string ext {name.substr(name.find(".") + 1)};
    if(ext == "png")
        return PNG;
    if(ext == "jpg" || ext == "jpeg")
        return JPG;
    return INVALID;
}

float scaleValue(float x, float a, float b, float c, float d)
{
    return c + (x - a) * ((d - c) / (b - a));
}
