#include <cstdio>
#include <set>

using namespace std;

int main(){
	int m[501][501];
	int c[501][501];
	memset(m,0x3f,sizeof(m));
	memset(c,0x3f,sizeof(c));
	int path[501] = {0};
	int dist[501] = {0};
	int cost = 0, dist = 0;
	int N,M,S,D;
	scanf("%d%d%d%d",&N,&M,&S,&D);
	int from,to,d,co;
	while(M--){
		scanf("%d%d%d%d",&from,&to,&d,&co);
		m[from][to] = d;
		m[to][from] = d;
		c[from][to] = co;
		c[to][from] = co;
	}
	
	for(int i = 0;i<N;i++) dist[i] = m[S][i];
	
	vector<int> city(N);
	for(int i = 0;i<S;++i) city.push_back(i);
	for(int i = S + 1;i<N;++i) city.push_back(i);
	
	for(int i = 0;i<N;++i){
		int min, min_d = INT_MAX;
		for(auto i : city){
			if(m[S][i]<min_d){
				min_d = m[S][i];
				min = i;
			}
		}
		city.erase(min);
	}
	
	for(auto i : city){
		if(dist[S][min]+m[min][i]<dist[S][i]){
			dist[S][i] = dist[S][min]+m[min][i];
		}
	}
	return 0;
}
