void printi( int x);
void printc( char x);
int scani();
void exit();

int partition( int * data, int low, int high ) {
	int i,j,p,t;
	p = data[high];
	i = low - 1;
	for ( j = low; j <= high - 1; j++ ) {
		if ( data[j] < p ) {
			i++;
			t = data[i];
			data[i] = data[j];
			data[j] = t;
			
		}
		
	}
	t = data[i+1];
	data[i+1] = data[high];
	data[high] = t;
	return i + 1;
}





void quick_sort (int * data, int low, int high) {
        int i, j, p, t;
	if ( low >= high ) 
		return;
	p = partition( data, low, high);

	quick_sort( data, low, p-1);
	quick_sort( data, p+1, high);

}
int main() {
	int i;
	int j;
	int k;
	int a[50];
	j = 31;
	for ( i = 0; i < 50; i++) 
	{ 
		a[i] = j*35 - i;
		printi(a[i]);
		printc(' ');
	}
	printc('\n');

	quick_sort(a,0,49);
	for ( i = 0; i < 50; i++) 
	{ 
		printi(a[i]);
		printc(' ');
	}
	printc('\n');
	return 0;
}
