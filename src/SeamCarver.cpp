#include "SeamCarver.h"

#include <cmath>

SeamCarver::SeamCarver(Image image)
    : m_image(std::move(image))
{
}

const Image & SeamCarver::GetImage() const
{
    return m_image;
}

size_t SeamCarver::GetImageWidth() const
{
    return m_image.width();
}

size_t SeamCarver::GetImageHeight() const
{
    return m_image.height();
}

int delta(Image::Pixel first, Image::Pixel second)
{
    int R = first.m_red - second.m_red;
    int G = first.m_green - second.m_green;
    int B = first.m_blue - second.m_blue;
    return R * R + G * G + B * B;
}

double SeamCarver::GetPixelEnergy(size_t columnId, size_t rowId) const
{
    size_t height = GetImageHeight();
    size_t width = GetImageWidth();

    int deltaX = delta(m_image.GetPixel((columnId + 1) % width, rowId),
                       m_image.GetPixel((columnId + width - 1) % width, rowId));
    int deltaY = delta(m_image.GetPixel(columnId, (rowId + 1) % height),
                       m_image.GetPixel(columnId, (height + rowId - 1) % height));
    return sqrt(deltaX + deltaY);
}

std::vector<std::vector<double>> SeamCarver::GetPixelEnergyTable() const
{
    size_t width = GetImageWidth();
    size_t height = GetImageHeight();
    std::vector<std::vector<double>> energy_table(width, std::vector<double>(height));
    for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
            energy_table[i][j] = SeamCarver::GetPixelEnergy(i, j);
        }
    }
    return energy_table;
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    size_t height = GetImageWidth();
    size_t width = GetImageHeight();
    std::vector<std::vector<double>> energy_table = GetPixelEnergyTable();
    Seam seam;
    seam.resize(height);
    double minEl = INT32_MAX;
    std::vector<std::vector<double>> dp = energy_table;

    for (size_t i = 1; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            if (j == 0) {
                dp[i][j] += std::min(dp[i - 1][j], dp[i - 1][j + 1]);
            }
            else if (j == width - 1) {
                dp[i][j] += std::min(dp[i - 1][j], dp[i - 1][j - 1]);
            }
            else {
                dp[i][j] += std::min(std::min(dp[i - 1][j - 1], dp[i - 1][j]), dp[i - 1][j + 1]);
            }
            if (i == height - 1) {
                if (dp[i][j] < minEl) {
                    minEl = dp[i][j];
                    seam[height - 1] = j;
                }
            }
        }
    }

    size_t j = seam[height - 1];
    for (size_t i = height - 1; i > 0; i--) {
        if (j == 0) {
            if (dp[i - 1][j] > dp[i - 1][j + 1]) {
                j++;
            }
        }
        else if (j == width - 1) {
            if (dp[i - 1][j] > dp[i - 1][j - 1]) {
                j--;
            }
        }
        else {
            double parent = std::min(std::min(dp[i - 1][j - 1], dp[i - 1][j]), dp[i - 1][j + 1]);
            if (dp[i - 1][j - 1] == parent) {
                j--;
            }
            else if (dp[i - 1][j + 1] == parent) {
                j++;
            }
        }
        seam[i - 1] = j;
    }

    return seam;
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    size_t height = GetImageWidth();
    size_t width = GetImageHeight();
    std::vector<std::vector<double>> energy_table = GetPixelEnergyTable();
    Seam seam;
    seam.resize(width);
    double minEl = INT32_MAX;
    std::vector<std::vector<double>> dp = energy_table;

    for (size_t i = 1; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
            if (j == 0) {
                dp[j][i] += std::min(dp[j][i - 1], dp[j + 1][i - 1]);
            }
            else if (j == height - 1) {
                dp[j][i] += std::min(dp[j][i - 1], dp[j - 1][i - 1]);
            }
            else {
                dp[j][i] += std::min(std::min(dp[j - 1][i - 1], dp[j][i - 1]), dp[j + 1][i - 1]);
            }
            if (i == width - 1) {
                if (dp[j][i] < minEl) {
                    minEl = dp[j][i];
                    seam[width - 1] = j;
                }
            }
        }
    }

    size_t i = seam[width - 1];
    for (size_t j = width - 1; j > 0; j--) {
        if (i == 0) {
            if (dp[i][j - 1] > dp[i + 1][j - 1]) {
                i++;
            }
        }
        else if (i == height - 1) {
            if (dp[i][j - 1] > dp[i - 1][j - 1]) {
                i--;
            }
        }
        else {
            double parent = std::min(std::min(dp[i - 1][j - 1], dp[i][j - 1]), dp[i + 1][j - 1]);
            if (dp[i - 1][j - 1] == parent) {
                i--;
            }
            else if (dp[i + 1][j - 1] == parent) {
                i++;
            }
        }
        seam[j - 1] = i;
    }

    return seam;
}

void SeamCarver::RemoveHorizontalSeam(const Seam & seam)
{
    size_t height = GetImageHeight();
    size_t width = GetImageWidth();

    for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height - 1; j++) {
            if (seam[i] <= j) {
                m_image.m_table[i][j] = m_image.m_table[i][j + 1];
            }
        }
    }

    for (size_t i = 0; i < width; i++) {
        m_image.m_table[i].resize(height - 1);
    }
}

void SeamCarver::RemoveVerticalSeam(const Seam & seam)
{
    size_t height = GetImageHeight();
    size_t width = GetImageWidth();

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width - 1; j++) {
            if (seam[i] <= j) {
                m_image.m_table[j][i] = m_image.m_table[j + 1][i];
            }
        }
    }

    m_image.m_table.resize(width - 1);
}
