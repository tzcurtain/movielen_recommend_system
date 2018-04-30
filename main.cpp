#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>
#include <sstream>

using namespace std;

struct ratenode{
	int userid;
	int movieid;
	double rating;
};

vector<ratenode> Rates;

int getID(const string& str){
	int len = (int)str.length();
	int base = 1,res = 0;
	for (int i=len-1;i>=0;--i){
		if (!isdigit(str[i])){
			return -1;
		}
		res += base * (str[i] - '0');
		base *= 10;
	}
	return res;
}

double getRate(const string& str){
	stringstream ss(str);
	double res;
	ss >> res;
	return res;
}

const int MAXUSERSIZE = 700;
const int MAXMOVIESIZE = 10000;
map<int,int> MovieIndexmap;
int totalIndex = 0;

void dealRate(const string& lineStr){
	stringstream ss(lineStr);
	string str;
	int now = 0;
	ratenode newNode{};
	while (getline(ss,str,',')){
		++now;
		switch (now){
			case 1:
				newNode.userid = getID(str); break;
			case 2:
				newNode.movieid = getID(str);
				if (MovieIndexmap.find(newNode.movieid) == MovieIndexmap.end()){
					totalIndex ++;
					MovieIndexmap[newNode.movieid] = totalIndex;
				}
				break;
			case 3:
				newNode.rating = getRate(str); break;
			default:
				break;
		}
	}
	Rates.push_back(newNode);
}

void deal(const string& lineStr,int mode){
	switch (mode) {
		case 0:
			// dealMovie(lineStr); break;
		case 1:
			dealRate(lineStr); break;
		default:
			fprintf(stderr,"something goes wrong!!"); return;
	}
}

void readCSV(const string& fname,int mode){
	ifstream inFile(fname,ios::in);
	string lineStr;
	getline(inFile,lineStr); // delete first row
	while (getline(inFile,lineStr)){
		deal(lineStr,mode);
	}
}

// Adjacent Matrix & List
int linkedMat[MAXUSERSIZE + 1][MAXMOVIESIZE + 1];
vector<int> linkedTab[MAXUSERSIZE + 1];
int UserDegree[MAXUSERSIZE + 1];
int MovieDegree[MAXMOVIESIZE + 1];

vector<pair<int,int> > trainset;
const double per = 3.0;

void buildModel(){ // get the training and testing sets in the mean time
	int nowsize = Rates.size();
	memset(linkedMat,0,sizeof(linkedMat));
	// build model

	vector<int> order;

	for (int i=0;i<nowsize;i++){
		// calc degree of user and movie
		UserDegree[Rates[i].userid] ++;
		MovieDegree[MovieIndexmap[Rates[i].movieid]] ++;
		if (Rates[i].rating > per)
			order.push_back(i);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(order.begin(),order.end(),g); // use random-shuffle

	int size = order.size();
	nowsize = size * 9 / 10;

	for (int i=0;i<nowsize;i++){
		int user = Rates[order[i]].userid;
		int movie = Rates[order[i]].movieid;
		int index = MovieIndexmap[movie];
		linkedMat[user][index] = 1;
		linkedTab[user].push_back(index);
	}
	// get test set
	for (int i=nowsize;i<size;i++){
		int user = Rates[order[i]].userid;
		int movie = Rates[order[i]].movieid;
		int index = MovieIndexmap[movie];
		pair<int,int> temp;
		temp.first = user; temp.second = index;
		trainset.push_back(temp);
	}
}

// Resource Matrix W
double w[MAXMOVIESIZE + 1][MAXMOVIESIZE + 1];

void calcResourceMatrix(){
	memset(w,0,sizeof(w));
	for (int p=1;p<=MAXUSERSIZE;p++){
		int nowsize = linkedTab[p].size();
		int kl = UserDegree[p];
		double invkl = 1.0 / kl;
		for (int q=0;q<nowsize;q++){
			int i = linkedTab[p][q];
			for (int r=q+1;r<nowsize;r++){
				int j = linkedTab[p][r];
				int kj = MovieDegree[j];
				w[i][j] += invkl/kj;
				kj = MovieDegree[i];
				w[j][i] += invkl/kj;
			}
		}
	}
}

struct ranknode{
	double val;
	int id;
	int rank;
} Ranks[MAXUSERSIZE + 1][MAXMOVIESIZE + 1];
bool cmp(ranknode r1,ranknode r2){
	return (r1.val > r2.val);
}
bool cmp2(ranknode r1,ranknode r2){
	return (r1.id < r2.id);
}

// 阈值
#define dc 0.02
set<int> Recommend[MAXUSERSIZE + 1];
double d[MAXUSERSIZE + 1][MAXUSERSIZE + 1];
void initD(){
	for (int i=1;i<=MAXUSERSIZE;i++){
		for (int j=i+1;j<=MAXUSERSIZE;j++){
			double t = 0;
			int nowsize = linkedTab[i].size();
			for (int k=0;k<nowsize;++k){
				int nowk = linkedTab[i][k];
				Recommend[i].insert(nowk);
				if (linkedMat[j][nowk]) t += 1.0;
			}
			d[i][j] = t / UserDegree[i];
			if (UserDegree[i] == 0) d[i][j] = 0;
			d[j][i] = d[i][j];
			if (d[i][j] > dc){
				int nowsize2 = linkedTab[j].size();
				for (int k=0;k<nowsize2;k++){
					Recommend[i].insert(linkedTab[j][k]);
				}
			}
		}
	}
}

map<int,int> f3[MAXUSERSIZE];
void RankMovie(){
	initD();
	bool f[MAXMOVIESIZE + 1];
	int f2[MAXMOVIESIZE + 1];
	for (int i=1;i<=MAXUSERSIZE;i++){
		int totalsize = 0;
		memset(f2,0,sizeof(f2));
		int fsize = Recommend[i].size();
		for (auto it = Recommend[i].begin();it != Recommend[i].end(); it ++){
			int j = *it;
			totalsize ++;
			f[totalsize] = false;
			if (linkedMat[i][j]){
				f[totalsize] = true;
			}
			f2[totalsize] = j;
			f3[i][j] = totalsize;
		}
		for (int k=1;k<=fsize;k++){
			Ranks[i][k].val = 0.0;
			Ranks[i][k].id = k;
		}
		for (int k=1;k<=fsize;k++){
			if (f[k]){
				int nowk = f2[k];
				for (int j=1;j<=fsize;j++){
					int nowj = f2[j];
					Ranks[i][j].val += w[nowj][nowk];
				}
			}
		}
		sort(Ranks[i]+1,Ranks[i]+1+fsize,cmp);
		int nowrank = 0;
		for (int j=1;j<=fsize;j++){
			int nowpos = Ranks[i][j].id;
			if (!f[nowpos]){
				nowrank ++;
				Ranks[i][j].rank = nowrank;
			}
		}
		sort(Ranks[i]+1,Ranks[i]+1+fsize,cmp2);
	}
}

double GetAccuracy(){
	int nowsize = trainset.size();
	double sum = 0;
	for (int i=0;i<nowsize;i++){
		int user = trainset[i].first;
		int Li = totalIndex - UserDegree[user];
		int movi = f3[user][trainset[i].second];
		double Rij = Ranks[user][movi].rank * 1.0;
		Rij /= Li;
		sum += Rij;
	}
	return sum / nowsize;
}

int main(){
	readCSV("ratings.csv",1);

	buildModel();
	calcResourceMatrix();
	RankMovie();

	cout << "Dc is " << dc << " Accuracy is " << GetAccuracy();

	return 0;
}