#include "SeamCarver.h"

#include <algorithm>
#include <cmath>
#include <vector>

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
    return m_image.GetWidth();
}

size_t SeamCarver::GetImageHeight() const
{
    return m_image.GetHeight();
}

int delta(Image::Pixel first, Image::Pixel second)
{
    int R = first.m_red - second.m_red;
    int G = first.m_green - second.m_green;
    int B = first.m_blue - second.m_blue;
    return R * R + G * G + B * B;
}

double SeamCarver::GetPixelEnergy(const size_t columnId, const size_t rowId) const
{
    const size_t height = GetImageHeight();
    const size_t width = GetImageWidth();

    int deltaX = delta(m_image.GetPixel((columnId + 1) % width, rowId),
                       m_image.GetPixel((columnId + width - 1) % width, rowId));
    int deltaY = delta(m_image.GetPixel(columnId, (rowId + 1) % height),
                       m_image.GetPixel(columnId, (height + rowId - 1) % height));
    return std::sqrt(deltaX + deltaY);
}

std::vector<std::vector<double>> SeamCarver::GetPixelEnergyTable() const
{
    const size_t width = GetImageWidth();
    const size_t height = GetImageHeight();
    std::vector<std::vector<double>> energy_table(width, std::vector<double>(height));
    for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
            energy_table[i][j] = GetPixelEnergy(i, j);
        }
    }
    return energy_table;
}

template <typename T>
std::vector<std::vector<T>> transposedMatrix(const std::vector<std::vector<T>> & matrix)
{
    const size_t width = matrix.size();
    const size_t height = matrix.empty() ? 0 : matrix.front().size();
    std::vector<std::vector<T>> res(height, std::vector<T>(width));
    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            res[y][x] = matrix[x][y];
        }
    }
    return res;
}

std::vector<size_t> findSeam(std::vector<std::vector<double>> && dp)
{
    const size_t imageWidth = dp.size();
    const size_t imageHeight = dp.empty() ? 0 : dp.front().size();
    std::vector<size_t> seam(imageWidth);
    double minEl = std::numeric_limits<int32_t>::max();

    for (size_t i = 1; i < imageWidth; i++) {
        for (size_t j = 0; j < imageHeight; j++) {
            if (j == 0) {
                dp[i][j] += std::min(dp[i - 1][j], dp[i - 1][j + 1]);
            }
            else if (j == imageHeight - 1) {
                dp[i][j] += std::min(dp[i - 1][j], dp[i - 1][j - 1]);
            }
            else {
                dp[i][j] += std::min({dp[i - 1][j - 1], dp[i - 1][j], dp[i - 1][j + 1]});
            }
            if (i == imageWidth - 1) {
                if (dp[i][j] < minEl) {
                    minEl = dp[i][j];
                    seam[imageWidth - 1] = j;
                }
            }
        }
    }

    size_t j = seam[imageWidth - 1];
    for (size_t i = imageWidth - 1; i > 0; i--) {
        if (j == 0) {
            if (dp[i - 1][j] > dp[i - 1][j + 1]) {
                j++;
            }
        }
        else if (j == imageHeight - 1) {
            if (dp[i - 1][j] > dp[i - 1][j - 1]) {
                j--;
            }
        }
        else {
            double parent = std::min({dp[i - 1][j - 1], dp[i - 1][j], dp[i - 1][j + 1]});
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

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    return findSeam(GetPixelEnergyTable());
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    return findSeam(transposedMatrix(GetPixelEnergyTable()));
}

void SeamCarver::RemoveHorizontalSeam(const Seam & seam)
{
    const size_t width = GetImageWidth();
    for (size_t y = 0; y < width; y++) {
        m_image.m_table[y].erase(m_image.m_table[y].begin() + seam[y]);
    }
}

void SeamCarver::RemoveVerticalSeam(const Seam & seam)
{
    const size_t height = GetImageHeight();
    const size_t width = GetImageWidth();

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width - 1; j++) {
            if (seam[i] <= j) {
                m_image.m_table[j][i] = m_image.m_table[j + 1][i];
            }
        }
    }

    m_image.m_table.pop_back();
}
