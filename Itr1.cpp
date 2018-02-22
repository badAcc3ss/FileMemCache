// Itr1.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include "lru.cpp"
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "reader.h"
#include "writer.h"
using namespace std;

//string dataPath = "C:/Users/Swaraj/Documents/MemCache/FileMemCache/";

	int ReadIntoQueue(ifstream &obj, deque<string> &queue);
	void OpenFile(ifstream &obj, string path, string filename, bool showMessage);
	string dataPath = "C:/Users/Swaraj/Documents/MemCache/FileMemCache/";

namespace multithread {
	deque<string> file_queue; //contains filename that will be read
	deque<string> file_queue2; //contains filename that will be written
	deque<int> count_queue; //contains the occurence of a keyword from files
	int currentreaderWorking = 0; //shows how many readers are processing right now. This will prevent writers from early stop.
	boost::shared_mutex _access;
	mutex fileQueueMTX;
	mutex countQueueMTX;
	mutex consoleOutMTX;//no interrupt during printing message in terminal
	}//	class reader {
//		int id;
//		thread _th;
//		ifstream file;
//		string dataPath;
//		string filename;
//		string keyword;
//		int countLineByLine();
//		void start();
//		int countSubstring(string &line);
//		vector<int> GetPosition(ifstream &obj);
//		void ProduceReaderOut(string &filename, string &value, bool check);
//	public:
//		reader(string d, string k, int i, string readername) {
//			id = i;
//			dataPath = d;
//			keyword = k;
//			_th = thread(&reader::reader::start, this); //passing the thread filename here so we can read it again 
//		}
//		~reader() {
//			if (_th.joinable()) {
//				_th.join();
//			}
//		}
//		void join() {
//			_th.join();
//		}
//	};
//	class writer {
//		int id;
//		thread _th;
//		string filename;
//		/*vector<int>*/ unordered_map<string, string> GetPosition(ifstream &obj);
//		//void ProduceWriterOut(string &filename, string &datapath, string &value);
//		void startWriter();
//	public:
//		writer(int i) {
//			id = i;
//			_th = thread(&writer::startWriter, this);
//		}
//		~writer() {
//			if (_th.joinable()) {
//				_th.join();
//			}
//		}
//		void join() {
//			_th.join();
//		}
//	};
//
//}

int main(int argc, char** argv) {
	int N_reader, N_writer;
	string keyword;
	string dataPath = "C:/Users/Swaraj/Documents/MemCache/FileMemCache/"; //hardcoded datapath
	ifstream FILE, FILE2;
	OpenFile(FILE, dataPath, "Reader.txt", true); //OpenFile
	N_reader = ReadIntoQueue(FILE, multithread::file_queue);
	FILE.close();
	//=======================Writter========================
	OpenFile(FILE2, dataPath, "Writer.txt", true); //OpenFile
	N_writer = ReadIntoQueue(FILE2, multithread::file_queue2);
	FILE2.close();
	
	vector</*multithread::*/reader*> readers;
	for (int i = 0; i<N_reader; i++) {
		/*multithread::*/reader *ptr = new /*multithread::*/reader(dataPath, keyword, i, multithread::file_queue.at(i)); //passing the file string to read
		readers.push_back(ptr);
	}
	
	vector</*multithread::*/writer*> writers;
	for (int i = 0; i<N_writer; i++) {
		/*multithread::*/writer *ptr = new /*multithread::*/writer(i);
		writers.push_back(ptr);
	}

	for (int i = 0; i<N_reader; i++) {
		readers[i]->join();
	}
	for (int i = 0; i<N_writer; i++) {
		writers[i]->join();
	}
	//
	//cout << "=============" << endl;

	//cout << "Total count of \"" << keyword << "\": " << multithread::count_queue.front() << endl;
	//cout << "Time elapse: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
	//return 0;
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

unordered_map<string, string> /*multithread::*/writer::GetPosition(ifstream &obj)
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

void /*multithread::*/writer::startWriter() {
	while (true) {
		multithread::fileQueueMTX.lock(); //make this exclusive lock shared writer 
		boost::unique_lock< boost::shared_mutex > lock(multithread:: _access);
		if (multithread::file_queue2.size() != 0) { //check if the queue have filename remaining
			filename = multithread::file_queue2.front();
			//OK till here as we get the filename from the required queue now we want to read the file again
			ifstream FILE;
			//Store the postion in a int/vector for a particular Reader thread
			//Pass the int/vector to cache to check whether already available
			OpenFile(FILE, Memcache::dataPath, filename, true);
			/*vector<int> newposition =*/ 
			unordered_map<string, string>map=GetPosition(FILE); //return key value pair
			FILE.close();
			//lru cache
			lru_cache<string, string> *lru = new lru_cache<string, string>(3);
			//lru check for key value to save a bad write
			//iterate through map
			unordered_map<string, string>::iterator itr;
			fstream file;
			file.open(Memcache::dataPath + "itemfile.txt");
			if(file.is_open()){
			for (itr = map.begin(); itr != map.end(); itr++)
			{
				string sLine;
				int line_no = 0;
				while (line_no != stoi(itr->first)/*newpos.key/*newposition.at(i)*/ && getline(file, sLine)) { //getline() check??
					++line_no;
				}
				if (line_no == stoi(itr->first)/*key*/) {
					// sLine contains the x line in the file.
				//	getline(FILE, sLine);
					//ProduceWriterOut(filename, dataPath, itr->second);
					file.seekp(file.tellp());
					file << itr->second << endl;
				}
				else {
					// The file contains fewer than x lines.

				}
				//lru->Put(/*to_string(newposition.at(i)*/), sLine);
			}
		}
		
		//check condition from cache with flag then read from item file it not 
		//and write to Reader*.out file
		multithread::consoleOutMTX.lock();
		cout << "reader #" << id + 1 << " reading " << filename << endl;
		multithread::consoleOutMTX.unlock();
		multithread::file_queue2.pop_front();
		}
		else {//if not, unlock the mutex and return the function
			boost::unique_lock< boost::shared_mutex > unlock(multithread::_access);
			//fileQueueMTX.unlock();
			multithread::currentreaderWorking--;
			return;
		}
		//multithread::fileQueueMTX.unlock();
		boost::unique_lock< boost::shared_mutex > unlock(multithread::_access);
		
	}
}

	//void multithread::writer::ProduceWriterOut(string &filename, string &dataPath, string &value)
	//{
	//	ofstream file;
	//	file.open(dataPath + filename);
	//	int line_no = 0;
	//	//while (line_no != newpos.key/*newposition.at(i)*/ && getline(FILE, sLine)) {
	//	//	++line_no;
	//	//}
	//	//if (line_no == key) {
	//	//	// sLine contains the x line in the file.
	//	//	//	getline(FILE, sLine);
	//	//	file << value;
	//	//}
	//	
	//}

void /*multithread::*/reader::ProduceReaderOut(string& filename, string &value, bool check)
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

vector<int>/* multithread::*/reader::GetPosition(ifstream &obj)
{
	string tmp;
	vector<int> position;
	while (getline(obj, tmp)) {
		position.push_back(stoi(tmp)); //push file name into the queue
	}
	return position;
}

void /*multithread::*/reader::start() {
	//multithread::currentreaderWorking++;
	while (true) {
		//multithread::fileQueueMTX.lock(); //make this lock shared reader 
		boost::shared_lock< boost::shared_mutex > lock(multithread::_access);
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
			cout << "reader #" << id + 1 << " reading " << filename << endl;
			multithread::consoleOutMTX.unlock();
			multithread::file_queue.pop_front();
		}
		else {//if not, unlock the mutex and return the function
			//fileQueueMTX.unlock();
			boost::shared_lock< boost::shared_mutex > unlock(multithread::_access);
			multithread::currentreaderWorking--;
			return;
		}
		//multithread::fileQueueMTX.unlock();
		boost::shared_lock< boost::shared_mutex > unlock(multithread::_access);
		/* process counting occurence of keyword */
		OpenFile(file, dataPath, filename, false);
		file.close();
		multithread::consoleOutMTX.lock();
		//	cout << "reader #" << id + 1 << " pushing " << count << " into count_queue" << endl;
		multithread::consoleOutMTX.unlock();

	}
}
