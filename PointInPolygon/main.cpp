#include <iostream>
#include <vector>
#include <fstream>
#include "imgproc.hpp"  
#include "highgui.h"  
#include "opencv2/opencv.hpp"  

#define MASK_FLAG 1

using namespace std;
using namespace cv;

struct Point2D{
	int x;
	int y;
};

/*
*   函数功能:判断点是否在多边形内
*   输入参数： vector<Point2D> poly  构成多边形的顶点坐标
*				int x                 需要判断点的横坐标
*				int y                 需要判断点的纵坐标
*   返 回 值： bool                  true为在多边形内，false为不在。也可修改为返回string，用于区分：多边形上、多边形内、多边形外
*/
bool PointInPolygon(vector<Point2D> poly, int x, int y)
{
	bool flag = false;
	int size = poly.size();
	for (int i = 0, j = size - 1; i < size; j = i, i++)
	{
		int x1 = poly[i].x;
		int y1 = poly[i].y;
		int x2 = poly[j].x;
		int	y2 = poly[j].y;

		// 点与多边形顶点重合
		if ((x1 == x && y1 == y) || (x2 == x && y2 == y))
		{
			//return 'on'; // 点在轮廓上
			return true;
		}

		// 判断线段两端点是否在射线两侧
		if ((y1 < y && y2 >= y) || (y1 >= y && y2 < y)) // 只有一边取等号，当射线经过多边形顶点时，只计一次
		{
			// 线段上与射线 Y 坐标相同的点的 X 坐标
			double crossX = (y - y1) * (x2 - x1) / (y2 - y1) + x1;  // y=kx+b变换成x=(y-b)/k 其中k=(y2-y1)/(x2-x1)

			// 点在多边形的边上
			if (crossX == x)
			{
				//return 'on'; // 点在轮廓上
				return true;
			}

			// 右射线穿过多边形的边界，每穿过一次flag的值变换一次
			if (crossX > x)
			{
				flag = !flag;  // 穿过奇数次为true，偶数次为false
			}
		}
	}

	// 射线穿过多边形边界的次数为奇数时点在多边形内
	//return flag ? 'in' : 'out'; // 点在轮廓内或外
	return flag ? true : false;
}

Mat DrowPolygon(vector<Point2D> rois)
{
	// 绘制一块黑布
	Mat canvas = Mat::zeros(Size(512, 512), CV_8UC3);


	// 初始化默认第一个点为最大最小XY
	int minX = rois[0].x;
	int maxX = rois[0].x;
	int minY = rois[0].y;
	int maxY = rois[0].y;
	int tempX = 0;
	int tempY = 0;

	// 寻找ROI边界
	for (vector<Point2D>::const_iterator it = rois.cbegin(); it != rois.cend(); it++)
	{
		tempX = (*it).x;
		tempY = (*it).y;
		if (tempX > maxX)
		{
			maxX = tempX;
		}
		if (tempX < minX)
		{
			minX = tempX;
		}
		if (tempY > maxY)
		{
			maxY = tempY;
		}
		if (tempY < minY)
		{
			minY = tempY;
		}
	}

	// 蓝色边框
	rectangle(canvas, Rect(minX, minY, maxX - minX, maxY - minY), CV_RGB(0, 0, 255), 1);

	// 遍历找到mask
	vector<Point2D> mask;
	int inPolygon = 0;
	int outPolygon = 0;
	int count = 0;

	int dim[3] = { 512, 512, 340 };

	int len = dim[0] * dim[1] * dim[2];
	char *mask8 = new char[len];
	memset(mask8, 0, len*sizeof(char));
	int index = 171 * dim[0] * dim[1];
	for (int i = minY; i <= maxY; i++)
	{
		int rawNum = 0;
		for (int j = minX; j <= maxX; j++)
		{
			count++;
			if (PointInPolygon(rois, j, i))
			{
				Point2D p;
				p.x = j;
				p.y = i;
				mask.push_back(p);
				inPolygon++;
				rawNum++;
				circle(canvas, Point(j, i), 1, CV_RGB(255, 0, 0));
				mask8[index + i*dim[0] + j] = MASK_FLAG;
			}
			else
			{
				outPolygon++;
			}
		}
		cout << "y = " << i << " PointNum:" << rawNum << endl;
	}
	// 输出mask
	FILE *fp;
	fopen_s(&fp, "mask8.raw", "wb");
	fwrite(mask8, sizeof(char), len, fp);
	fclose(fp);
	cout << "minX: " << minX << endl;
	cout << "maxX: " << maxX << endl;
	cout << "minY: " << minY << endl;
	cout << "maxY: " << maxY << endl;

	cout << "count: " << count << endl;
	cout << "inPolygon: " << inPolygon << endl;
	cout << "outPolygon: " << outPolygon << endl;



	//绘制多边形  
	int pointSize = rois.size();
	for (int i = 0; i < pointSize; i++)
	{
		if (i + 1 == pointSize)
		{
			line(canvas, Point(rois[i].x, rois[i].y), Point(rois[0].x, rois[0].y), CV_RGB(0, 255, 0));
		}
		else
		{
			line(canvas, Point(rois[i].x, rois[i].y), Point(rois[i + 1].x, rois[i + 1].y), CV_RGB(0, 255, 0));
		}
	}

	return canvas;
}
void main()
{
	vector<Point2D> polygon;
	Point2D p;

	ifstream in("ori.txt");
	while (in >> p.x >> p.y)
	{
		polygon.push_back(p);
	}
	in.close();

	imshow("Polygon", DrowPolygon(polygon));
	waitKey(0);
}