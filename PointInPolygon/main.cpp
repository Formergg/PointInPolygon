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
*   ��������:�жϵ��Ƿ��ڶ������
*   ��������� vector<Point2D> poly  ���ɶ���εĶ�������
*				int x                 ��Ҫ�жϵ�ĺ�����
*				int y                 ��Ҫ�жϵ��������
*   �� �� ֵ�� bool                  trueΪ�ڶ�����ڣ�falseΪ���ڡ�Ҳ���޸�Ϊ����string���������֣�������ϡ�������ڡ��������
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

		// �������ζ����غ�
		if ((x1 == x && y1 == y) || (x2 == x && y2 == y))
		{
			//return 'on'; // ����������
			return true;
		}

		// �ж��߶����˵��Ƿ�����������
		if ((y1 < y && y2 >= y) || (y1 >= y && y2 < y)) // ֻ��һ��ȡ�Ⱥţ������߾�������ζ���ʱ��ֻ��һ��
		{
			// �߶��������� Y ������ͬ�ĵ�� X ����
			double crossX = (y - y1) * (x2 - x1) / (y2 - y1) + x1;  // y=kx+b�任��x=(y-b)/k ����k=(y2-y1)/(x2-x1)

			// ���ڶ���εı���
			if (crossX == x)
			{
				//return 'on'; // ����������
				return true;
			}

			// �����ߴ�������εı߽磬ÿ����һ��flag��ֵ�任һ��
			if (crossX > x)
			{
				flag = !flag;  // ����������Ϊtrue��ż����Ϊfalse
			}
		}
	}

	// ���ߴ�������α߽�Ĵ���Ϊ����ʱ���ڶ������
	//return flag ? 'in' : 'out'; // ���������ڻ���
	return flag ? true : false;
}

Mat DrowPolygon(vector<Point2D> rois)
{
	// ����һ��ڲ�
	Mat canvas = Mat::zeros(Size(512, 512), CV_8UC3);


	// ��ʼ��Ĭ�ϵ�һ����Ϊ�����СXY
	int minX = rois[0].x;
	int maxX = rois[0].x;
	int minY = rois[0].y;
	int maxY = rois[0].y;
	int tempX = 0;
	int tempY = 0;

	// Ѱ��ROI�߽�
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

	// ��ɫ�߿�
	rectangle(canvas, Rect(minX, minY, maxX - minX, maxY - minY), CV_RGB(0, 0, 255), 1);

	// �����ҵ�mask
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
	// ���mask
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



	//���ƶ����  
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