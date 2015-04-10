#include <iostream>
#include <list>
#include <string.h>
using namespace std;

typedef struct tagTRange
{
	int x;
	int y;
	
	tagTRange(){ clean(); }
	tagTRange(int nx, int ny) { x = nx; y = ny; }
	void clean(){ x = 0; y = 0; }
	
	tagTRange& operator=(const struct tagTRange &range)
	{
		clean();
		
		x = range.x;
		y = range.y;
		
		return *this;
	}
}TRange;

static std::list<TRange> rangeList;

bool Merge(TRange &tRange1, TRange &tRange2);
void Composite();
void PrintRangeList();
void SortRangeList();
bool Compare(TRange &range1, TRange &rang2);//自定义比较器

int main()
{
	//{1,3}{7,10}
	TRange range1(1, 3);
	TRange range2(7, 10);
	
	//insert {2, 5}
	TRange rangeInsert(2, 6);
	
	rangeList.push_back(range1);
	rangeList.push_back(range2);
	rangeList.push_back(rangeInsert);
	
	Composite();
	
	cout<<"***** finished ******\n";
	
	PrintRangeList();
	
	return 0;
}

void Composite()
{
	SortRangeList();
	
	TRange tRange1;
	TRange tRange2;
	std::list<TRange>::iterator it = rangeList.begin();
	std::list<TRange>::iterator it_limit = --rangeList.end();
	for(it; it != it_limit; )
	{
		std::list<TRange>::iterator it_tmp1 = it;
		std::list<TRange>::iterator it_tmp2 = ++it;
		
		tRange1 = *it_tmp1;
		tRange2 = *it_tmp2;
		
		if( Merge(tRange1, tRange2) )//进行了合并操作
		{
			rangeList.erase(it_tmp1);
			rangeList.erase(it_tmp2);
			
			rangeList.push_back(tRange1);
			
			Composite();
			break;
		}
	}
}

bool Merge(TRange &tRange1, TRange &tRange2)
{
	//合并结果存放到tRange1中
	
	if( (tRange2.x <= tRange1.x) && (tRange2.y >= tRange1.y) )//tRange2完全包含tRange1
	{
		tRange1.x = tRange2.x;
		tRange1.y = tRange2.y;
		return true;
	}
	else if( (tRange2.x >= tRange1.x) && (tRange2.y <= tRange1.y) )//tRange1完全包含了tRange2
	{
		return true;
	}
	else if( (tRange2.x < tRange1.x) && (tRange1.x <= tRange2.y) && (tRange2.y <= tRange1.y) )//2的x比1的小，2的y落在1的区间内
	{
		tRange1.x = tRange2.x;
		return true;
	}
	else if( tRange2.y < tRange1.x )//2的区间完全比1的小
	{
		if( tRange2.y == tRange1.x-1 )//区间相邻，合并
		{
			tRange1.x = tRange2.x;
			return true;
		}
		
		return false;
	}
	else if( tRange2.x > tRange1.y )//2的区间完全比1的大
	{
		if( tRange1.y == tRange2.x-1 )//区间相邻，合并
		{
			tRange1.y = tRange2.y;
			return true;
		}
		
		return false;
	}
	else if( (tRange1.x < tRange2.x) && (tRange2.x < tRange1.y) && (tRange1.y < tRange2.y) )//2的x落在1的区间内，2的y比1dy大
	{
		tRange1.y = tRange2.y;
		return true;
	}
}

void PrintRangeList()
{
	std::list<TRange>::iterator it = rangeList.begin();
	for( it; it != rangeList.end(); it++)
	{
		cout<<it->x<<", "<<it->y<<endl;
	}
}

bool Compare(TRange &range1, TRange &rang2)
{
	if( range1.x <= rang2.x )
	{
		return false;
	}
	
	return true;
}

void SortRangeList()
{
	rangeList.sort( Compare );
}
