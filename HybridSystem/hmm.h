#define MaxUsers 1000                    /* Max number of users */ 
#define MaxMovies 1700                   /* Max number of movies */ 

#define MaxWatchedMovies 500	         /* Max number of movies watched by a user */
#define MaxUsersWatching 500	         /* Max number of users who watched a movie */
#define MaxNeighbours 50                 /* Max number of neighbours for the KNN algorithm */

#define MaxStates 5 			         /* Max number of states */
#define MaxSymbols 15           		 /* Max number of symbols */
#define MaxFeatures 6
#define MinDouble 1e-150                  
#define MinFloat 1e-7                   
#define Epsi 1.0                          


// A HMM
typedef struct HMM HMM;
struct HMM
{
	int N;
	int M;
	double A[MaxStates][MaxStates];
	double B[MaxStates][MaxSymbols];
	double Pi[MaxStates];
	double Phi[MaxStates];				
};

// Adjustment of a HMM
void AdjustHMM(HMM * Lambda){
	int i,j,k;
	double Som;
	
		/* Reajustement Pi */
		
		Som = 0.0;
		for(i=0;(i < (*Lambda).N);i++){
			Som += (*Lambda).Pi[i];
		}	
		Som = fabs(1.0 - Som)/(*Lambda).N;
		for(i=0;(i < (*Lambda).N);i++){
			(*Lambda).Pi[i] += Som;
		}	
		/* Reajustement A */	
		for(i=0;(i < (*Lambda).N);i++){
			Som = 0.0;
			for(j=0;(j < (*Lambda).N);j++){
				Som += (*Lambda).A[i][j];
			}
			Som = fabs(1.0 - Som)/(*Lambda).N;
			for(j=0;(j < (*Lambda).N);j++){
				(*Lambda).A[i][j] += Som;
			}	
		}	
		/* Reajustement b */	
		for(j=0;(j < (*Lambda).N);j++){
			Som = 0.0;
			for(k=0;(k < (*Lambda).M);k++){
				Som += (*Lambda).B[j][k];
			}
			Som = fabs(1.0 - Som)/(*Lambda).M;
			for(k=0;(k < (*Lambda).M);k++){
				(*Lambda).B[j][k] += Som;
			}	
		}

	return;
}

// Replaces low probability values by zeros in the HMM 
void CleanHMM(HMM * Lambda){
	int i,j;

	for(i=0;(i < (*Lambda).N);i++){
		for(j=0;(j < (*Lambda).N);j++){
			if((*Lambda).A[i][j] < MinFloat) (*Lambda).A[i][j] = 0.0;
		}
	}
	for(i=0;(i < (*Lambda).N);i++){
		for(j=0;(j < (*Lambda).M);j++){
			if((*Lambda).B[i][j] < MinFloat) (*Lambda).B[i][j] = 0.0;
		}
	}
	for(i=0;(i < (*Lambda).N);i++){
		if((*Lambda).Pi[i] < MinFloat) (*Lambda).Pi[i] = 0.0;
	}
	for(i=0;(i < (*Lambda).N);i++){
		if((*Lambda).Phi[i] < MinFloat) (*Lambda).Phi[i] = 0.0;
	}
	return;
}


// Copy of a HMM
void CopyHMM(HMM * Lambda1, HMM Lambda2){
	int i,j;
	
	(*Lambda1).N = Lambda2.N;
	(*Lambda1).M = Lambda2.M;
	for(i = 0; i < Lambda2.N; i++)
	{
		(*Lambda1).Pi[i] = Lambda2.Pi[i];	
		(*Lambda1).Phi[i] = Lambda2.Phi[i];	
		for(j = 0; j < Lambda2.N; j++)
		{
			(*Lambda1).A[i][j] = Lambda2.A[i][j];
		}
		for(j = 0; j < Lambda2.M; j++)
		{
			(*Lambda1).B[i][j] = Lambda2.B[i][j];
		}
	}
	return;
}

// Saves a HMM in a textfile
void SaveHMMtxt(HMM Lambda, char * OutputFile){
	int i,j;
	FILE * f;
	
	f = fopen(OutputFile, "wt");
	fprintf(f, "\nParameters: %i states AND %i symbols\n\n", Lambda.N, Lambda.M);
	fprintf(f, "Matrix A :\n\n");

	for(i = 0; i < Lambda.N; i++)
	{
		for(j = 0; j < Lambda.N; j++)
			fprintf(f, "%g\t", Lambda.A[i][j]);

		fprintf(f, "\n");
 	}

	fprintf(f, "\n\nMatrix B :\n\n");

	for(i = 0; i < Lambda.N; i++)
	{
		for(j = 0; j < Lambda.M; j++)
			fprintf(f, "%g\t", Lambda.B[i][j]);

		fprintf(f, "\n");
 	}

	fprintf(f, "\n\nVector Pi :\n\n");

	for(j = 0; j < Lambda.N; j++)
		fprintf(f, "%g\t", Lambda.Pi[j]);

	fprintf(f, "\n");

	fprintf(f, "\n\nVector Phi :\n\n");

	for(j = 0; j < Lambda.N; j++)
		fprintf(f, "%g\t", Lambda.Phi[j]);

	fprintf(f, "\n");
	fclose(f);
	return;
}


// Copy of a matrix
void Copy_Matrix(double M1[MaxStates][MaxStates], double M2[MaxStates][MaxStates], int NbRow, int NbCol){
	int i, j;
	for(i = 0; i < NbRow; i++)
	{
		for(j = 0; j < NbCol; j++)
		{
			M1[i][j] = M2[i][j];
		}
	}
	return;
}


// Computes the stationnary distribution of a HMM
int StationaryDistribution(HMM * Lambda){
	int i,j,n, k,nb;
	double A_tem[MaxStates][MaxStates], A_Product[MaxStates][MaxStates];
	
	n = (*Lambda).N;
	Copy_Matrix(A_tem, (*Lambda).A,n,n);
	nb = 0;
	do
	{
		for(i = 0; i < n; i++)
		{
			for(j = 0; j < n; j++)
			{
				A_Product[i][j] = 0.0;
				for(k = 0; k < n; k++)
				{
					A_Product[i][j] += A_tem[i][k] * (*Lambda).A[k][j];
				}
			}
		}
		Copy_Matrix(A_tem, A_Product, n, n);
		nb++;
	}while(nb <= 100);

	for(i = 0; i < n; i++)
	{
		(*Lambda).Phi[i] = A_Product[0][i];
	}
	
	return j;
}


// Euclidean distance
double EuclideanDistance(double X[MaxFeatures*MaxSymbols], double Y[MaxFeatures*MaxSymbols], int N){
	double x,y,D;
	int i;
	
	D = 0.0;
	for(i=0;(i < N);i++){
		x = X[i];
		y = Y[i];
		D += (x-y)*(x-y);
	}
	D = sqrt(D);
	return D;
}


// Manhattan distance
double ManhattanDistance(double X[MaxFeatures*MaxSymbols], double Y[MaxFeatures*MaxSymbols], int N){
	double x,y,D;
	int i;
	
	D = 0.0;
	for(i=0;(i < N);i++){
		x = X[i];
		y = Y[i];
		D += fabs(x-y);
	}
	return D;
}


// Computes the k-th component of the vector associated with a HMM
double ProbaSymbolLongTime(int k, HMM Lambda){
	double E;
	int i;
	
	E = 0.0;
	for(i=0;(i < Lambda.N);i++){
		E += Lambda.Phi[i]*Lambda.B[i][k];
	}
	return E;
}

// Computes the vector associated with a HMM
void VectorHMM(HMM Lambda, double V[MaxSymbols]){
	int k;
	
	for(k=0;(k < Lambda.M);k++){
		V[k] = ProbaSymbolLongTime(k,Lambda);
		
		if(V[k] < MinFloat){
			V[k] = 0.0;
		}
	}
	return;
}

// Swaps two doubles
void swapDouble(double* a, double* b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

// Swaps two floats
void swapFloat(float* a, float* b) {
    float temp = *a;
    *a = *b;
    *b = temp;
}

// Swaps two integers
void swapInt(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


