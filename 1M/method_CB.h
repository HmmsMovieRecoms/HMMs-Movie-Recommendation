#define MaxClustersGenres 6                   /* Max number of clusters for the movie genres */    
#define MaxNeighbours 50                      /* Max number of neighbours for the KNN algorithm */
#define	SizeOfUserVector 61
#define SizeOfMovieVector 20

// Sampling of the Score to obtain the corresponding state
int StateOf_CB(float Score){
	int state;
	
	if((0.0 <= Score)&&(Score <= 1.0))state = 1;
	else if((1.0 < Score)&&(Score <= 2.0))state = 2; 
	else if((2.0 < Score)&&(Score <= 3.0))state = 3; 
	else if((3.0 < Score)&&(Score <= 4.0))state = 4; 
	else if(4.0 < Score)state = 5; 
	
	return state;
}

// Sampling of the Release_date to obtain the corresponding symbol
int SymbolOf_Release_date(int Release_date){
	int symbol;
	
	if(Release_date <= 1979)symbol = 1;
	else if((1980 <= Release_date)&&(Release_date <= 1985))symbol = 2; 
	else if((1986 <= Release_date)&&(Release_date <= 1990))symbol = 3; 
	else if((1991 <= Release_date)&&(Release_date <= 1995))symbol = 4; 
	else if((1996 <= Release_date)&&(Release_date <= 2000))symbol = 5; 
	else if((2001 <= Release_date)&&(Release_date <= 2005))symbol = 6; 
	else if((2006 <= Release_date)&&(Release_date <= 2010))symbol = 7; 
	else if((2011 <= Release_date)&&(Release_date <= 2015))symbol = 8; 
	else if((2016 <= Release_date)&&(Release_date <= 2020))symbol = 9; 
	else if(2021 <= Release_date)symbol = 10; 
	
	return symbol;
}


// Sampling of the Vote_average to obtain the corresponding symbol
int SymbolOf_Vote_average(float Vote_average){
	int symbol;
	
	if((0.0 <= Vote_average)&&(Vote_average <= 0.5))symbol = 1;
	else if((0.5 < Vote_average)&&(Vote_average <= 1.0))symbol = 2; 
	else if((1.0 < Vote_average)&&(Vote_average <= 1.5))symbol = 3; 
	else if((1.5 < Vote_average)&&(Vote_average <= 2.0))symbol = 4; 
	else if((2.0 < Vote_average)&&(Vote_average <= 2.5))symbol = 5; 
	else if((2.5 < Vote_average)&&(Vote_average <= 3.0))symbol = 6; 
	else if((3.0 < Vote_average)&&(Vote_average <= 3.5))symbol = 7; 
	else if((3.5 < Vote_average)&&(Vote_average <= 4.0))symbol = 8; 
	else if((4.0 < Vote_average)&&(Vote_average <= 4.5))symbol = 9; 
	else if(4.5 < Vote_average)symbol = 10; 
	
	return symbol;
}


// Sampling of the Vote_count to obtain the corresponding symbol
int SymbolOf_Vote_count(int Vote_count){
	int symbol;
	
	if((0 <= Vote_count)&&(Vote_count <= 10))symbol = 1;
	else if((11 <= Vote_count)&&(Vote_count <= 50))symbol = 2; 
	else if((51 <= Vote_count)&&(Vote_count <= 100))symbol = 3; 
	else if((101 <= Vote_count)&&(Vote_count <= 250))symbol = 4; 
	else if((251 <= Vote_count)&&(Vote_count <= 500))symbol = 5; 
	else if((501 <= Vote_count)&&(Vote_count <= 1000))symbol = 6; 
	else if((1001 <= Vote_count)&&(Vote_count <= 1500))symbol = 7; 
	else if((1501 <= Vote_count)&&(Vote_count <= 2000))symbol = 8; 
	else if((2001 <= Vote_count)&&(Vote_count <= 2500))symbol = 9; 
	else if(2501 <= Vote_count)symbol = 10; 
	
	return symbol;
}

// Sampling and clustering of the Genres to obtain the corresponding symbol
int SymbolOf_Genres(int Genres[MaxGenres], int NbGenres, int FrequencyOfGenres[MaxGenres], int TheGenres[MaxGenres], int NbClusters){
	int Binary[MaxClustersGenres], i,j,k,symbol,Cluster[MaxGenres];
	
	for(i=0;(i < NbGenres);i++){
		
		k = 0;
		while(Genres[i] != TheGenres[k]){
			k++;
		}

		if(700 <= FrequencyOfGenres[k]) Cluster[i] = 1;
		else if((400 <= FrequencyOfGenres[k])&&(FrequencyOfGenres[k] < 700)) Cluster[i] = 2;
		else if((200 <= FrequencyOfGenres[k])&&(FrequencyOfGenres[k] < 400)) Cluster[i] = 3;
		else if((100 <= FrequencyOfGenres[k])&&(FrequencyOfGenres[k] < 200)) Cluster[i] = 4;
		else if(FrequencyOfGenres[k] < 100) Cluster[i] = 5;
	}

	for(j=0;(j < NbClusters);j++){
		Binary[j] = 0;
	}
	
	for(i=0;(i < NbGenres);i++){
		Binary[Cluster[i]-1] = 1;
	}

	symbol = Binary[0];
	for(j=1;(j < NbClusters);j++){
		symbol = 2*symbol + Binary[j];
	}
	
	return symbol;
}


int FrequencyOf_Genres(Movie * TheMovies, int NbMovies, int FrequencyOfGenres[MaxGenres], int TheGenres[MaxGenres]){
	int i, j, k, Nb_Genres;
	
	Nb_Genres = 0;
	for(i=0;(i < NbMovies);i++){
		for(k=0;(k < TheMovies[i].NbGenres);k++){
			j=0;
			while((j < Nb_Genres)&&(TheMovies[i].Genres[k] != TheGenres[j])){
				j++;
			}
			if(j < Nb_Genres){
				FrequencyOfGenres[j] += 1;
			}else{
				TheGenres[j] = TheMovies[i].Genres[k];
				FrequencyOfGenres[j] = 1;
				Nb_Genres++;
			}
		}
	}
	
	/**
	for(i=0;(i < Nb_Genres);i++){
		printf("%i|%i\n",TheGenres[i],FrequencyOfGenres[i]);
	}
	**/
	
	return Nb_Genres;
}

// Index of a movie in the array 'TheMovies' according to its 'Movie_Id'
int IndexOfMovie(int Movie_Id, Movie * TheMovies, int NbMovies){
	int k;
	
	for (k = 0; k < NbMovies; k++) {
		if(TheMovies[k].Movie_Id == Movie_Id){
			return k;
		}
	}
	return -1;
}

// Constructs the Markov chain associated with each user u
void ConstructMCsOfUser(User u, MC * TheMCs, int ** Sequences, int NbFeatures, Movie * TheMovies, int NbMovies, int FrequencyOfGenres[MaxGenres], int TheGenres[MaxGenres], int NbClustersGenres, FILE ** f){
	int i, j, MovieIndex, state;
	
	for(j=0;(j < NbFeatures);j++){
		TheMCs[j].NbCouples = u.NbWatchedMovies;
	}

	for(i=0;(i < u.NbWatchedMovies);i++){
		/* States */
		state = StateOf_CB(u.LastWatchedMovies[i].Score);
		for(j=0;(j < NbFeatures);j++){
			TheMCs[j].Elements[i].State = state;
		}
		
		/* Symbols */
		MovieIndex = IndexOfMovie(u.LastWatchedMovies[i].Movie_Id,TheMovies,NbMovies);
		if(MovieIndex != -1){
			TheMCs[0].Elements[i].Symbol = SymbolOf_Release_date(TheMovies[MovieIndex].Release_date);
			TheMCs[1].Elements[i].Symbol = SymbolOf_Vote_average(TheMovies[MovieIndex].Vote_average);
			TheMCs[2].Elements[i].Symbol = SymbolOf_Vote_count(TheMovies[MovieIndex].Vote_count);
			TheMCs[3].Elements[i].Symbol = SymbolOf_Genres(TheMovies[MovieIndex].Genres,TheMovies[MovieIndex].NbGenres,FrequencyOfGenres,TheGenres,NbClustersGenres);
			
			for(j=0;(j < NbFeatures);j++){
				Sequences[j][i] = TheMCs[j].Elements[i].Symbol - 1;
			}
		}
	}

	fprintf(*f,"_______________________________________________________________\n");
	for(j=0;(j < NbFeatures);j++){
		fprintf(*f,"delta(u_%i,%i):\t",u.User_Id,j+1);
		DisplayMCfile(TheMCs[j],f);
	}
	
	return;
}


// Constructs all the initial HMMs associated with each user
int InitialHMMsOfUser(MC * TheMCsOfUser, int NbFeatures, HMM Lambda[MaxFeatures]){
	int NbSymbols, i,j,k,IndexFeature,State,NextState,Symbol,From[MaxStates],UseOfState[MaxStates];
	double Sum;
	
	NbSymbols = 0;
	
	for(IndexFeature=0;(IndexFeature < NbFeatures);IndexFeature++){
		
		Lambda[IndexFeature].N = 5; 
		
		if(IndexFeature == 3){
			/* Genres */
			Lambda[IndexFeature].M = 31;  /* 2^(NbClusters) - 1 */
		}else{
			Lambda[IndexFeature].M = 10;
		}
		NbSymbols += Lambda[IndexFeature].M;
	}
	
	for(IndexFeature=0;(IndexFeature < NbFeatures);IndexFeature++){
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Lambda[IndexFeature].Pi[i] = 0.0;
			From[i] = 0;
			UseOfState[i] = 0;
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Lambda[IndexFeature].A[i][j] = 0.0;
			}
			for(j=0;(j < Lambda[IndexFeature].M);j++){
				Lambda[IndexFeature].B[i][j] = 0.0;
			}
		}
		
		if(TheMCsOfUser[IndexFeature].NbCouples == 1){
			State = TheMCsOfUser[IndexFeature].Elements[0].State - 1;
			Symbol = TheMCsOfUser[IndexFeature].Elements[0].Symbol - 1;

			UseOfState[State] += 1;
			Lambda[IndexFeature].Pi[State] += 1.0;
			Lambda[IndexFeature].B[State][Symbol] += 1.0;
		}else if(TheMCsOfUser[IndexFeature].NbCouples >= 2){
			for(j=0;(j < TheMCsOfUser[IndexFeature].NbCouples);j++){
				if(j != TheMCsOfUser[IndexFeature].NbCouples -1){
					State = TheMCsOfUser[IndexFeature].Elements[j].State - 1;
					NextState = TheMCsOfUser[IndexFeature].Elements[j+1].State - 1;
					Symbol = TheMCsOfUser[IndexFeature].Elements[j].Symbol - 1;

					From[State] += 1;
					UseOfState[State] += 1;
					Lambda[IndexFeature].A[State][NextState] += 1.0;
					Lambda[IndexFeature].B[State][Symbol] += 1.0;
				
					if(j == 0){
						Lambda[IndexFeature].Pi[State] += 1.0;
					}
				}else{
					State = TheMCsOfUser[IndexFeature].Elements[j].State - 1;
					Symbol = TheMCsOfUser[IndexFeature].Elements[j].Symbol - 1;

					UseOfState[State] += 1;
					Lambda[IndexFeature].B[State][Symbol] += 1.0;
				}
			}	
		}
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Lambda[IndexFeature].Pi[i] /= (1+Epsi);
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Lambda[IndexFeature].A[i][j] /= (From[i]+Epsi);
			}
		}
		for(j=0;(j < Lambda[IndexFeature].N);j++){
			for(k=0;(k < Lambda[IndexFeature].M);k++){
				Lambda[IndexFeature].B[j][k] /= (UseOfState[j]+Epsi);
			}
		}

		/* Readjustment of Pi */
		Sum = 0.0;
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Sum += Lambda[IndexFeature].Pi[i];
		}	
		Sum = fabs(1.0 - Sum)/Lambda[IndexFeature].N;
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Lambda[IndexFeature].Pi[i] += Sum;
		}	
		
		/* Readjustment of A */
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Sum = 0.0;
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Sum += Lambda[IndexFeature].A[i][j];
			}
			Sum = fabs(1.0 - Sum)/Lambda[IndexFeature].N;
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Lambda[IndexFeature].A[i][j] += Sum;
			}	
		}	
		
		/* Readjustment of B */
		for(j=0;(j < Lambda[IndexFeature].N);j++){
			Sum = 0.0;
			for(k=0;(k < Lambda[IndexFeature].M);k++){
				Sum += Lambda[IndexFeature].B[j][k];
			}
			Sum = fabs(1.0 - Sum)/Lambda[IndexFeature].M;
			for(k=0;(k < Lambda[IndexFeature].M);k++){
				Lambda[IndexFeature].B[j][k] += Sum;
			}	
		}
		
		/* Not necessary at this level */
		StationaryDistribution(&Lambda[IndexFeature]);
	}
	
	return NbSymbols;
}	

// Manhattan distance
double ManhattanDistance(double * X, double * Y, int N){
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

void DistancesMovieVectors(double ** MovieVectors, int NbVectors, double ** Distances, double * Distance_Max){
	int j,l;
	
	(*Distance_Max) = 0.0;
    for (j = 0; j < NbVectors; j++) {
		Distances[j][j] = 0.0;
		for (l = j+1; l < NbVectors; l++) {
			Distances[j][l] = ManhattanDistance(MovieVectors[j], MovieVectors[l], SizeOfMovieVector);
			Distances[l][j] = Distances[j][l];
			if(Distances[j][l] > (*Distance_Max)){
				(*Distance_Max) = Distances[j][l];
			}
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

// Function of partitionnement for the QuickSortKNN
int PartitionKNN_CB(double * distances, int * indices, int low, int high) {
    double pivot = distances[high]; // pivot
    int i = (low - 1); // Index of the smallest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is lower than pivot
        if (distances[j] < pivot) {
            i++; // Increments the index of the smallest element
            swapDouble(&distances[i], &distances[j]);
            swapInt(&indices[i], &indices[j]);
        }
    }
    swapDouble(&distances[i + 1], &distances[high]);
    swapInt(&indices[i + 1], &indices[high]);
    return (i + 1);
}

// QuickSort KNN
void QuicksortKNN(double * distances, int * indices, int low, int high) {
    if (low < high) {
        int pi = PartitionKNN_CB(distances, indices, low, high);
        QuicksortKNN(distances, indices, low, pi - 1);
        QuicksortKNN(distances, indices, pi + 1, high);
    }
}

// KNN algorithm 
void KNN(int NbNeighbours, double ** UserVectors, int NbVectors, int ** TheNeighbours) {
    int i,j,m, * indices;
	double * distances;

	indices = (int *)malloc(MaxUsers*sizeof(int));
	distances = (double *)malloc(MaxUsers*sizeof(double));

    for (i = 0; i < NbVectors; i++) {

        // collects the distance between the i-th vector and all the system vectors
        for (j = 0; j < NbVectors; j++) {
            if (i != j) {
				distances[j] = ManhattanDistance(UserVectors[i], UserVectors[j], SizeOfUserVector);
            } else {
				// An infinite distance between the i-th vector and itself
                distances[j] = DBL_MAX; // 
            }
            indices[j] = j;
        }

        // QuickSort of the distances and the indices
		QuicksortKNN(distances,indices,0,NbVectors-1);
        // Records the K nearest neighbors and the corresponding distances
        for (m = 0; m < NbNeighbours; m++) {
            TheNeighbours[i][m] = indices[m];
        }
    }
	free(indices);
	free(distances);
	return;
}

float PredictedScore(int Movie_Id, int User_Index, User * TheUsers, double ** Distances, double Distance_Max){
	double distMin, dist, epsilon = 1e-10;
	int i,j,k,x;
	float Score;
	
	if(TheUsers[User_Index].NbWatchedMovies == 0){
		return 0.0;
	}
	
	j = TheUsers[User_Index].LastWatchedMovies[0].Movie_Id-1;
	distMin = Distances[Movie_Id-1][j];
	
	for(i=1;(i < TheUsers[User_Index].NbWatchedMovies);i++){
		j = TheUsers[User_Index].LastWatchedMovies[i].Movie_Id-1;
		dist = Distances[Movie_Id-1][j];
		if(dist < distMin){
			distMin = dist;
		}
	}
	
	k = 0;
	Score = 0.0;
	for(i=0;(i < TheUsers[User_Index].NbWatchedMovies);i++){
		j = TheUsers[User_Index].LastWatchedMovies[i].Movie_Id-1;
		dist = Distances[Movie_Id-1][j];
		if( fabs(dist - distMin) <= epsilon){
			Score += TheUsers[User_Index].LastWatchedMovies[i].Score;
			k++;
		}
	}
	Score = (Score/k);
	Score = Score*(1.0 - (distMin/Distance_Max));
	return Score;
}

// Index of a recommended movie in the array 'TheMovieRecom' according to its 'Movie_Id'
int IndexOfMovieRecom(int Movie_Id, MovieRecom * TheMovieRecom, int NbMovieRecom){
	int k;
	
	for (k = 0; k < NbMovieRecom; k++) {
		if(TheMovieRecom[k].Movie_Id == Movie_Id){
			return k;
		}
	}
	return -1;
}

// Computes the initial set of recommended movies (Equation 8 in the paper)
// and their corresponding Importance  (Equation 9 in the paper)
void InitMovieRecom(MovieRecom ** TheMovieRecom, int * NbMovieRecom, int ** TheNeighbours,  int NbNeighbours, User * TheUsers, int NbUsers, double ** Distances, double Distance_Max){
	int i,j,k,l,m,n;
	float p;
	
	for (i = 0; i < NbUsers; i++) {
		NbMovieRecom[i] = 0;
	}	

	for (i = 0; i < NbUsers; i++) {
		for(k=0;(k < NbNeighbours);k++){
			/* l is the index of the current (k-th) neighbour of the i-th user */
			l = TheNeighbours[i][k];
			
			/* Browsing the movies watched by the current neighbour i-th user */
			for(j=0;(j < TheUsers[l].NbWatchedMovies);j++){
				m = TheUsers[l].LastWatchedMovies[j].Movie_Id;
				n = IndexOfMovieRecom(m,TheMovieRecom[i],NbMovieRecom[i]);
				if(n == -1){
					/* Inserting a new recommended movie */
					TheMovieRecom[i][NbMovieRecom[i]].Movie_Id = m;
					TheMovieRecom[i][NbMovieRecom[i]].Frequency = 1;
					TheMovieRecom[i][NbMovieRecom[i]].Importance = 0.0;
					p = PredictedScore(m,i,TheUsers,Distances,Distance_Max);
					TheMovieRecom[i][NbMovieRecom[i]].PredictedScore = p;
					NbMovieRecom[i]++;
				}else{
					/* Updating the frequency of an existing recommended movie */
					TheMovieRecom[i][n].Frequency += 1;
				}
			}
		}
		/* Updating the importance of all the movies in the set of recommendations */
		if(TheUsers[i].NbWatchedMovies > 0){
			for(j=0;(j < NbMovieRecom[i]);j++){
				l = TheMovieRecom[i][j].Movie_Id;
				for(k=0;(k < TheUsers[i].NbWatchedMovies);k++){
					m = TheUsers[i].LastWatchedMovies[k].Movie_Id;
					TheMovieRecom[i][j].Importance += TheUsers[i].LastWatchedMovies[k].Score/(Distances[l][m] + MinDouble);
				}
				TheMovieRecom[i][j].Importance *= (TheMovieRecom[i][j].Frequency);
				TheMovieRecom[i][j].Importance = log(TheMovieRecom[i][j].Importance);
			}
		}
	}
	
	return;
}


// Function of partitionnement for the QuickSortMovies
int PartitionMovies_CB(MovieRecom * TheMovieRecom, int low, int high) {
    float pivot = TheMovieRecom[high].Importance; // pivot
    int i = (low - 1); // Index of the highest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is greater than pivot
        if (TheMovieRecom[j].Importance > pivot) {
            i++; // Increments the index of the highest element 
			
            swapInt(&TheMovieRecom[i].Movie_Id, &TheMovieRecom[j].Movie_Id);
            swapInt(&TheMovieRecom[i].Frequency, &TheMovieRecom[j].Frequency);
            swapDouble(&TheMovieRecom[i].Importance, &TheMovieRecom[j].Importance);
            swapFloat(&TheMovieRecom[i].PredictedScore, &TheMovieRecom[j].PredictedScore);
        }
    }
    swapInt(&TheMovieRecom[i+1].Movie_Id, &TheMovieRecom[high].Movie_Id);
    swapInt(&TheMovieRecom[i+1].Frequency, &TheMovieRecom[high].Frequency);
    swapDouble(&TheMovieRecom[i+1].Importance, &TheMovieRecom[high].Importance);
    swapFloat(&TheMovieRecom[i+1].PredictedScore, &TheMovieRecom[high].PredictedScore);
    return (i + 1);
}


// QuickSort Movies
void QuicksortMovies_CB(MovieRecom * TheMovieRecom, int low, int high) {
    if (low < high) {
        int pi = PartitionMovies_CB(TheMovieRecom, low, high);
        QuicksortMovies_CB(TheMovieRecom, low, pi - 1);
        QuicksortMovies_CB(TheMovieRecom, pi + 1, high);
    }
}

// Sort the recommended movies in decrease order of their importance 
void SortMovieRecom_by_Importance_CB(MovieRecom ** TheMovieRecom, int * NbMovieRecom, int NbUsers){
	int i;
	
	for (i = 0; i < NbUsers; i++) {
		QuicksortMovies_CB(TheMovieRecom[i],0,NbMovieRecom[i]-1);
	}
	return;
}


// Function of partitionnement for the QuickSortMovies_Relevance
int Partition_Relevance(float * TheMovieRelevance, int low, int high) {
    float pivot = TheMovieRelevance[high]; // pivot
    int i = (low - 1); // Index of the highest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is greater than pivot
        if (TheMovieRelevance[j] > pivot) {
            i++; // Increments the index of the highest element 
            swapFloat(&TheMovieRelevance[i], &TheMovieRelevance[j]);
        }
    }
    swapFloat(&TheMovieRelevance[i+1], &TheMovieRelevance[high]);
    return (i + 1);
}


// QuickSort recommendations by relevance
void Quicksort_Relevance(float * TheMovieRelevance, int low, int high) {
    if (low < high) {
        int pi = Partition_Relevance(TheMovieRelevance, low, high);
        Quicksort_Relevance(TheMovieRelevance, low, pi - 1);
        Quicksort_Relevance(TheMovieRelevance, pi + 1, high);
    }
}


// Computes the F1-measure, the Precision, the Recall, the RMSE and the MAE of the system
int ComputeMetrics_CB(User * TheUsers, int NbUsers, MovieRecom ** TheMovieRecom, int * NbMovieRecom, int MaxRecoms, double ** Distances, double Distance_Max, double Metrics[6], double std_dev[3], FILE ** f){
	double * Precision_user, * Recall_user, Intersect, diff;
	double ndcg_user, dcg_user, idcg_user; 
	float *y, *z;
	int i,j,k,l,n,BadRecoms,GoodRecoms, * Good;
	float PredictScore;

	y = (float *)malloc(MaxRecoms*sizeof(float));
	z = (float *)malloc(MaxRecoms*sizeof(float));
	Precision_user = (double *)malloc(MaxUsers*sizeof(double));
	Recall_user = (double *)malloc(MaxUsers*sizeof(double));
	Good = (int *)malloc(MaxMovies*sizeof(int));

	
	Metrics[0] = 0.0;  // overall F1
	Metrics[1] = 0.0;  // overall Precision
	Metrics[2] = 0.0;  // overall Recall
	Metrics[3] = 0.0;  // RMSE
	Metrics[4] = 0.0;  // MAE
	Metrics[5] = 0.0;  // NDCG

	BadRecoms = 0;
	n = 0;
	
	for (i = 0; i < NbUsers; i++) {
		Intersect = 0.0;
		GoodRecoms = 0;
		
		for(j=0;(j < MaxRecoms);j++){
			Good[j] = 0;
			y[k] = -1.0;
			z[k] = -1.0;
		}
		
		for(j=0;(j < TheUsers[i].NbWatchedMovies);j++){
			for (k = 0; (k < MaxRecoms); k++) {
				if(TheUsers[i].LastWatchedMovies[j].Movie_Id == TheMovieRecom[i][k].Movie_Id){
					Intersect += 1.0;
					GoodRecoms++;
					Good[k] = 1;
					y[k] = TheUsers[i].LastWatchedMovies[j].Score;
					z[k] = y[k];
					k = MaxRecoms+1;  // to exit the loop
				}
			}
			
			PredictScore = PredictedScore(TheUsers[i].LastWatchedMovies[j].Movie_Id,i,TheUsers,Distances,Distance_Max);
			diff = (double)(TheUsers[i].LastWatchedMovies[j].Score - PredictScore);
			Metrics[3] += diff*diff;
			Metrics[4] += fabs(diff);
			n++;
		}	
		
		l = 0;
		for(j=0;(j < MaxRecoms);j++){
			if(Good[j] == 0){
				if(l == 0){
					fprintf((*f),"%i,[(%i,%.1f)",TheUsers[i].User_Id,TheMovieRecom[i][j].Movie_Id,TheMovieRecom[i][j].PredictedScore);
				}else{
					fprintf((*f),",(%i,%.1f)",TheMovieRecom[i][j].Movie_Id,TheMovieRecom[i][j].PredictedScore);
				}
				y[j] = TheMovieRecom[i][j].PredictedScore;
				z[j] = y[j];
				l++;
			}
		}
		if(l != 0){
			fprintf((*f),"]\n");
		}
		
		Precision_user[i] = (Intersect / MaxRecoms);
		Recall_user[i] = (Intersect / TheUsers[i].NbWatchedMovies);
		
		dcg_user = 0.0;
		for(j=0;(j < MaxRecoms);j++){
			dcg_user += (pow(2.0, y[j]) - 1.0) / log2(j + 2);
		}
		
		Quicksort_Relevance(z,0,MaxRecoms-1);
		idcg_user = 0.0;
		for(j=0;(j < MaxRecoms);j++){
			idcg_user += (pow(2.0, z[j]) - 1.0) / log2(j + 2);
		}
		ndcg_user = (dcg_user / idcg_user);
		
		Metrics[1] += Precision_user[i];
		Metrics[2] += Recall_user[i];
		Metrics[5] += ndcg_user;
		
		BadRecoms += (MaxRecoms - GoodRecoms);
	}	
	
	Metrics[1] = (Metrics[1]/NbUsers); 
	Metrics[2] = (Metrics[2]/NbUsers); 
	Metrics[5] = (Metrics[5]/NbUsers); 

	for (j = 1; j <= 2; j++) {
		std_dev[j] = 0.0;
	}
	Metrics[0] = (2.0*Metrics[1]*Metrics[2])/(Metrics[1] + Metrics[2] + MinDouble);
	Metrics[3] = sqrt(Metrics[3]/n);
	Metrics[4] = Metrics[4]/n;
	
	for (i = 0; i < NbUsers; i++) {
		std_dev[1] += (Precision_user[i] - Metrics[1])*(Precision_user[i] - Metrics[1]);
		std_dev[2] += (Recall_user[i] - Metrics[2])*(Recall_user[i] - Metrics[2]);
	}
	
	
	for (j = 1; j <= 2; j++) {
		std_dev[j] /= NbUsers;
		std_dev[j] = sqrt(std_dev[j]);
	}
	
	free(y);
	free(z);
	free(Precision_user);
	free(Recall_user);
	free(Good);
	return BadRecoms;
}
