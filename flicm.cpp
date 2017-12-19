#include <iostream>
#include <vector>
#include <ctime>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

//����������ģ�cNumΪ�������ĵ���Ŀ��mΪ���������ٶȵĲ�����ͨ��Ϊ2
void calcCenters(Mat& image, vector<Mat>& U, int cNum, double m, double* center)
{
	double sSum;
	double sum;
	for (int k = 0; k < cNum; k++)
	{
		sSum = 0;
		sum = 0;
		for (int i = 0; i < image.rows; i++)
			for (int j = 0; j < image.cols; j++)
			{
				sSum += pow(U[k].at<double>(i,j), m);
				sum += pow(U[k].at<double>(i,j), m) * (double)(image.at<uchar>(i,j));
			}
		center[k] = sum / sSum;	//�õ���������
	}
}

void FLICM(Mat& image, vector<Mat>& U, double m, int cNum, int winSize, int maxIter, double thrE, int& iter)
{
	int sStep = (winSize - 1) / 2;
	double* center = new double[cNum];
	calcCenters(image, U, cNum, m, center);
	double dMax = 10.0;	// �ȳ�ʼ��һ���ϴ��ֵ��|U_new-U_old|
	// vectorĬ�ϸ��ƹ��캯��Ϊǳ���������Լ�����
	Mat tmp(image.size(), CV_64FC1);	// ����ʱ��ʹ�õ���ʱ����
	vector<Mat> Uold;
	for (int k = 0; k < cNum; k++)
	{
		for (int i = 0; i < image.rows; i++)
			for (int j = 0; j < image.cols; j++)
				tmp.at<double>(i,j) = U[k].at<double>(i,j);
		Uold.push_back(tmp);
	}

	vector<double> d1(cNum, 0);	// d1Ϊ��ʽ��17���е�G_ki
	vector<double> d2(cNum, 0);	// d2Ϊ��ʽ��19���еķ�ĸ��ǰ�벿��
	double sSum = 0;
	double dd;
	int x, y;	// �����x, y
	double dist = 0.0;	// ����
	double val = 0;	// ���صĻҶ�ֵ
	double* cenOld = new double[cNum];
	while (dMax > thrE && iter < maxIter)	// ����6
	{
		for (int i = 0; i < image.rows; i++)
		{
			for (int j = 0; j < image.cols; j++)
			{
				for (int k = 0; k < cNum; k++)
				{
					sSum =  4.9407e-324;	//����sSum
					for (int ii = -sStep; ii <= sStep; ii++)	// sStep�Ǵ��ڰ뾶��ii��jj�ǶԾֲ��������
						for (int jj = -sStep; jj <= sStep; jj++)
						{
							x = i + ii;		// �д�˼��
							y = j + jj;
							dist = sqrt(pow((x - i), 2)+ pow((y - j), 2));	// �ֲ������У� ��(x, y) �� (i, j)�ľ���
							// ��x,y�����ܳ����߽磬Ҳ�����루i,j���غ�
							if ( x >= 0 && x < image.rows && y >= 0 && y < image.cols && (ii != 0 || jj != 0))
								{
									val = (double)image.at<uchar>(x, y);	 // i�������j��ĻҶ�ֵ����ʽ��17��
									sSum = sSum + 1.0 / (1.0 + dist) * (1 - pow(Uold[k].at<double>(i, j), m)) * pow(abs(val - center[k]), 2);
								}	
						}
					d1[k] = sSum;
					d2[k] = pow(abs((double)image.at<uchar>(i,j) - center[k]), 2);
				}
				for (int k = 0; k < cNum; k++)
				{
					if (d1[k] == 0)
						d1[k] =  4.9407e-324;	 // �ӽ���0�ļ�Сֵ
					if (d2[k] == 0)
						d2[k] =  4.9407e-324;	 // �ӽ���0�ļ�Сֵ
				}
				for (int k = 0; k < cNum; k++)
				{
					dd = d1[k] + d2[k];	// ÿ��k���õ���
					sSum =  4.9407e-324;
					for (int ii = 0; ii < cNum; ii++)
						sSum = sSum + pow((dd / (d1[ii] + d2[ii])), (1.0 / (m - 1.0)));
					U[k].at<double>(i,j) = 1.0 / sSum;
				}
			}	// end for j
		}	// end for i
		for (int k = 0; k < cNum; k++)
			cenOld[k] = center[k];
		calcCenters(image, U, cNum, m, center);	 // ���þ������Ⱦ��󣬼����ʼ��������
		for (int k = 0; k < cNum; k++)
		{
			if (dMax < abs(cenOld[k] - center[k]))
				dMax = abs(cenOld[k] - center[k]);
			Uold[k] = U[k];
		}
		cout << "��" << iter << "�ε���" << endl;
		iter++; // ��¼��������
	}	// end for while

	delete [] center;
	delete [] cenOld;
}

void Flicm_Cluster(Mat& img, Mat& out_img, int cNum, double m, int winSize, int maxIter, double thrE, int& iter)
{
	Mat gray;
	img.copyTo(gray);
	Mat image(img.size(), CV_64FC1);
	if (gray.channels() > 1)	// ת��Ϊ�Ҷ�ͼ
	{
		cvtColor(gray, gray, CV_RGB2GRAY);
		image = gray;	// ת��Ϊ0-255��double��
	}	
	else
		image = img;
	// �����ʼ�������Ⱦ���U,��СΪ(H,W,cNum)
	vector<Mat> U;	// �����Ⱦ���U
	// һ��Ҫ��ǰָ��U��Ԫ�ص����ͺʹ�С��
	for (int k = 0; k < cNum; k++)
	{
		Mat u(img.size(), CV_64FC1);
		U.push_back(u);
	}

	Mat col_sum(image.rows, image.cols, CV_64FC1);
	// һ��Ҫ��ʼ����
	for (int i = 0; i < image.rows; i++)
		for (int j = 0; j < image.cols; j++)
			col_sum.at<double>(i, j) = 0.0;

	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
			for (int k = 0; k < cNum; k++)
			{
				U[k].at<double>(i,j) = rand() / (double)(RAND_MAX+1.0);	// ����һ��0~1֮���С��
				col_sum.at<double>(i,j) += U[k].at<double>(i,j);
			}
	}
	for (int k = 0; k < cNum; k++)
		divide(U[k], col_sum, U[k]);

	FLICM(image, U, m, cNum, winSize, maxIter, thrE, iter);
	
	// ������������ȣ��б�ÿ�����ص��������
	Mat clus(image.size(), CV_8UC1);
	int max_clus;
	for (int i = 0; i < clus.rows; i++)
		for (int j = 0; j < clus.cols; j++)
		{
			max_clus = 0;
			for (int k = 1; k < cNum; k++)
			{
				if (U[k].at<double>(i,j) > U[k-1].at<double>(i,j))
					max_clus = k;
			}
			clus.at<uchar>(i,j) = max_clus;
		}
		// ��ʾ����Ľ��ͼƬ
		for (int i = 0; i < out_img.rows; i++)
			for (int j = 0; j < out_img.cols; j++)
			{
				/*for (int k = 0; k < cNum; k++)
					if (clus.at<uchar>(i,j) == k)
					{
						out_img.at<cv::Vec3b>(i,j)[0] = 255*k;
						out_img.at<cv::Vec3b>(i,j)[1] = 255*k;
						out_img.at<cv::Vec3b>(i,j)[2] = 255*k;
					}*/
				if (clus.at<uchar>(i,j) == 0)
				{
					out_img.at<cv::Vec3b>(i,j)[0] = 0;
					out_img.at<cv::Vec3b>(i,j)[1] = 255;
					out_img.at<cv::Vec3b>(i,j)[2] = 0;
				}

				else if (clus.at<uchar>(i,j) == 1)
				{
					out_img.at<cv::Vec3b>(i,j)[0] = 0;
					out_img.at<cv::Vec3b>(i,j)[1] = 0;
					out_img.at<cv::Vec3b>(i,j)[2] = 255;
				}
				else
				{
					out_img.at<cv::Vec3b>(i,j)[0] = 255;
					out_img.at<cv::Vec3b>(i,j)[1] = 255;
					out_img.at<cv::Vec3b>(i,j)[2] = 255;
				}
			}
}

int main()
{
	// ��������ʱ��
	clock_t start, end;
	start = clock();

	// ���þ����㷨��һЩ��ʼ����
	int cNum = 3;	// ������������
	double m = 2;	// ģ��ָ��m
	int winSize = 3;	// �ֲ�����ֱ��
	int maxIter = 100;	// ����������
	double thrE =  0.00001;	  // ������ֵ
	Mat img = imread("brain.tif");
	Mat out_img(img.size(), CV_8UC3);
	int iter = 0;
	Flicm_Cluster(img, out_img, cNum, m, winSize, maxIter, thrE, iter);
	cout << "�ܹ�������" << iter << "��" << endl;
	imshow("dst", out_img);
	//imwrite("1_8_1.jpg", out_img);

	// ��ʾ���ж���ʱ��
	end = clock();
	double total_time = 0;
	total_time = (end - start) / CLOCKS_PER_SEC;
	cout << "����" << iter << "����Ҫ����" << total_time << "��" << endl; 

	waitKey();

	return 0;
}