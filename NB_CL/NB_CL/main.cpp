// NBCL.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>
#include <cmath>
#include <cstdlib>
using namespace std;

#define ANGER 1
#define DISGUST 2
#define FEAR 3
#define JOY 4
#define SAD 5
#define SURPRISE 6
const string EMOTION[] = { "nah","anger","disgust","fear","joy","sad", "surprise"};
int emotion_to_index(const string& emotion)
{
	if (emotion == "anger") { return 1; }
	else if(emotion == "disgust") { return 2; }
	else if (emotion == "fear") { return 3; }
	else if (emotion == "joy") { return 4; }
	else if (emotion == "sad") { return 5; }
	else if (emotion == "surprise") { return 6; }
	else return 0;
}
//在 string向量里找出字符串str，返回索引
int find_word_in_vc(string &str, vector<string> &string_vc)
{
	vector<string>::iterator it = find(string_vc.begin(), string_vc.end(), str);
	if (it == string_vc.end())
	{
		return -1;
	}
	return it - string_vc.begin();
}

class trainRow
{
public:
	int number_of_words;
	int dictSize;
	int* data;
	int emotion;//anger,disgust,fear,joy,sad,surprise

	trainRow();
	trainRow(int dictSize);
	trainRow(const trainRow& TR);//带指针数组的类一定要有拷贝构造函数
	~trainRow();
};
trainRow::trainRow()
{
	dictSize = 0;
	number_of_words = 0;
	emotion = 0;
}
trainRow::trainRow(int dictSize)
{
	this->dictSize = dictSize;
	data = new int[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		data[i] = 0;
	}
	emotion = 0;
}
trainRow::trainRow(const trainRow& TR)
{
	dictSize = TR.dictSize;
	data = new int[TR.dictSize];
	for (int i = 0; i < TR.dictSize; i++)
	{
		data[i] = TR.data[i];
	}
	emotion = TR.emotion;
}
trainRow::~trainRow()
{
	delete[] data;
}

class trainCase
{
public:
	int dictSize;			//不重复记录单词数，矩阵的列数 
	int rowCnt;				//一共多少训练用的文章，矩阵的行数 
	vector<string> wordsVC;	//不重复记录单词 
	vector<trainRow*> matrix;//onehot矩阵 TF矩阵 感情向量

	trainCase();
	trainCase(const string &filename);
	~trainCase();
	void get_words(const string &filename);	//获取wordVC向量,同时充当了构造函数
	void write_matrix(const string &filename);//获取onehot矩阵，emotions数组
	friend void operator<<(ostream& os, const trainCase& TC);
};
trainCase::trainCase()
{
	dictSize = rowCnt = 0;
}
trainCase::trainCase(const string &filename)
{
	get_words(filename);//Initialize dictSize/rowCnt/wordsVC
	write_matrix(filename);//Initialize matrix
}
trainCase::~trainCase()
{
	for (int i = 0; i < matrix.size(); i++)
	{
		delete matrix[i];
	}
}
void trainCase::get_words(const string &filename)
{
	rowCnt = 0;
	ifstream fin(filename.c_str());
	string s;
	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		rowCnt++;
		int wordCnt = 0;
		string words, word;
		trainRow* TR = new trainRow();

		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		ss.str(words.c_str());
		while (ss >> word)
		{
			wordCnt++;
			if (find(wordsVC.begin(), wordsVC.end(), word) == wordsVC.end())//从未出现过的单词 
			{
				wordsVC.push_back(word);//记录到vector里
			}
		}
		TR->number_of_words = wordCnt;
		matrix.push_back(TR);
	}
	dictSize = wordsVC.size();
}
void trainCase::write_matrix(const string &filename)
{
	//打开要读的文件 
	ifstream fin(filename);
	int currRow = 0;//当前处理的行号
	string s;//用于记录读取到的每一行的字符串 

	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		matrix[currRow]->dictSize = dictSize;
		matrix[currRow]->data = new int[dictSize];
		for (int i = 0; i < dictSize; i++)
		{
			matrix[currRow]->data[i] = 0;
		}

		string words, word;
		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		istringstream Wss(words);
		while (Wss >> word)//获得当前行的记录 
		{
			//在 string_vc里找出outstr，返回索引
			int location = find_word_in_vc(word, wordsVC);
			if (location != -1)
			{
				matrix[currRow]->data[location] = 1;
			}
		}
		for (int i = 0; i < 6; i++)
		{
			string tmpEmotion;
			getline(ss, tmpEmotion, ',');
			matrix[currRow]->emotion = emotion_to_index(tmpEmotion);
		}
		currRow++;
	}
}
void operator<<(ostream& os, const trainCase& TC)
{
	for (int i = 0; i < TC.rowCnt; i++)
	{
		for (int j = 0; j < TC.dictSize; j++)
		{
			os << TC.matrix[i]->data[j] << ' ';
		}
		os << endl;
		os << EMOTION[TC.matrix[i]->emotion];
		os << endl;
	}
}


int main()
{
    return 0;
}

