// Itr1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include "lru.cpp"
using namespace std;

string dataPath = "C:/Users/Swaraj/Documents/MemCache/FileMemCache/";

void OpenFile(ifstream &obj, string path, string filename, bool showMessage);
int ReadIntoQueue(ifstream &obj, deque<string> &queue);

//void UserInput(int &mapper_count, int &reducer_count, string &keyword);

namespace multithread {
	deque<string> file_queue; //contains filename that will be read
	deque<string> file_queue2; //contains filename that will be written
	deque<int> count_queue; //contains the occurence of a keyword from files
	int currentMapperWorking = 0; //shows how many mappers are processing right now. This will prevent reducers from early stop.
	mutex fileQueueMTX;
	mutex countQueueMTX;
	mutex consoleOutMTX;//no interrupt during printing message in terminal
	class mapper {
		int id;
		thread _th;
		ifstream file;
		string dataPath;
		string filename;
		string keyword;
		int countLineByLine();
		void start();
		int countSubstring(string &line);
		vector<int> GetPosition(ifstream &obj);
		void ProduceReaderOut(string &filename, string &value, bool check);
	public:
		mapper(string d, string k, int i, string readername) {
			id = i;
			dataPath = d;
			keyword = k;
			_th = thread(&mapper::mapper::start, this); //passing the thread filename here so we can read it again 
		}
		~mapper() {
			if (_th.joinable()) {
				_th.join();
			}
		}
		void join() {
			_th.join();
		}
	};
	class reducer {
		int id;
		thread _th;
		string filename;
		/*vector<int>*/ unordered_map<string, string> GetPosition(ifstream &obj);
		void ProduceWriterOut(string &filename, string &datapath, string &value);
		void startWriter();
	public:
		reducer(int i) {
			id = i;
			_th = thread(&reducer::startWriter, this);
		}
		~reducer() {
			if (_th.joinable()) {
				_th.join();
			}
		}
		void join() {
			_th.join();
		}
	};

}



int main(int argc, char** argv) {
	int N_mapper, N_reducer;
	string keyword;
	string dataPath = "C:/Users/Swaraj/Documents/MemCache/FileMemCache/"; //hardcoded datapath
	ifstream FILE, FILE2;
	OpenFile(FILE, dataPath, "Reader.txt", true); //OpenFile
	N_mapper = ReadIntoQueue(FILE, multithread::file_queue);
	FILE.close();
	//=======================Writter========================
	OpenFile(FILE2, dataPath, "Writer.txt", true); //OpenFile
	N_reducer = ReadIntoQueue(FILE2, multithread::file_queue2);
	FILE2.close();

	//UserInput(N_mapper, N_reducer, keyword);
	//cout << "Keyword: " << keyword << endl;
	//multithread::currentMapperWorking = N_mapper;
	
	chrono::system_clock::time_point start = chrono::system_clock::now();
	vector<multithread::mapper*> mappers;
	for (int i = 0; i<N_mapper; i++) {
		multithread::mapper *ptr = new multithread::mapper(dataPath, keyword, i, multithread::file_queue.at(i)); //passing the file string to read
		mappers.push_back(ptr);
	}
	
	vector<multithread::reducer*> reducers;
	for (int i = 0; i<N_reducer; i++) {
		multithread::reducer *ptr = new multithread::reducer(i);
		reducers.push_back(ptr);
	}

	for (int i = 0; i<N_mapper; i++) {
		mappers[i]->join();
	}
	for (int i = 0; i<N_reducer; i++) {
		reducers[i]->join();
	}
	chrono::system_clock::time_point end = chrono::system_clock::now();

	cout << "=============" << endl;

	cout << "Total count of \"" << keyword << "\": " << multithread::count_queue.front() << endl;
	cout << "Time elapse: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
	return 0;
}
int ReadIntoQueue(ifstream &obj, deque<string> &queue) /*purpose of deque unknown*/ {
	string tmp;
	while (getline(obj, tmp)) {
		queue.push_back(tmp); //push file name into the queue
	}
	return queue.size();
}


void OpenFile(ifstream &obj, string path, string filename="Reader.txt", bool showMessage=true) {
	obj.open(path + filename);
	if (showMessage) {
		cout << "Opening " << filename << "...";
		if (obj.is_open()) {
			cout << "[Sucessful]" << endl;
		}
		else {
			cout << "[Fail]" << endl;
		}
	}
}

//void UserInput(int &mapper_count, int &reducer_count, string &keyword) {
//	cout << "Enter the number of Mapper: ";
//	cin >> mapper_count;
//	cout << "Enter the number of Reducer: ";
//	cin >> reducer_count;
//	cout << "Enter the keyword: ";
//	cin >> keyword;
//}

unordered_map<string, string> multithread::reducer::GetPosition(ifstream &obj)
{
	unordered_map<string, string> map;
	string key, value;
	string tmp, temp;
	while (getline(obj, tmp))
	{
		int cntformap = 0;
		stringstream ss(tmp);
		while (getline(ss, temp, ' '))
		{
			if (cntformap == 0) {
				key = temp;
				cntformap++;
			}
			else {
				value = temp;//store key value pair
			}
		}
		map.insert(make_pair(key, value));
		//return key value pair
	}
	return map;
}


void multithread::reducer::startWriter() {
	while (true) {
		multithread::fileQueueMTX.lock(); //make this exclusive lock shared writer 
		if (multithread::file_queue2.size() != 0) { //check if the queue have filename remaining
			filename = multithread::file_queue2.front();
			//OK till here as we get the filename from the required queue now we want to read the file again
			ifstream FILE;
			//Store the postion in a int/vector for a particular Reader thread
			//Pass the int/vector to cache to check whether already available
			OpenFile(FILE, dataPath, filename, true);
			/*vector<int> newposition =*/ 
			unordered_map<string, string>map=GetPosition(FILE); //return key value pair
			//lru cache
			//Open item file and read from it

			//for (int i = 0; i < newposition.size(); i++)
			//{
			//	if ((lru->Size()) == 3)   //check if cache max size is reached
			//	{
			//		string value = lru->Get(to_string(newposition.at(i)));
			//		ProduceWriterOut(filename, value, true);
			//	}
			//	else
			//	{
			lru_cache<string, string> *lru = new lru_cache<string, string>(3);
			//lru check for key value to save a bad write
			//iterate through map
			unordered_map<string, string>::iterator itr;
			ofstream file;
			file.open(dataPath + "itemfile.txt", ios::app);
			for (itr = map.begin(); itr != map.end(); itr++)
			{
				string sLine;
				OpenFile(FILE, dataPath, "itemfile.txt", true);
				int line_no = 0;
				while (line_no != stoi(itr->first)/*newpos.key/*newposition.at(i)*/ && getline(FILE, sLine)) { //getline() check??
					++line_no;
				}
				if (line_no == stoi(itr->first)/*key*/) {
					// sLine contains the x line in the file.
				//	getline(FILE, sLine);
					//ProduceWriterOut(filename, dataPath, itr->second);
					file << itr->second << endl;
				}
				else {
					// The file contains fewer than x lines.

				}
				//lru->Put(/*to_string(newposition.at(i)*/), sLine);
			}
			//}
		
		//check condition from cache with flag then read from item file it not 
		//and write to Reader*.out file
		multithread::consoleOutMTX.lock();
		cout << "Mapper #" << id + 1 << " reading " << filename << endl;
		multithread::consoleOutMTX.unlock();
		multithread::file_queue2.pop_front();
		}
		else {//if not, unlock the mutex and return the function
			fileQueueMTX.unlock();
			multithread::currentMapperWorking--;
			return;
		}
		multithread::fileQueueMTX.unlock();
		//while (true) {
		//	int sum, a, b;
		//	multithread::countQueueMTX.lock();
		//	//cout << "lock after" << endl;
		//	if (count_queue.size()>1) {
		//		//cout << "size > 1 before" << endl;
		//		a = count_queue.front();//[a,b,c....]
		//		count_queue.pop_front();//[b,c....]
		//		b = count_queue.front();//[b,c....]
		//		count_queue.pop_front();//[c....]
		//								//multithread::consoleOutMTX.lock();
		//		sum = a + b;
		//		cout << "Reducer #" << id + 1 << " pushing " << a << "+" << b << "=" << sum << " into count_queue" << endl;
		//		//multithread::consoleOutMTX.unlock();
		//		count_queue.push_back(sum);//[c....,sum];
		//		multithread::countQueueMTX.unlock();
		//	}
		//	else if (multithread::currentMapperWorking == 0) {
		//		multithread::countQueueMTX.unlock();
		//		return;
		//	}
		//	else {
		//		multithread::countQueueMTX.unlock();
		//	}
		//}
	}
}

	void multithread::reducer::ProduceWriterOut(string &filename, string &dataPath, string &value)
	{
		ofstream file;
		file.open(dataPath + filename);
		int line_no = 0;
		//while (line_no != newpos.key/*newposition.at(i)*/ && getline(FILE, sLine)) {
		//	++line_no;
		//}
		//if (line_no == key) {
		//	// sLine contains the x line in the file.
		//	//	getline(FILE, sLine);
		//	file << value;
		//}
		
	}

/** thread safe function **/
/** All variabes are local variable ***/
/** no longer use **/
/*
int multithread::mapper::countSubstring(string &line){
int sum=0;
int currentIndex = 0;
bool isContinuous = true;
map<char,bool> mapping;
for(int i=0;i<line.size();i++){
if(line[i]==keyword[0]){
int match=1;
for(int j=1;j<keyword.size() && (i+j)<line.size();j++){
if(keyword[j]==line[i+j]){
match++;
}
else{
break;
}
}
if(match == keyword.size()){
sum++;
i+=keyword.size()-1;
}
}
}
return sum;
}
*/
void multithread::mapper::ProduceReaderOut(string& filename, string &value, bool check)
{
	ofstream file;
	file.open(filename + "out.txt", ios::app);
	if (check)
	{
		file << value + "Cache" << endl;
	}
	else {
		file << value + "Disk"<< endl;
	}
	file.close();
}

vector<int> multithread::mapper::GetPosition(ifstream &obj)
{
	string tmp;
	vector<int> position;
	while (getline(obj, tmp)) {
		position.push_back(stoi(tmp)); //push file name into the queue
	}
	return position;
}

int multithread::mapper::countLineByLine() {
	int count = 0;
	string word;
	/* old version
	while(getline(file,line)){
	count += countSubstring(line);
	}
	*/
	while (file >> word) {
		if (keyword == word)
			count++;
	}
	return count;
}
void multithread::mapper::start() {
	//multithread::currentMapperWorking++;
	while (true) {
		multithread::fileQueueMTX.lock(); //make this lock shared reader 
		if (multithread::file_queue.size() != 0) { //check if the queue have filename remaining
			filename = multithread::file_queue.front();
			//OK till here as we get the filename from the required queue now we want to read the file again
			ifstream FILE;
			//Store the postion in a int/vector for a particular Reader thread
			//Pass the int/vector to cache to check whether already available
			OpenFile(FILE, dataPath, filename, true);
				vector<int> newposition = GetPosition(FILE);
			//lru cache
			//Open item file and read from it
			lru_cache<string, string> *lru = new lru_cache<string, string>(3);
			ofstream file(filename + "out.txt", ios::trunc);
			for (int i = 0; i < newposition.size(); i++)
			{ 
				if ((lru->Size()) == 3)   //check if cache max size is reached
				{
					string value = lru->Get(to_string(newposition.at(i)));
					ProduceReaderOut(filename, value, true);
				}
				else
				{


					ifstream FILE;
					string sLine;
					OpenFile(FILE, dataPath, "itemfile.txt", true);
						int line_no = 0;
					while (line_no != newposition.at(i) && getline(FILE, sLine)) {
						++line_no;
					}
					if (line_no == newposition.at(i)) {
						// sLine contains the x line in the file.
						getline(FILE, sLine);
					}
					else {
						// The file contains fewer than x lines.

					}
					lru->Put(to_string(newposition.at(i)), sLine);
					ProduceReaderOut(filename, sLine, false);
				}
			}
			//check condition from cache with flag then read from item file it not 
			//and write to Reader*.out file
			multithread::consoleOutMTX.lock();
			cout << "Mapper #" << id + 1 << " reading " << filename << endl;
			multithread::consoleOutMTX.unlock();
			multithread::file_queue.pop_front();
		}
		else {//if not, unlock the mutex and return the function
			fileQueueMTX.unlock();
			multithread::currentMapperWorking--;
			return;
		}
		multithread::fileQueueMTX.unlock();
		/* process counting occurence of keyword */
		OpenFile(file, dataPath, filename, false);
		int count = countLineByLine();
		file.close();
		multithread::countQueueMTX.lock();
		multithread::consoleOutMTX.lock();
		cout << "Mapper #" << id + 1 << " pushing " << count << " into count_queue" << endl;
		multithread::consoleOutMTX.unlock();
		multithread::count_queue.push_back(count);
		multithread::countQueueMTX.unlock();
	}
}
