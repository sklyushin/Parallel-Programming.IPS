#include <stdio.h>
#include <ctime>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include <chrono>
#include <iostream>

using namespace std::chrono;
using namespace std;

// количество строк в исходной квадратной матрице
const int MATRIX_SIZE = 1500;

/// Функция InitMatrix() заполняет переданную в качестве 
/// параметра квадратную матрицу случайными значениями
/// matrix - исходная матрица СЛАУ
void InitMatrix(double** matrix)
{
	for (int i = 0; i < MATRIX_SIZE; ++i)
	{
		matrix[i] = new double[MATRIX_SIZE + 1];
	}

	for (int i = 0; i < MATRIX_SIZE; ++i)
	{
		for (int j = 0; j <= MATRIX_SIZE; ++j)
		{
			matrix[i][j] = rand() % 2500 + 1;
		}
	}
}

/// Функция SerialGaussMethod() решает СЛАУ методом Гаусса 
/// matrix - исходная матрица коэффиициентов уравнений, входящих в СЛАУ,
/// последний столбей матрицы - значения правых частей уравнений
/// rows - количество строк в исходной матрице
/// result - массив ответов СЛАУ
double SerialGaussMethod(double **matrix, const int rows, double* result)
{
	int k;
	double koef;

	// прямой ход метода Гаусса
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (k = 0; k < rows; ++k)
	{
		//
		for (int i = k + 1; i < rows; ++i)
		{
			koef = -matrix[i][k] / matrix[k][k];

			for (int j = k; j <= rows; ++j)
			{
				matrix[i][j] += koef * matrix[k][j];
			}
		} 
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	// обратный ход метода Гаусса
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for (k = rows - 2; k >= 0; --k)
	{
		result[k] = matrix[k][rows];

		//
		for (int j = k + 1; j < rows; ++j)
		{
			result[k] -= matrix[k][j] * result[j];
		}

		result[k] /= matrix[k][k];
	}

	duration<double> duration = (t2 - t1);
	cout << "Duration of forward Gaussian elimination (Serial execution): " << duration.count() << " seconds" << endl << endl;

	return duration.count();
}

double ParallelGaussMethod(double **matrix, const int rows, double* result)
{
	int k;
	double koef;

	// прямой ход метода Гаусса
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (k = 0; k < rows; ++k)
	{
		//
		cilk_for (int i = k + 1; i < rows; ++i)
		{
			koef = -matrix[i][k] / matrix[k][k];

			for (int j = k; j <= rows; ++j)
			{
				matrix[i][j] += koef * matrix[k][j];
			}
		}
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	cout << "Duration of forward Gaussian elimination (Parallel execution): " << duration.count() << " seconds" << endl << endl;

	// обратный ход метода Гаусса
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for (k = rows - 2; k >= 0; --k)
	{
		cilk::reducer < cilk::op_add<double> > temp_result(matrix[k][rows]);

		//
		cilk_for (int j = k + 1; j < rows; ++j)
		{
			*temp_result += (-1)*matrix[k][j] * result[j];
		}
		result[k] = (temp_result.get_value()) / matrix[k][k];
	}
	
	return duration.count();
}

int main()
{
	srand((unsigned)time(0));

	__cilkrts_set_param("nworkers", "4");

	int i;
	double durationSerial, durationParallel;

	double **matrix = new double*[MATRIX_SIZE];
	double **matrix_p = new double*[MATRIX_SIZE];

	// цикл по строкам
	for (i = 0; i < MATRIX_SIZE; ++i)
	{
		matrix[i] = new double[MATRIX_SIZE + 1];
		matrix_p[i] = new double[MATRIX_SIZE + 1];
	}

	// массив решений СЛАУ
	double *result = new double[MATRIX_SIZE];
	double *result_p = new double[MATRIX_SIZE];

	// инициализация матрицы
	InitMatrix(matrix);
	
	for (int i = 0; i < MATRIX_SIZE; ++i)
	{
		for (int j = 0; j <= MATRIX_SIZE; ++j)
		{
			matrix_p[i][j] = matrix[i][j];
		}
	}

	durationSerial = SerialGaussMethod(matrix, MATRIX_SIZE, result);
	durationParallel = ParallelGaussMethod(matrix_p, MATRIX_SIZE, result_p);

	cout << "Boost is " << durationSerial / durationParallel << endl;

	for (i = 0; i < MATRIX_SIZE; ++i)
	{
		delete[] matrix[i];
		delete[] matrix_p[i];
	}

	delete[] result;
	delete[] result_p;

	return 0;
}