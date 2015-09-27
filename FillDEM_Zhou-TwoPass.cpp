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

typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;
void InitPriorityQue(CDEM& dem, Flag& flag, Flag& flag2, queue<Node>& traceQueue, PriorityQueue& priorityQueue,int& percentFive)
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

						flag.SetFlags(row,col,flag2);
						break;
					}
				}
			}
			else{
				flag.SetFlags(row,col,flag2);
			}
		}
	}

	percentFive = validElementsCount / 20;
}
void ProcessTraceQue(CDEM& dem,Flag& flag, Flag& flag2,queue<Node>& traceQueue, PriorityQueue& priorityQueue,int& count, int percentFive)
{
	int iRow, iCol,i;
	float iSpill;
	Node N,node,headNode;
	int width=dem.Get_NX();
	int height=dem.Get_NY();	
	queue<Node> traceQueue2(traceQueue);
	int total=0;
	while (!traceQueue.empty())
	{
		node = traceQueue.front();
		traceQueue.pop();
		total++;
		if ((count+total/2) % percentFive == 0)
		{
			std::cout<<"Progress:"<<(count+total/2) / percentFive * 5 <<"%\r";
		}

 		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i, node.col);
			if (flag.IsProcessedDirect(iRow,iCol)) continue;		
			
			iSpill = dem.asFloat(iRow, iCol);
			
			if (iSpill <= node.spill) 
				continue;  

			//slope cell
			N.col = iCol;
			N.row = iRow;
			N.spill = iSpill;
			traceQueue.push(N);
			flag.SetFlag(iRow,iCol);		
		}
	}
	int nPSC=0;
	int count0=count;
	count+=total/2;
	total=0;
	bool bInPQ=false;
	while (!traceQueue2.empty())
	{
		node = traceQueue2.front();
		traceQueue2.pop();
		total++;
		if ((count+total/2) % percentFive == 0)
		{
			std::cout<<"Progress:"<<(count+total/2) / percentFive * 5 <<"%\r";
		}

		bInPQ=false;
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i, node.col);
			if (flag2.IsProcessedDirect(iRow,iCol)) continue;					

			if (flag.IsProcessedDirect(iRow,iCol)){
				N.col = iCol;
				N.row = iRow;
			    flag2.SetFlag(iRow,iCol);
				traceQueue2.push(N);
			}
			else {
				if (!bInPQ) {
					node.spill=dem.asFloat(node.row, node.col);
					priorityQueue.push(node);				
					bInPQ=true;
					nPSC++;
				}
			}
		}
	}	
	count=count0+total-nPSC;
}

void ProcessPit(CDEM& dem, Flag& flag, Flag& flag2,queue<Node>& depressionQue,queue<Node>& traceQueue,PriorityQueue& priorityQueue,int& count, int percentFive)
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
			std::cout<<"Progress:"<<count / percentFive * 5 <<"%\r";
		}
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i,  node.col);

			if (flag.IsProcessedDirect(iRow,iCol)) continue;		
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill > node.spill) 
			{   //slope cell
				N.row = iRow;
				N.col = iCol;
				N.spill = iSpill;
				traceQueue.push(N);
				flag.SetFlag(iRow,iCol);
				flag2.SetFlag(iRow,iCol);
				continue;
			}

			//depressio cell
			flag.SetFlags(iRow,iCol,flag2);
			dem.Set_Value(iRow, iCol, node.spill);
			N.row = iRow;
			N.col = iCol;
			N.spill = node.spill;
			depressionQue.push(N);
		}
	}
}

void FillDEM_Zhou_TwoPass(char* inputFile, char* outputFilledPath)
{
	queue<Node> traceQueue;//追踪队列
	queue<Node> depressionQue;//洼地点列表

	//读入数据
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
	std::cout<<"Using the two-pass implementation of the proposed variant to fill DEM"<<endl;


	Flag flag;
	if (!flag.Init(width,height)) {
		printf("Out of memory!\n");
		return;
	}

	Flag flag2;
	if (!flag2.Init(width,height)) {
		printf("Failed to allocate memory!\n");
		return;
	}

	PriorityQueue priorityQueue;
	int percentFive;
	int count = 0,potentialSpillCount=0;
	int iRow, iCol, row,col;
	float iSpill,spill;

	InitPriorityQue(dem,flag,flag2,traceQueue,priorityQueue,percentFive);
	while (!priorityQueue.empty())
	{
		Node tmpNode = priorityQueue.top();
		priorityQueue.pop();
		count++;
		if (count % percentFive == 0)
		{
			std::cout<<"Progress:"<<count / percentFive * 5 <<"%\r";
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
				flag.SetFlags(iRow,iCol,flag2);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = spill;
				depressionQue.push(tmpNode);
				ProcessPit(dem,flag,flag2,depressionQue,traceQueue,priorityQueue,count,percentFive);
			}
			else
			{
				//slope cell
				flag.SetFlags(iRow,iCol,flag2);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				traceQueue.push(tmpNode);
			}			
			ProcessTraceQue(dem,flag,flag2,traceQueue,priorityQueue,count,percentFive);
		}
	}
	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	std::cout<<"Time used:"<<consumeTime<<" seconds"<<endl;

	//计算统计量
	double min, max, mean, stdDev;
	calculateStatistics(dem, &min, &max, &mean, &stdDev);

	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(), 
		(void *)dem.getDEMdata(),GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return;
}
