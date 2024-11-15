#include <stdio.h>
#include <stdlib.h>
#include <csignal>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include "test_matrices.h"

#define MAX_PROCESSES 10
#define MAX_ROW_COL 50
#define MIN_ROW_COL 1

volatile sig_atomic_t ready = 0;

/*
 * Custom function declarations.
 */
bool compare_matrices(const std::vector<std::vector<int>>& result, const std::vector<std::vector<int>>& expected);
std::vector<std::vector<int>> generate_matrix_values(int rows, int cols);
std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> generate_matrices(int rowsA, int colsA, int colsB);
std::vector<std::vector<int>> matrix_multiplication(const std::vector<std::vector<int>> matrixA, const std::vector<std::vector<int>> matrixB);

std::vector<std::vector<int>> signal_based_logic();
std::vector<std::vector<int>> signal_based_logic(const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB);
std::vector<std::vector<int>> file_based_logic();
std::vector<std::vector<int>> file_based_logic(const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB);

void print_matrix(const std::vector<std::vector<int>> matrix);
void are_equal_output(bool result);
void use_menu();
void calculate_element(int row, int col, const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB, int* result);
void calculate_and_write_to_file(int row, int col, const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB);
void test_cases();
void signal_handler(int sig);

/*
 * Main program run.
 */
int main(int argc, char** argv) {
    srand(time(nullptr));
    use_menu();
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 * Custom function definitions.
*/

void signal_handler(int sig) {
    ready = 1;
}

void print_matrix(const std::vector<std::vector<int>> matrix) {
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    }
}


void use_menu()
{
    std::cout << std::string(40,'-') + '\n'; 
    std::cout << "Matrix Multiplication\n\t\t\tKrystian Fryca\n";
    std::cout << std::string(40,'-') + "\n"; 
    int decision;
    while(true)
    {
        std::vector<std::vector<int>> resultMatrix;
        std::cout << "\n1. Signals,\n2. Files,\n3. Test cases,\n4. Exit.: ";
        std::cin >> decision;
        switch (decision)
        {
        case 1:
            resultMatrix = signal_based_logic();
            std::cout << "\nResult Matrix (Signal-based):\n";
            print_matrix(resultMatrix);
            break;
        case 2:
            resultMatrix = file_based_logic();
            std::cout << "\nResult Matrix (File-based):\n";
            print_matrix(resultMatrix);
            break;
        case 3:
            test_cases();
            break;
        case 4:
            exit(0);
        default:
            std::cout << "Unsupported option: " << decision << "\n";
            std::cout << "Exiting.." << "\n";
            exit(0);
        }
    }
}

bool compare_matrices(const std::vector<std::vector<int>>& result, const std::vector<std::vector<int>>& expected) {
    if (result.size() != expected.size() || result[0].size() != expected[0].size()) return false;
    for (int i = 0; i < result.size(); ++i) {
        for (int j = 0; j < result[i].size(); ++j) {
            if (result[i][j] != expected[i][j]) return false;
        }
    }
    return true;
}

std::vector<std::vector<int>> generate_matrix_values(int rows, int cols) {
    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols));
    for (auto& row : matrix) {
        for (auto& elem : row) {
            elem = rand() % 10;
        }
    }
    return matrix;
}

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> generate_matrices(int rowsA, int colsA, int colsB) {
    auto matrixA = generate_matrix_values(rowsA, colsA);
    auto matrixB = generate_matrix_values(colsA, colsB);
    return {matrixA, matrixB};
}

void calculate_element(int row, int col, const std::vector<std::vector<int>>& matrixA, 
                       const std::vector<std::vector<int>>& matrixB, int* result) {
    int value = 0;
    for (int k = 0; k < matrixA[0].size(); ++k) {
        value += matrixA[row][k] * matrixB[k][col];
    }
    *result = value;
    kill(getppid(), SIGUSR1);
    exit(0);
}

void calculate_and_write_to_file(int row, int col, const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB) {
    int value = 0;
    for (int k = 0; k < matrixA[0].size(); ++k) {
        value += matrixA[row][k] * matrixB[k][col];
    }
    std::ofstream outfile("result_" + std::to_string(row) + "_" + std::to_string(col) + ".txt");
    outfile << value;
    outfile.close();
    exit(0);
}

std::vector<std::vector<int>> signal_based_logic() {
    int rowsA = rand() % MAX_ROW_COL + MIN_ROW_COL;
    int colsA = rand() % MAX_ROW_COL + MIN_ROW_COL;
    int colsB = rand() % MAX_ROW_COL + MIN_ROW_COL;

    signal(SIGUSR1, signal_handler);

    auto [matrixA, matrixB] = generate_matrices(rowsA, colsA, colsB);

    std::cout << "Matrix A:\n";
    print_matrix(matrixA);
    std::cout << "\nMatrix B:\n";
    print_matrix(matrixB);

    // allocating shared block of memory
    int* result = (int*)mmap(nullptr, rowsA * colsB * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        std::cout << "mmap failed\n";
        exit(1);
    }

    int active_processes = 0;

    for (int row = 0; row < rowsA; ++row) {
        for (int col = 0; col < colsB; ++col) {
            if (active_processes >= MAX_PROCESSES) {
                while (!ready) { }
                ready = 0;
                active_processes--;
            }

            pid_t pid = fork();
            if (pid == 0) {
                calculate_element(row, col, matrixA, matrixB, &result[row * colsB + col]);
            } else if (pid > 0) {
                active_processes++;
            } else {
                std::cout << "Fork failed\n";
                exit(1);
            }
        }
    }

    while (active_processes > 0) {
        waitpid(-1, nullptr, 0);
        active_processes--;
    }

    std::vector<std::vector<int>> resultMatrix(rowsA, std::vector<int>(colsB, 0));
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            resultMatrix[i][j] = result[i * colsB + j];
        }
    }
    return resultMatrix;
}

std::vector<std::vector<int>> signal_based_logic(const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB) {
    int rowsA = matrixA.size();
    int colsA = matrixA[0].size();
    int rowsB = matrixB.size();
    int colsB = matrixB[0].size();

    if (colsA != rowsB) {
        std::cout << "Matrix dimensions do not match for multiplication.\n";
        return {};
    }

    signal(SIGUSR1, signal_handler);

    int* result = (int*)mmap(nullptr, rowsA * colsB * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        std::cout << "mmap failed\n";
        exit(1);
    }

    int active_processes = 0;

    for (int row = 0; row < rowsA; ++row) {
        for (int col = 0; col < colsB; ++col) {
            if (active_processes >= MAX_PROCESSES) {
                while (!ready) { }
                ready = 0;
                active_processes--;
            }

            pid_t pid = fork();
            if (pid == 0) {
                calculate_element(row, col, matrixA, matrixB, &result[row * colsB + col]);
            } else if (pid > 0) {
                active_processes++;
            } else {
                std::cout << "Fork failed";
                exit(1);
            }
        }
    }

    while (active_processes > 0) {
        waitpid(-1, nullptr, 0);
        active_processes--;
    }

    std::vector<std::vector<int>> resultMatrix(rowsA, std::vector<int>(colsB, 0));
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            resultMatrix[i][j] = result[i * colsB + j];
        }
    }
    return resultMatrix;
}

std::vector<std::vector<int>> file_based_logic() {
    int rowsA = rand() % MAX_ROW_COL + MIN_ROW_COL;
    int colsA = rand() % MAX_ROW_COL + MIN_ROW_COL;
    int colsB = rand() % MAX_ROW_COL + MIN_ROW_COL;

    auto [matrixA, matrixB] = generate_matrices(rowsA, colsA, colsB);

    std::cout << "Matrix A:\n";
    print_matrix(matrixA);
    std::cout << "\nMatrix B:\n";
    print_matrix(matrixB);

    int active_processes = 0;

    for (int row = 0; row < rowsA; ++row) {
        for (int col = 0; col < colsB; ++col) {
            if (active_processes >= MAX_PROCESSES) {
                wait(nullptr);
                active_processes--;
            }

            pid_t pid = fork();
            if (pid == 0) {
                calculate_and_write_to_file(row, col, matrixA, matrixB);
            } else if (pid > 0) {
                active_processes++;
            } else {
                std::cout << "Fork failed";
                exit(1);
            }
        }
    }

    while (active_processes > 0) {
        wait(nullptr);
        active_processes--;
    }

    std::vector<std::vector<int>> resultMatrix(rowsA, std::vector<int>(colsB, 0));
    for (int row = 0; row < rowsA; ++row) {
        for (int col = 0; col < colsB; ++col) {
            std::ifstream infile("result_" + std::to_string(row) + "_" + std::to_string(col) + ".txt");
            if (infile.is_open()) {
                infile >> resultMatrix[row][col];
                infile.close();
                std::remove(("result_" + std::to_string(row) + "_" + std::to_string(col) + ".txt").c_str());
            }
        }
    }
    return resultMatrix;
}

std::vector<std::vector<int>> file_based_logic(const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB) {
    int rowsA = matrixA.size();
    int colsA = matrixA[0].size();
    int rowsB = matrixB.size();
    int colsB = matrixB[0].size();

    if (colsA != rowsB) {
        std::cout << "Matrix dimensions do not match for multiplication.\n";
        return {};
    }

    int active_processes = 0;

    for (int row = 0; row < rowsA; ++row) {
        for (int col = 0; col < colsB; ++col) {
            if (active_processes >= MAX_PROCESSES) {
                wait(nullptr);
                active_processes--;
            }

            pid_t pid = fork();
            if (pid == 0) {
                std::ofstream outfile("result_" + std::to_string(row) + "_" + std::to_string(col) + ".txt");
                int value = 0;
                for (int k = 0; k < matrixA[0].size(); ++k) {
                    value += matrixA[row][k] * matrixB[k][col];
                }
                outfile << value;
                outfile.close();
                exit(0);
            } else if (pid > 0) {
                active_processes++;
            } else {
                std::cout << "Fork failed\n";
                exit(1);
            }
        }
    }

    while (active_processes > 0) {
        wait(nullptr);
        active_processes--;
    }

    std::vector<std::vector<int>> resultMatrix(rowsA, std::vector<int>(colsB, 0));
    for (int row = 0; row < rowsA; ++row) {
        for (int col = 0; col < colsB; ++col) {
            std::ifstream infile("result_" + std::to_string(row) + "_" + std::to_string(col) + ".txt");
            if (infile.is_open()) {
                infile >> resultMatrix[row][col];
                infile.close();
                std::remove(("result_" + std::to_string(row) + "_" + std::to_string(col) + ".txt").c_str());
            } else {
                std::cout << "Error reading file for position (" << row << ", " << col << ")\n";
            }
        }
    }
    return resultMatrix;
}

void are_equal_output(bool result)
{
    if (result){
        std::cout << "Matrices are Equal.\n";
    }
    else
        std::cout << "Matrices are NOT equal.\n";
}

void test_cases()
{
    std::vector<std::vector<int>> resultMatrix;
    std::cout << "\nTEST_MATRIX_A_2x2 x TEST_MATRIX_B_2x3\n";
    std::cout << "Matrix A:\n";
    print_matrix(TEST_MATRIX_A_2x2);

    std::cout << "\nMatrix B:\n";
    print_matrix(TEST_MATRIX_B_2x2);

    std::cout << "\nMultiplying using signal based parallel processing...\n";
    resultMatrix = signal_based_logic(TEST_MATRIX_A_2x2, TEST_MATRIX_B_2x2);
    print_matrix(resultMatrix);
    are_equal_output(compare_matrices(resultMatrix, EXPECTED_RESULT_2x2));

    std::cout << "\nMultiplying using file based parallel processing...\n";
    resultMatrix = file_based_logic(TEST_MATRIX_A_2x2, TEST_MATRIX_B_2x2);
    print_matrix(resultMatrix);
    are_equal_output(compare_matrices(resultMatrix, EXPECTED_RESULT_2x2));

    std::cout << '\n' + std::string(50,'-') + '\n';

    std::cout << "\nTEST_MATRIX_A_3x2 x TEST_MATRIX_B_2x3\n";
    std::cout << "Matrix A:\n";
    print_matrix(TEST_MATRIX_A_3x2);

    std::cout << "\nMatrix B:\n";
    print_matrix(TEST_MATRIX_B_2x3);

    std::cout << "\nMultiplying using signal based parallel processing...\n";
    resultMatrix = signal_based_logic(TEST_MATRIX_A_3x2, TEST_MATRIX_B_2x3);
    print_matrix(resultMatrix);
    are_equal_output(compare_matrices(resultMatrix, EXPECTED_RESULT_3x3));

    std::cout << "\nMultiplying using file based parallel processing...\n";
    resultMatrix = file_based_logic(TEST_MATRIX_A_3x2, TEST_MATRIX_B_2x3);
    print_matrix(resultMatrix);
    are_equal_output(compare_matrices(resultMatrix, EXPECTED_RESULT_3x3));
}

