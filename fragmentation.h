#pragma once
#include "box.h"
#include <vector>

typedef std::pair< std::vector<double>, std::vector<double> > min_max_vectors;
typedef std::pair<Box, Box> boxes_pair;

extern const double g_l1_max;
extern const double g_l2_max;
extern const double g_l1_min;
extern const double g_l2_min;
extern const double g_l0;

extern const double g_precision;

class low_level_fragmentation
{
protected:
	/// ������������� � ������ ������ box
	Box current_box;
	/// ������� VerticalSplitter() ��������� ���������� � �������� ��������� box �� ������
	void VerticalSplitter( const Box& box, boxes_pair& vertical_splitter_pair );
	/// ������� HorizontalSplitter() ��������� ���������� � �������� ��������� box �� ������
	void HorizontalSplitter( const Box& box, boxes_pair& horizontal_splitter_pair );
	/// ������� GetNewBoxes() ��������� ���������� � �������� ��������� box �� ������� �������,
	/// ������� VerticalSplitter() ��� HorizontalSplitter()
	void GetNewBoxes( const Box& box, boxes_pair& new_pair_of_boxes );
	/// ������� FindTreeDepth() ���������� ������� ��������� ������, 
	/// ������� ������������� ���������� �������� ���������
	unsigned int FindTreeDepth();
	/// ������� ClasifyBox() ����������� box � �������������� ���
	int ClasifyBox( const min_max_vectors& vects );
	/// ������� GetBoxType() ��������� ������������������ ����� box �� ��������� �������, 
	/// ��� ������� ��� �� �������, ��� ��������� ��� � ��������� �������, 
	/// ��� ������� ��� � ���, ��� �������� ����������� �������
	void GetBoxType( const Box& box );
	/// ������� GetMinMax() ���������� ��������� �������� ������� gj �� box-e
	/// ��������! � ������ ������� ������ ����������� ������� GetMinMax() ���� �� ������
	virtual void GetMinMax( const Box& box, min_max_vectors& min_max_vector ) = 0;

public:
	low_level_fragmentation() {}

	low_level_fragmentation( double& min_x, double& min_y, double& x_width, double& y_height );

	low_level_fragmentation( const Box& box );

	/// ����������� ����������
	virtual ~low_level_fragmentation() {}
};


class high_level_analysis : public low_level_fragmentation
{
protected:
	/// ���������������� � �������� ������ ������� GetMinMax()
	void GetMinMax(const Box& box, min_max_vectors& min_max_vector) override;

public:
	high_level_analysis() {}

	high_level_analysis( double& min_x, double& min_y, double& x_width, double& y_height );

	high_level_analysis( Box& box );

	///  ������� GetSolution() ��������� �������� ���������� ������� ������� ������������
	void GetSolution();
};


/// ������� WriteResults() ���������� ��������� ���������� box-�� (����������� � ��������
/// ������������, � ��������� ������� � �� ���������, �� ����������� ��������) � �������� 
/// ����� ��� ���������� ������������
void WriteResults( const char* file_names[] );