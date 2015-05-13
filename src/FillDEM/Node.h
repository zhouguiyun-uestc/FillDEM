#ifndef NODE_HEAD_H
#define NODE_HEAD_H
#include <functional>
class Node
{
public:
	int row;
	int col;
	float spill;
	
	Node()
	{
		row = 0;
		col = 0;
		spill = -9999.0;
	}

	struct Greater : public std::binary_function< Node, Node, bool >
	{
		bool operator()(const Node n1, const Node n2) const
		{
			return n1.spill > n2.spill;
		}
	};

	bool operator==(const Node& a)
	{
		return (this->col == a.col) && (this->row == a.row);
	}
	bool operator!=(const Node& a)
	{
		return (this->col != a.col) || (this->row != a.row);
	}
	bool operator<(const Node& a)
	{
		return this->spill < a.spill;
	}
	bool operator>(const Node& a)
	{
		return this->spill > a.spill;
	}
	bool operator>=(const Node& a)
	{
		return this->spill >= a.spill;
	}
	bool operator<=(const Node& a)
	{
		return this->spill <= a.spill;
	}
};

#endif