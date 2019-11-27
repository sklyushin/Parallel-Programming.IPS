#include <stdio.h>
#include <ctime>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include <chrono>
#include <iostream>

using namespace std::chrono;
using namespace std;

#define F(x)	8 / (2 + 2 * x * x)

double integralParallel(const double a, const double b, const int n)
{
	int i;
	double x, h, S;
	cilk::reducer < cilk::op_add<double> > sum(0);

	h = (b - a) / n;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	cilk_for(i = 0; i < n; ++i) {
		x = a + i * h;
		*sum += F(x);
	}
	S = sum.get_value() * h;
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	cout << "Duration of integral parallel calculation (" << n << " partitions): "
		<< duration.count() << " seconds" << endl << endl;

	return S;
}

double integralSerial(const double a, const double b, const int n)
{
	int i;
	double x, h, S = 0;

	h = (b - a) / n;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	for (i = 0; i < n; ++i) {
		x = a + i * h;
		S += F(x);
	}
	S = S * h;
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	cout << "Duration of integral serial calculation (" << n << " partitions):   "
		<< duration.count() << " seconds" << endl;

	return S;
}

int main()
{
	const double a = -1, b = 1;
	double time, Sser, Spar;

	__cilkrts_set_param("nworkers", "4");

	for (int n = 10; n <= 1000000; n *= 10)
	{
		Sser = integralSerial(a, b, n);
		Spar = integralParallel(a, b, n);

		cout << "Result (Serial):   " << Sser << endl;
		cout << "Result (Parallel): " << Spar << endl << endl;

		cout << "|---------------------------------------------------------------------------------|" << endl;
	}

	return 0;
}