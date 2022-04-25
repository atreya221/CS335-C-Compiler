void printi(int c);
void printc(char c);
void prints(char *str);
int scani();
 
void DFS(int ver, int G[10][10], int n, int* visited)
{
    int j;
	printi(ver);
    visited[ver]=1;
	
	for(j=0;j<n;j++)
       if(visited[j]==0&&G[ver][j]==1)
            DFS(j, G, n, visited);
    return;
}

void main()
{
    int i,j;
    int G[10][10],visited[10],n; 
    prints("Enter number of vertices:");
   
	n=scani();
 
	prints("\nEnter adjecency matrix of the graph:");
   
	for(i=0;i<n;i++)
       for(j=0;j<n;j++)
		G[i][j]=scani();
 
   for(i=0;i<n;i++)
        visited[i]=0;
    prints("\n DFS Traversal is\n");
    DFS(0, G, n, visited);
}
 
