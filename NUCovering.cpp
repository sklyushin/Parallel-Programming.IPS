#include "fragmentation.h"
#include <locale.h>
#include <iostream>
#include <chrono>

using namespace std::chrono;
using namespace std;

/// параметры начальной прямоугольной области
const double g_l1_max = 12.0;
const double g_l2_max = g_l1_max;
const double g_l1_min = 8.0;
const double g_l2_min = g_l1_min;
const double g_l0 = 5.0;

/// точность аппроксимации рабочего пространства
const double g_precision = 0.25;


int main()
{
	high_resolution_clock::time_point t1, t2;
	duration<double> duration;

	setlocale(LC_ALL,"Rus");

	double min_x = -10, min_y = 0, x_width = 24, y_height = 16;

	high_level_analysis main_object(min_x, min_y, x_width, y_height);

	t1 = high_resolution_clock::now();

	main_object.GetSolution();

	t2 = high_resolution_clock::now();

	// Внимание! здесь необходимо определить пути до выходных файлов!
	const char* out_files[3] = { "solution.txt", "boundary.txt", "not_solution.txt" };
	WriteResults( out_files );

	duration = (t2 - t1);
	cout << "Duration : " << duration.count() << " seconds" << endl;

	return 0;
}