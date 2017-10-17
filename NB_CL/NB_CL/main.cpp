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
#include <Windows.h>
using namespace std;

#define ANGER 0
#define DISGUST 1
#define FEAR 2
#define JOY 3
#define SAD 4
#define SURPRISE 5
const string EMOTION[] = { "anger","disgust","fear","joy","sad", "surprise" };
int emotion_to_index(const string& emotion)
{
	if (emotion == "anger") { return 0; }
	else if (emotion == "disgust") { return 1; }
	else if (emotion == "fear") { return 2; }
	else if (emotion == "joy") { return 3; }
	else if (emotion == "sad") { return 4; }
	else if (emotion == "surprise") { return 5; }
	else return 5;
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
void normalize_6(double* x)
{
	double sum = 0;
	for (int i = 0; i < 6; i++)
	{
		sum += x[i];
	}
	for (int i = 0; i < 6; i++)
	{
		x[i] = x[i] / sum;
	}
}
double mean(double* data, int size)
{
	double res = 0;
	for (int i = 0; i < size; i++)
	{
		res += data[i];
	}
	res = res / size;
	return res;
}
double variance(double* data, int size)
{
	double m = mean(data, size);
	double res = 0;
	for (int i = 0; i < size; i++)
	{
		res += pow(data[i] - m, 2);
	}
	return res;
}
double Maximum(double* data, int size)
{
	double max = data[0];
	for (int i = 1; i < size; i++)
	{
		if (data[i] > max)
		{
			max = data[i];
		}
	}
	return max;
}
double Minimum(double* data, int size)
{
	double min = data[0];
	for (int i = 1; i < size; i++)
	{
		if (data[i] < min)
		{
			min = data[i];
		}
	}
	return min;
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
	emotion = -1;
}
trainRow::trainRow(int dictSize)
{
	this->dictSize = dictSize;
	data = new int[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		data[i] = 0;
	}
	emotion = -1;
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
	int* emotion_row_count;	//每个感情在训练集里出现的次数,是一个有6个值的数组
	int* emotion_word_count_unique;//每个感情中不重复的单词数
	int* emotion_word_count;
	vector<string> wordsVC;	//不重复记录单词 
	vector<trainRow*> matrix;//onehot矩阵 TF矩阵 感情向量
	vector<string> wordsVC_emotion[6];//不重复记录一个感情的单词

	trainCase();
	trainCase(const string &filename);
	~trainCase();
	void get_words(const string &filename);		//获取wordVC向量,同时充当了构造函数
	void write_matrix(const string &filename);	//获取onehot矩阵，emotions数组
	int count_Bern(string &word, int emotion);	//对某一感情进行单词计数，将用于伯努利模型
	int count_multi(string &word, int emotion);	//对某一感情进行单词计数，将用于多项式/词袋模型
	friend void operator<<(ostream& os, const trainCase& TC);
};
trainCase::trainCase()
{
	dictSize = rowCnt = 0;
	emotion_row_count = new int[6];
	emotion_word_count_unique = new int[6];
	emotion_word_count = new int[6];
	for (int i = 0; i < 6; i++)
	{
		emotion_row_count[i] = 0;
		emotion_word_count_unique[i] = 0;
		emotion_word_count[i] = 0;
	}
}
trainCase::trainCase(const string &filename)
{
	dictSize = rowCnt = 0;
	emotion_row_count = new int[6];
	emotion_word_count_unique = new int[6];
	emotion_word_count = new int[6];
	for (int i = 0; i < 6; i++)
	{
		emotion_row_count[i] = 0;
		emotion_word_count_unique[i] = 0;
		emotion_word_count[i] = 0;
	}
	get_words(filename);//Initialize dictSize/rowCnt/wordsVC
	write_matrix(filename);//Initialize matrix
}
trainCase::~trainCase()
{
	for (int i = 0; i < matrix.size(); i++)
	{
		delete matrix[i];
	}
	delete[] emotion_row_count;
	delete[] emotion_word_count_unique;
	delete[] emotion_word_count;
}
void trainCase::get_words(const string &filename)
{
	int currRow = 0;
	ifstream fin(filename.c_str());
	string s;
	getline(fin, s);//去掉说明 
	
	while (getline(fin, s))
	{
		int wordCnt = 0;
		string words, word, tmpEmotion;
		trainRow* TR = new trainRow();

		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		getline(ss, tmpEmotion);
		int currEmotion = emotion_to_index(tmpEmotion);
		TR->emotion = currEmotion;
		emotion_row_count[currEmotion] ++;
		ss.clear();
		ss.str(words.c_str());
		while (ss >> word)
		{
			wordCnt++;
			if (find(wordsVC.begin(), wordsVC.end(), word) == wordsVC.end())//从未出现过的单词 
			{
				wordsVC.push_back(word);//记录到vector里
			}

			if (find(wordsVC_emotion[currEmotion].begin(), wordsVC_emotion[currEmotion].end(), word) == wordsVC_emotion[currEmotion].end())
			{
				wordsVC_emotion[currEmotion].push_back(word);
			}
		}
		TR->number_of_words = wordCnt;
		matrix.push_back(TR);

		emotion_word_count[currEmotion] += wordCnt;
		currRow++;
	}
	dictSize = wordsVC.size();
	rowCnt = matrix.size();
	for (int i = 0; i < 6; i++)
	{
		emotion_word_count_unique[i] = wordsVC_emotion[i].size();
	}
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
				matrix[currRow]->data[location] += 1;
			}
		}


		currRow++;
	}
}
int trainCase::count_Bern(string &word, int emotion)
{
	int res = 0;
	int location = find_word_in_vc(word, wordsVC);
	if (location == -1)
	{
		return 0;
	}
	for (int i = 0; i < rowCnt; i++)
	{
		if (matrix[i]->data[location] != 0 && matrix[i]->emotion == emotion)
		{
			res++;
		}
	}
	return res;
}
int trainCase::count_multi(string &word, int emotion)
{
	int res = 0;
	int location = find_word_in_vc(word, wordsVC);
	if (location == -1)
	{
		return 0;
	}
	for (int i = 0; i < rowCnt; i++)
	{
		if (matrix[i]->emotion == emotion)
		{
			res += matrix[i]->data[location];
		}
	}
	return res;
}
void operator<<(ostream& os, const trainCase& TC)
{
	for (int j = 0; j < TC.dictSize; j++)
	{
		os << TC.wordsVC[j] << ' ';
	}
	os << endl;
	for (int i = 0; i < TC.rowCnt; i++)
	{
		for (int j = 0; j < TC.dictSize; j++)
		{
			os << TC.matrix[i]->data[j] << ' ';
		}
		//os << endl;
		os << EMOTION[TC.matrix[i]->emotion];
		os << endl;
	}
	os << "anger" << '\t' << "disgust" << '\t' << "fear" << '\t';
	os << "joy" << '\t' << "sad" << '\t' << "surprise" << endl;
	for (int i = 0; i < 6; i++)
	{
		os << TC.emotion_row_count[i] << '\t';
	}
	os << endl;
	for (int i = 0; i < 6; i++)
	{
		os << TC.emotion_word_count_unique[i] << '\t';
	}
	os << endl;
	for (int i = 0; i < 6; i++)
	{
		os << TC.emotion_word_count[i] << '\t';
	}
	os << endl;
}

class testCase
{
public:
	double* emotion_posibility;
	int* newWords_emotion;

	testCase();
	testCase(const string &words, trainCase &TC, int model, double lp);//0 for bernoulli, 1 for multinomial
	testCase(const testCase& TC);
	~testCase();
	void get_newWords_emotion(const string& words, vector<string>* VC_emotion);
	void score_normalize(double* x);
	void sacling_normalize(double* x);
	int classify();
	void print();
};
testCase::testCase()
{
	emotion_posibility = new double[6];
	newWords_emotion = new int[6];
	for (int i = 0; i < 6; i++)
	{
		emotion_posibility[i] = 1;
	}
}
testCase::testCase(const testCase& TC)
{
	emotion_posibility = new double[6];
	newWords_emotion = new int[6];
	for (int i = 0; i < 6; i++)
	{
		emotion_posibility[i] = TC.emotion_posibility[i];
	}
}
testCase::testCase(const string &words, trainCase &TC, int model, double lp)
{
	emotion_posibility = new double[6];
	newWords_emotion = new int[6];
	get_newWords_emotion(words, TC.wordsVC_emotion);
	double emotion_posibility_de[6];//分母
	double emotion_posibility_nu[6];//分子
	for (int i = 0; i < 6; i++)
	{
		emotion_posibility[i] = 1;
		emotion_posibility_de[i] = 1;
		emotion_posibility_nu[i] = 1;
	}

	vector<string> word_record;
	string word;
	istringstream iss(words);
	while (iss >> word)
	{
		if (find(word_record.begin(),word_record.end(),word) == word_record.end())
		{
			word_record.push_back(word);
			for (int i = 0; i < 6; i++)
			{
				if (model == 0)//伯努利模型
				{
					int cnt = TC.count_Bern(word, i);
					emotion_posibility_nu[i] *= cnt + 1.0*lp;
					emotion_posibility_de[i] *= TC.emotion_row_count[i] + 2.0*lp;
				}
				else //词袋模型
				{
					int cnt = TC.count_multi(word, i);
					emotion_posibility_nu[i] *= (cnt + 1.0*lp);
					emotion_posibility_de[i] *= (newWords_emotion[i] * lp + TC.emotion_word_count_unique[i] * lp + TC.emotion_word_count[i]);
					//cout << word << ": " << cnt << endl;//debug
				}
			}
		}
	}

	//debug
	//for (int i = 0; i < 6; i++)
	//{
	//	//cout << emotion_posibility_nu[i] << ' ' << emotion_posibility_de[i] << endl;
	//	cout << emotion_posibility_nu[i] / emotion_posibility_de[i] << endl;
	//}
	score_normalize(emotion_posibility_nu);
	score_normalize(emotion_posibility_de);
	//for (int i = 0; i < 6; i++)
	//{
	//	//cout << emotion_posibility_nu[i] << ' ' << emotion_posibility_de[i] << endl;
	//	cout << emotion_posibility_nu[i] / emotion_posibility_de[i] << endl;
	//}
	for (int i = 0; i < 6; i++)
	{
		emotion_posibility_nu[i] *= 1.0 * TC.emotion_row_count[i];
		emotion_posibility_de[i] *= 1.0 * TC.rowCnt;
		emotion_posibility[i] *= emotion_posibility_nu[i] / emotion_posibility_de[i];
	}
	//for (int i = 0; i < 6; i++)
	//{
	//	emotion_posibility[i] *= emotion_posibility_nu[i] / emotion_posibility_de[i];
	//}
	//score_normalize(emotion_posibility);
	////normalize_6(emotion_posibility);
	//for (int i = 0; i < 6; i++)
	//{
	//	emotion_posibility[i] *= 1.0*TC.emotion_row_count[i]/ TC.rowCnt;		
	//}
}
testCase::~testCase()
{
	delete[] emotion_posibility;
	delete[] newWords_emotion;
}
void testCase::get_newWords_emotion(const string& words, vector<string>* VC_emotion)
{
	for (int i = 0; i < 6; i++)
	{
		newWords_emotion[i] = 0;
	}
	for (int i = 0; i < 6; i++)
	{
		vector<string> word_record;
		string word;
		istringstream iss(words);
		while (iss >> word)
		{
			if (find(VC_emotion[i].begin(), VC_emotion[i].end(), word) == VC_emotion[i].end() && find(word_record.begin(), word_record.end(), word) == word_record.end())
			{
				word_record.push_back(word);
				newWords_emotion[i]++;
			}
		}
	}
	
}
void testCase::score_normalize(double* x)
{
	//standard score
	double m = mean(x, 6);
	double v = variance(x, 6);

	for (int i = 0; i < 6; i++)
	{
		x[i] = (x[i] - m) / sqrt(v) + 1;
	}
}
void testCase::sacling_normalize(double* x)
{
	double min = Minimum(x, 6);
	double max = Maximum(x, 6);

	for (int i = 0; i < 6; i++)
	{
		x[i] = (x[i] - min) / (max - min);
	}
}
int testCase::classify()
{
	double max_posibility = 0;
	int max_emotion = 0;
	for (int i = 0; i < 6; i++)
	{
		if (emotion_posibility[i] - max_posibility > 0.000000000001)
		{
			max_posibility = emotion_posibility[i];
			max_emotion = i;
		}
	}
	return max_emotion;
}
void testCase::print()
{
	for (int i = 0; i < 6; i++)
	{
		cout << emotion_posibility[i] << ' ';
	}
	cout << endl;
	for (int i = 0; i < 6; i++)
	{
		cout << newWords_emotion[i] << ' ';
	}
	cout << endl;
}

double validHandle(string &valid_file_name, trainCase &TC, int model, double lp)
{
	int right_cnt = 0;
	int all_cnt = 0;
	ifstream fin(valid_file_name);
	string s;
	getline(fin, s);//去掉说明
	while (getline(fin, s))
	{
		string word, words, emotion_ans, emotion_pre;
		istringstream iss(s);
		getline(iss, words, ',');
		getline(iss, emotion_ans);

		//cout << words << endl;//debug

		testCase testcase(words, TC, model, lp);
		emotion_pre = EMOTION[testcase.classify()];
		if (emotion_pre == emotion_ans)
		{
			right_cnt++;
		}
		all_cnt++;

		//debug
		//if (emotion_pre != emotion_ans)
		//{
		//	cout << emotion_pre << ' ' << emotion_ans << endl;
		//}
		//cout << emotion_pre << endl;
		//testcase.print();
		//system("pause");
	}
	return 1.0*right_cnt / all_cnt;
}
void testHandle(ostream &os, string &test_file_name, trainCase &TC, int model, double lp)
{
	int right_cnt = 0, all_cnt = 0;
	ifstream fin(test_file_name);
	string s;
	getline(fin, s);//去掉说明
	while (getline(fin, s))
	{
		string word, words, emotion_pre;
		istringstream iss(s);
		getline(iss, words, ',');
		//cout << words << endl;//debug

		testCase testcase(words, TC, model, lp);
		emotion_pre = EMOTION[testcase.classify()];

		//cout << emotion_pre << endl;
		//testcase.print();
		system("pause");
		//os << emotion_pre << endl;
	}
}
int main()
{
	string train_file_name = "train_set.csv";
	string valid_file_name = "validation_set.csv";
	string test_file_name = "test_set_try.csv";

	trainCase traincase(train_file_name);

	//system("pause");

	int model = 1;
	double lp_start, lp_div, lp_end;
	cout << "lp_start?" << endl;
	cin >> lp_start;
	cout << "lp_div?" << endl; 
	cin >> lp_div;
	cout << "lp_end?" << endl;
	cin >> lp_end;

	double lp = lp_start;
	while (lp - lp_end < 0.0000001)
	{
		double correction = validHandle(valid_file_name, traincase, model, lp);
		cout << lp << ": " << correction << endl;
		lp += lp_div;
		//system("pause");
	}
	
	//train_file_name = "train_set_try.csv";
	//trainCase traincase_try(train_file_name);
	//ofstream try_fout("try_train_matrix.txt");//debug
	//try_fout << traincase_try;//debug
	//testHandle(cout, test_file_name, traincase_try, model,0.1);

	system("pause");
	return 0;
}