#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <unordered_map>

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::getline;
using std::fstream;
using std::ifstream;
using std::priority_queue;
using std::binary_function;


typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;

void FillDEM_Zhou_TwoPass(char* inputFile, char* outputFilledPath);
void FillDEM_Zhou_OnePass(char* inputFile, char* outputFilledPath);
void FillDEM_Zhou_Direct(char* inputFile, char* outputFilledPath);
//compute stats for a DEM
void calculateStatistics(const CDEM& dem, double* min, double* max, double* mean, double* stdDev)
{
	int width = dem.Get_NX();
	int height = dem.Get_NY();

	int validElements = 0;
	double minValue, maxValue;
	double sum = 0.0;
	double sumSqurVal = 0.0;
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				double value = dem.asFloat(row, col);
				
				if (validElements == 0)
				{
					minValue = maxValue = value;
				}
				validElements++;
				if (minValue > value)
				{
					minValue = value;
				}
				if (maxValue < value)
				{
					maxValue = value;
				}

				sum += value;
				sumSqurVal += (value * value);
			}
		}
	}

	double meanValue = sum / validElements;
	double stdDevValue = sqrt((sumSqurVal / validElements) - (meanValue * meanValue));
	*min = minValue;
	*max = maxValue;
	*mean = meanValue;
	*stdDev = stdDevValue;
}

//The implementation of the Priority-Flood algorithm in Wang and Liu (2006)
int FillDEM_Wang(char* inputFile, char* outputFilledPath)
{
	CDEM dem;
	double geoTransformArgs[6];
	cout<<"Reading tiff file..."<<endl;
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
	{
		cout<<"error!"<<endl;
		return 0;
	}
	
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	cout<<"DEM Width:"<<width<<"  Height:"<<height<<endl;
	
	Flag flag;
	if (!flag.Init(width,height)) {
		printf("Failed to allocate memory!\n");
		return 0;
	}


	cout<<"Using Wang & Liu (2006) method to fill DEM"<<endl;
	time_t timeStart, timeEnd;
	timeStart = time(NULL);

	PriorityQueue queue; 
	int validElementsCount = 0;
	// push border cells into the PQ
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			Node tmpNode;
			if (!dem.is_NoData(row, col))
			{
				validElementsCount++;
				for (int i = 0; i < 8; i++)
				{
					int iRow, iCol;
					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
					{
						tmpNode.col = col;
						tmpNode.row = row;
						tmpNode.spill = dem.asFloat(row, col);
						queue.push(tmpNode);
						flag.SetFlag(row,col);
						break;
					}
				}
			}
			else 
			{
				flag.SetFlag(row,col);
			}
		}
	}
	int percentFive = validElementsCount / 20;

	int count = 0;
	int iRow, iCol;
	float iSpill;
	while (!queue.empty())
	{
		count++;
		if (count % percentFive == 0)
		{
			int percentNum = count / percentFive;
			cout<<"Progress:"<<percentNum * 5 <<"%\r";
		}
		Node tmpNode = queue.top();
		queue.pop();

		int row = tmpNode.row;
		int col = tmpNode.col;
		float spill = tmpNode.spill;


		for (int i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, row);
			iCol = Get_colTo(i, col);

			if (!flag.IsProcessed(iRow, iCol))
			{
				iSpill = dem.asFloat(iRow, iCol);
				if (iSpill <= spill)
				{
					iSpill=spill;
				}
				dem.Set_Value(iRow, iCol, iSpill);
				flag.SetFlag(iRow,iCol);						
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				queue.push(tmpNode);
			}

		}

	}
	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	cout<<"Time used:"<<consumeTime<<" seconds"<<endl;

	double min, max, mean, stdDev;
	calculateStatistics(dem, &min, &max, &mean, &stdDev);

	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(), 
		(void *)dem.getDEMdata(),GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return 1;
}
//The implementation of the Priority-Flood algorithm in Barnes et al. (2014)
int FillDEM_Barnes(char* inputFile, char* outputFilledPath)
{
	CDEM dem;
	double geoTransformArgs[6];
	cout<<"Reading tiff file..."<<endl;
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
	{
		printf("Error occurred while reading GeoTIFF file!\n");
		return 0;
	}
	
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	cout<<"DEM Width:"<<  width<<"  Height:"<<height<<endl;
	
	cout<<"Using Barnes et al. (2014) method to fill DEM"<<endl;

	Flag flag;
	if (!flag.Init(width,height)) {
		printf("Failed to allocate memory!\n");
		return 0;
	}
	
	cout<<"\nStart filling depressions..."<<endl;
	time_t timeStart, timeEnd;
	timeStart = time(NULL);
	
	PriorityQueue queue;
	std::queue<Node> pitque;
	int validElementsCount = 0;
	// push border cells into the PQ
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			Node tmpNode;
			if (!dem.is_NoData(row, col))
			{
				validElementsCount++;
				for (int i = 0; i < 8; i++)
				{
					int iRow, iCol;
					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
					{
						tmpNode.col = col;
						tmpNode.row = row;
						tmpNode.spill = dem.asFloat(row, col);
						queue.push(tmpNode);
						flag.SetFlag(row,col);
						break;
					}
				}
			}
			else {
				flag.SetFlag(row,col);
			}
		}
	}
	int percentFive = validElementsCount / 20;

	int count = 0;
	Node tmpNode;
	int iRow, iCol;
	float iSpill;
	int i;
	while (!queue.empty() || ! pitque.empty())
	{
		count++;
		if (count % percentFive == 0)
		{
			int percentNum = count / percentFive;
			cout<<"Progress:"<<percentNum * 5 <<"%\r";
		}
		if (!pitque.empty()) {
			tmpNode = pitque.front();
			pitque.pop();
		}
		else
		{
			tmpNode = queue.top();
			queue.pop();
		}
		int row = tmpNode.row;
		int col = tmpNode.col;
		float spill = tmpNode.spill;


		for (i = 0; i < 8; i++)
		{	iRow = Get_rowTo(i, row);
			iCol = Get_colTo(i, col);
			if (!flag.IsProcessed(iRow,iCol))
			{
				iSpill = dem.asFloat(iRow, iCol);
				if (iSpill <= spill)
				{
					dem.Set_Value(iRow, iCol, spill);
					flag.SetFlag(iRow,iCol);						

					tmpNode.row = iRow;
					tmpNode.col = iCol;
					tmpNode.spill = spill;
					pitque.push(tmpNode);
				}
				else
				{
					dem.Set_Value(iRow, iCol, iSpill);
					flag.SetFlag(iRow,iCol);
					tmpNode.row = iRow;
					tmpNode.col = iCol;
					tmpNode.spill = iSpill;
					queue.push(tmpNode);
				}

			}

		}
	}
	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	cout<<"\nTime used:"<<consumeTime<<" seconds"<<endl;

	double min, max, mean, stdDev;
	calculateStatistics(dem, &min, &max, &mean, &stdDev);

	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(), 
		(void *)dem.getDEMdata(),GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return true;
}


int main(int argc, char* argv[])
{
	
	if (argc < 4)
	{
		cout<<"Fill DEM usage: FillDEM fillingMethod inputfileName outputfileName"<<endl;
		cout<<"wang: using the method in Wang and Liu (2006) \nbarnes: using the method in Barnes et al. (2014) method"<<endl;
		cout<<"zhou-twopass: using the two-pass implementation in the manuscript"<<endl;
		cout<<"zhou-onepass: using the one-pass implementation in the manuscript"<<endl;
		cout<<"zhou-direct: using the direct implementation in the manuscript"<<endl;
		cout<<"\nFor example, FillDEM zhou-onepass f:\\dem\\dem.tif f:\\dem\\dem_filled_wang.tif"<<endl;
		return 1;
	}
	
	string path(argv[2]);
	string outputFilledPath(argv[3]);
	size_t index = path.find(".tif");
	if (index ==string::npos) {
		cout<<"Input file name should have an extension of '.tif'"<<endl;
		return 1;
	}
	char* method=argv[1];
	string strFolder = path.substr(0, index);
	if (strcmp(method,"wang")==0)
	{
		FillDEM_Wang(&path[0], &outputFilledPath[0]); //wang 2006
	}
	else if (strcmp(method,"barnes")==0)
	{
		FillDEM_Barnes(&path[0], &outputFilledPath[0]); //barnes 2014
	}
	else if(strcmp(method,"zhou-twopass")==0)
	{
		FillDEM_Zhou_TwoPass(&path[0], &outputFilledPath[0]); //zhou
	}
	else if (strcmp(method,"zhou-onepass")==0)
	{
		FillDEM_Zhou_OnePass(&path[0], &outputFilledPath[0]); //zhou
	}
	else if(strcmp(method,"zhou-direct")==0)
	{
		FillDEM_Zhou_Direct(&path[0],&outputFilledPath[0]); //zhou
	}
	else 
	{
		cout<<"Unknown filling method"<<endl;
	}
	
	std::cout<<"\nPress any key to exit ..."<<endl;
	getchar();
	return 0;
}

