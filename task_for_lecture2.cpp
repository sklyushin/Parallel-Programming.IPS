#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
#include <iostream>
#include <vector>

#define ARRAY_SIZE(arr)	sizeof(arr)/sizeof(arr[0])

using namespace std::chrono;
using namespace std;

/// Функция ReducerMaxTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n",
		maximum->get_reference(), maximum->get_index_reference());
}

/// Функция ReducerMinTest() определяет минимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimum element = %d has index = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// Функция ParallelSort() сортирует массив в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void ParallelSort(int *begin, int *end)
{
	if (begin != end) 
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle); 
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

void CompareForAndCilk_For(size_t sz)
{
	cilk::reducer<cilk::op_vector<int>>red_vec;
	vector<int> vec;
	duration<double> duration;
	high_resolution_clock::time_point t1, t2;

	t1 = high_resolution_clock::now();
	cilk_for(long i = 0; i < sz; ++i) {
		red_vec->push_back(rand() % 20000 + 1);
	};
	t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	cout << "Duration of the filling reducer vector with cilk_for is: " << duration.count() << " seconds" << endl;

	
	t1 = high_resolution_clock::now();
	for (long i = 0; i < sz; ++i) {
		vec.push_back(rand() % 20000 + 1);
	}
	t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	cout << "Duration of the filling vector with for is: " << duration.count() << " seconds" << endl;
	cout << "==================================================================" << endl;
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	long i;
	const long mass_size = 1000000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size]; 
	size_t sz_set[] = { 1000000 , 100000 , 10000, 1000, 500, 100, 50, 10};

	cout << "mass size = " << mass_size << endl << endl;

	for(i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}
	
	mass_begin = mass;
	mass_end = mass_begin + mass_size;

	cout << "Before sort: " << endl;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	cout << "Duration of the sort is: " << duration.count() << " seconds" << endl << endl;

	cout << "After sort: " << endl;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	for (i = 0; i < ARRAY_SIZE(sz_set); ++i) {
		cout << "mass size = " << sz_set[i] << endl << endl;
		CompareForAndCilk_For(sz_set[i]);
	}

	delete[]mass;
	return 0;
}
