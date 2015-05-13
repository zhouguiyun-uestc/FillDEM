#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>
#include "DEM.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <stack>
#include <unordered_map>
using namespace std;

typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;
void InitPriorityQue_onepass(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue,int& percentFive)
{
	int width=dem.Get_NX();
	int height=dem.Get_NY();
	int validElementsCount = 0;
	Node tmpNode;
	int iRow, iCol;
	// push border cells into the PQ
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				validElementsCount++;
				for (int i = 0; i < 8; i++)
				{

					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
					{
						tmpNode.col = col;
						tmpNode.row = row;
						tmpNode.spill = dem.asFloat(row, col);
						priorityQueue.push(tmpNode);

						flag.SetFlag(row,col);
						break;
					}
				}
			}
			else{
				flag.SetFlag(row,col);
			}
		}
	}

	percentFive = validElementsCount / 20;
}
void ProcessTraceQue_onepass(CDEM& dem,Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue,int& count, int percentFive)
{
	int iRow, iCol,i;
	float iSpill;
	Node N,node,headNode;
	int width=dem.Get_NX();
	int height=dem.Get_NY();	
	int total=0,nPSC=0;
	bool bInPQ=false;
	bool isBoundary;
	int j,jRow,jCol;
	while (!traceQueue.empty())
	{
		node = traceQueue.front();
		traceQueue.pop();
		total++;
		if ((count+total) % percentFive == 0)
		{
			std::cout<<"Progress："<<(count+total) / percentFive * 5 <<"%\r";
		}
		bInPQ=false;
 		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i, node.col);
			if (flag.IsProcessedDirect(iRow,iCol)) continue;		
			
			iSpill = dem.asFloat(iRow, iCol);
			
			if (iSpill <= node.spill) 	{
				if (!bInPQ) {
					//decide  whether (iRow, iCol) is a true border cell
					isBoundary=true;
					for (j = 0; j < 8; j++)
					{
						jRow = Get_rowTo(j, iRow);
						jCol = Get_colTo(j, iCol);
						if (flag.IsProcessedDirect(jRow,jCol) && dem.asFloat(jRow,jCol)<iSpill)
						{
							isBoundary=false;
							break;
						}
					}
					if (isBoundary) {
						priorityQueue.push(node);
						bInPQ=true;
						nPSC++;
					}
				}
				continue; 
			}
			//otherwise
			//N is unprocessed and N is higher than C
			N.col = iCol;
			N.row = iRow;
			N.spill = iSpill;
			traceQueue.push(N);
			flag.SetFlag(iRow,iCol);		
		}
	}
	count+=total-nPSC;
}

void ProcessPit_onepass(CDEM& dem, Flag& flag, queue<Node>& depressionQue,queue<Node>& traceQueue,PriorityQueue& priorityQueue,int& count, int percentFive)
{
	int iRow, iCol,i;
	float iSpill;
	Node N;
	Node node;
	int width=dem.Get_NX();
	int height=dem.Get_NY();
	while (!depressionQue.empty())
	{
		node= depressionQue.front();
		depressionQue.pop();
		count++;
		if (count % percentFive == 0)
		{
			std::cout<<"Progress："<<count / percentFive * 5 <<"%\r";
		}
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i,  node.col);

			if (flag.IsProcessedDirect(iRow,iCol)) continue;		
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill > node.spill) 
			{ //slope cell
				N.row = iRow;
				N.col = iCol;
				N.spill = iSpill;				
				flag.SetFlag(iRow,iCol);
				traceQueue.push(N);
				continue;
			}

			//depression cell
			flag.SetFlag(iRow,iCol);
			dem.Set_Value(iRow, iCol, node.spill);
			N.row = iRow;
			N.col = iCol;
			N.spill = node.spill;
			depressionQue.push(N);
		}
	}
}

void FillDEM_Zhou_OnePass(char* inputFile, char* outputFilledPath)
{
	queue<Node> traceQueue;
	queue<Node> depressionQue;

	//read float-type DEM
	CDEM dem;
	double geoTransformArgs[6];
	std::cout<<"Reading tiff files..."<<endl;
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
	{
		printf("Error occurred while reading GeoTIFF file!\n");
		return;
	}	
	
	std::cout<<"Finish reading data"<<endl;

	time_t timeStart, timeEnd;
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	
	timeStart = time(NULL);
	std::cout<<"Using the one-pass implementation of the proposed variant to fill DEM"<<endl;


	Flag flag;
	if (!flag.Init(width,height)) {
		printf("Failed to allocate memory!\n");
		return;
	}

	PriorityQueue priorityQueue;
	int percentFive;
	int count = 0,potentialSpillCount=0;
	int iRow, iCol, row,col;
	float iSpill,spill;

	InitPriorityQue_onepass(dem,flag,traceQueue,priorityQueue,percentFive);
	while (!priorityQueue.empty())
	{
		Node tmpNode = priorityQueue.top();
		priorityQueue.pop();
		count++;
		if (count % percentFive == 0)
		{
			std::cout<<"Progress："<<count / percentFive * 5 <<"%\r";
		}
		row = tmpNode.row;
		col = tmpNode.col;
		spill = tmpNode.spill;

		for (int i = 0; i < 8; i++)
		{

			iRow = Get_rowTo(i, row);
			iCol = Get_colTo(i, col);

			if (flag.IsProcessed(iRow,iCol)) continue;
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill <= spill)
			{
				//depression cell
				dem.Set_Value(iRow, iCol, spill);
				flag.SetFlag(iRow,iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = spill;
				depressionQue.push(tmpNode);
				ProcessPit_onepass(dem,flag,depressionQue,traceQueue,priorityQueue,count,percentFive);
			}
			else
			{
				//slope cell
				flag.SetFlag(iRow,iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				traceQueue.push(tmpNode);
			}			
			ProcessTraceQue_onepass(dem,flag,traceQueue,priorityQueue,count,percentFive);
		}
	}
	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	std::cout<<"Time used："<<consumeTime<<" seconds"<<endl;

	//计算统计量
	double min, max, mean, stdDev;
	calculateStatistics(dem, &min, &max, &mean, &stdDev);
	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(), 
	(void *)dem.getDEMdata(),GDALDataType::GDT_Float32, geoTransformArgs,
	&min, &max, &mean, &stdDev, -9999);
	return;
}
