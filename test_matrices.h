#pragma once
#ifndef TEST_MATRICES_H
#define TEST_MATRICES_H

#include <vector>

const std::vector<std::vector<int>> TEST_MATRIX_A_2x2 = {
    {1, 2},
    {3, 4}
};

const std::vector<std::vector<int>> TEST_MATRIX_B_2x2 = {
    {5, 6},
    {7, 8}
};

const std::vector<std::vector<int>> EXPECTED_RESULT_2x2 = {
    {19, 22},
    {43, 50}
};

const std::vector<std::vector<int>> TEST_MATRIX_A_3x2 = {
    {1, 4},
    {2, 5},
    {3, 6}
};

const std::vector<std::vector<int>> TEST_MATRIX_B_2x3 = {
    {7, 8, 9},
    {10, 11, 12}
};

const std::vector<std::vector<int>> EXPECTED_RESULT_3x3 = {
    {47, 52, 57},
    {64, 71, 78},
    {81, 90, 99}
};

#endif
