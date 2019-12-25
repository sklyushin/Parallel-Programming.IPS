#include "fragmentation.h"
#include <fstream>
#include <algorithm>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include <cilk/reducer_vector.h>

using namespace std;

cilk::reducer<cilk::op_vector<Box>> solution;
cilk::reducer<cilk::op_vector<Box>> not_solution;
cilk::reducer<cilk::op_vector<Box>> boundary;
cilk::reducer<cilk::op_vector<Box>> temporary_boxes;

/// функции gj()
//------------------------------------------------------------------------------------------
double g1(double x1, double x2)
{
	return (x1*x1 + x2*x2 - g_l1_max*g_l1_max);
}

//------------------------------------------------------------------------------------------
double g2(double x1, double x2)
{
	return (g_l1_min*g_l1_min - x1*x1 - x2*x2);
}

//------------------------------------------------------------------------------------------
double g3(double x1, double x2)
{
	return (x1*x1 + x2*x2 - g_l2_max*g_l2_max);
}

//------------------------------------------------------------------------------------------
double g4(double x1, double x2)
{
	return (g_l2_min*g_l2_min - x1*x1 - x2*x2);
}


//------------------------------------------------------------------------------------------
low_level_fragmentation::low_level_fragmentation(double& min_x, double& min_y, double& x_width, double& y_height )
{
	current_box = Box( min_x, min_y, x_width, y_height );
}

//------------------------------------------------------------------------------------------
low_level_fragmentation::low_level_fragmentation(const Box& box)
{
	current_box = box;
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::VerticalSplitter(const Box& box, boxes_pair& vertical_splitter_pair)
{
	double xmin, ymin, width, height;

	box.GetParameters(xmin, ymin, width, height);

	vertical_splitter_pair.first = Box(xmin, ymin, width / 2, height);
	vertical_splitter_pair.second = Box(xmin + width / 2, ymin, width / 2, height);
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::HorizontalSplitter(const Box& box, boxes_pair& horizontal_splitter_pair)
{
	double xmin, ymin, width, height;

	box.GetParameters(xmin, ymin, width, height);

	horizontal_splitter_pair.first = Box(xmin, ymin, width, height / 2);
	horizontal_splitter_pair.second = Box(xmin, ymin + height / 2, width, height / 2);
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::GetNewBoxes(const Box& box, boxes_pair& new_pair_of_boxes)
{
	double xmin, ymin, width, height;

	box.GetParameters(xmin, ymin, width, height);

	if (height > width)
		HorizontalSplitter(box, new_pair_of_boxes);
	else if (width >= height)
		VerticalSplitter(box, new_pair_of_boxes);
}

//------------------------------------------------------------------------------------------
unsigned int low_level_fragmentation::FindTreeDepth()
{
	double box_diagonal = current_box.GetDiagonal();

	if (box_diagonal <= g_precision)
	{
		return 0;
	}
	else
	{
		boxes_pair new_boxes;
		// допустим, разобьем начальную область по ширине
		VerticalSplitter(current_box, new_boxes);
		unsigned int tree_depth = 1;

		box_diagonal = new_boxes.first.GetDiagonal();

		if (box_diagonal <= g_precision)
		{
			return tree_depth;
		}
		else
		{
			for (;;)
			{
				GetNewBoxes(new_boxes.first, new_boxes);
				++tree_depth;
				box_diagonal = new_boxes.first.GetDiagonal();

				if (box_diagonal <= g_precision)
				{
					break;
				}
			}
			return tree_depth;
		}
	}
}

//------------------------------------------------------------------------------------------
int low_level_fragmentation::ClasifyBox(const min_max_vectors& vects)
{
	if (*max_element(vects.second.begin(), vects.second.end()) < 0)
		return 1;

	if (*min_element(vects.first.begin(), vects.first.end()) > 0)
		return 0;

	if ((vects.first[0] == 0) && (vects.second[0] == 0))
		return 3;

	return 2;
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::GetBoxType(const Box& box)
{
	min_max_vectors min_max_vecs;
	boxes_pair pair;

	GetMinMax(box, min_max_vecs);

	switch (ClasifyBox(min_max_vecs)) {
		case 0: not_solution->push_back(box); break;
		case 1: solution->push_back(box); break;
		case 2: 
				GetNewBoxes(box, pair);
				temporary_boxes->push_back(pair.first);
				temporary_boxes->push_back(pair.second);
				break;
		case 3: boundary->push_back(box); break;
	}
	
}


//------------------------------------------------------------------------------------------
high_level_analysis::high_level_analysis( double& min_x, double& min_y, double& x_width, double& y_height ) :
					low_level_fragmentation(min_x, min_y, x_width, y_height) {}

//------------------------------------------------------------------------------------------
high_level_analysis::high_level_analysis( Box& box ) : low_level_fragmentation( box ) {}

//------------------------------------------------------------------------------------------
void high_level_analysis::GetMinMax( const Box& box, min_max_vectors& min_max_vecs )
{
	std::vector<double> g_min;
	std::vector<double> g_max;

	double a1min, a2min, a1max, a2max;
	double xmin, xmax, ymin, ymax;

	box.GetParameters(xmin, ymin, xmax, ymax);

	xmax = xmin + xmax;
	ymax = ymin + ymax;

	double curr_box_diagonal = box.GetDiagonal();

	if (curr_box_diagonal <= g_precision)
	{
		g_min.push_back(0);
		g_max.push_back(0);

		min_max_vecs.first = g_min;
		min_max_vecs.second = g_max;

		return;
	}

	// MIN
	// функция g1(x1,x2)
	a1min = __min(abs(xmin), abs(xmax));
	a2min = __min(abs(ymin), abs(ymax));
	g_min.push_back(g1(a1min, a2min));

	// функция g2(x1,x2)
	a1min = __max(abs(xmin), abs(xmax));
	a2min = __max(abs(ymin), abs(ymax));
	g_min.push_back(g2(a1min, a2min));

	// функция g3(x1,x2)
	a1min = __min(abs(xmin - g_l0), abs(xmax - g_l0));
	a2min = __min(abs(ymin), abs(ymax));
	g_min.push_back(g3(a1min, a2min));

	// функция g4(x1,x2)
	a1min = __max(abs(xmin - g_l0), abs(xmax - g_l0));
	a2min = __max(abs(ymin), abs(ymax));
	g_min.push_back(g4(a1min, a2min));

	// MAX
	// функция g1(x1,x2)
	a1max = __max(abs(xmin), abs(xmax));
	a2max = __max(abs(ymin), abs(ymax));
	g_max.push_back(g1(a1max, a2max));

	// функция g2(x1,x2)
	a1max = __min(abs(xmin), abs(xmax));
	a2max = __min(abs(ymin), abs(ymax));
	g_max.push_back(g2(a1max, a2max));

	// функция g3(x1,x2)
	a1max = __max(abs(xmin - g_l0), abs(xmax - g_l0));
	a2max = __max(abs(ymin), abs(ymax));
	g_max.push_back(g3(a1max, a2max));

	// функция g4(x1,x2)
	a1max = __min(abs(xmin - g_l0), abs(xmax - g_l0));
	a2max = __min(abs(ymin), abs(ymax));
	g_max.push_back(g4(a1max, a2max));

	min_max_vecs.first = g_min;
	min_max_vecs.second = g_max;
}

//------------------------------------------------------------------------------------------
void high_level_analysis::GetSolution()
{
	boxes_pair new_pair_of_boxes;
	size_t number_of_box_on_level = 0;
	int iteration_count = 0;

	iteration_count = FindTreeDepth() + 1;
	temporary_boxes->push_back(current_box);

	for (int i = 0; i < iteration_count; ++i) {
		vector<Box> tmp;
		temporary_boxes.move_out(tmp);
		number_of_box_on_level = tmp.size();
		vector<Box> curr_boxes(tmp);
		tmp.clear();
		temporary_boxes.set_value(tmp);

		for (int j = 0; j < number_of_box_on_level; ++j) {
			GetBoxType(curr_boxes[j]);
		}
	}
}


//------------------------------------------------------------------------------------------
void WriteResults( const char* file_names[] )
{
	double xmin, ymin, w, h;
	ofstream fout;
	vector<Box> solution_vect;

	solution.move_out(solution_vect);

	fout.open(file_names[0]);

	for (int i = 0; i < solution_vect.size(); i++) {
		
		solution_vect[i].GetParameters(xmin, ymin, w, h);

		fout << xmin << " " << ymin << " " << w << " " << h << '\n';
	}

	fout.close();

	fout.open(file_names[1]);

	boundary.move_out(solution_vect);

	for (int i = 0; i < solution_vect.size(); i++) {

		solution_vect[i].GetParameters(xmin, ymin, w, h);

		fout << xmin << " " << ymin << " " << w << " " << h << '\n';
	}

	fout.close();

	fout.open(file_names[2]);

	not_solution.move_out(solution_vect);

	for (int i = 0; i < solution_vect.size(); i++) {

		solution_vect[i].GetParameters(xmin, ymin, w, h);

		fout << xmin << " " << ymin << " " << w << " " << h << '\n';
	}

	fout.close();
}