#define MaxClustersGenres 5
#define	SizeOfUserVector 45
#define SizeOfMovieVector 22

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
	
	if((0 <= Vote_count)&&(Vote_count <= 25))symbol = 1;
	else if((26 <= Vote_count)&&(Vote_count <= 50))symbol = 2; 
	else if((51 <= Vote_count)&&(Vote_count <= 75))symbol = 3; 
	else if((76 <= Vote_count)&&(Vote_count <= 100))symbol = 4; 
	else if((101 <= Vote_count)&&(Vote_count <= 125))symbol = 5; 
	else if((126 <= Vote_count)&&(Vote_count <= 150))symbol = 6; 
	else if((151 <= Vote_count)&&(Vote_count <= 200))symbol = 7; 
	else if((201 <= Vote_count)&&(Vote_count <= 250))symbol = 8; 
	else if((251 <= Vote_count)&&(Vote_count <= 300))symbol = 9; 
	else if(301 <= Vote_count)symbol = 10; 
	
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
		else if((100 <= FrequencyOfGenres[k])&&(FrequencyOfGenres[k] < 400)) Cluster[i] = 3;
		else if(FrequencyOfGenres[k] < 100) Cluster[i] = 4;
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


int FrequencyOf_Genres(int FrequencyOfGenres[MaxGenres], int TheGenres[MaxGenres]){
	int i, Nb_Genres;
	int Frequency[10] = {723,982,425,635,596,287,317,64,49,83};
	
	Nb_Genres = 10;
	
	for(i=0;(i < Nb_Genres);i++){
		FrequencyOfGenres[i] = Frequency[i];
		TheGenres[i] = i+1;
	}
	
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
			Lambda[IndexFeature].M = 15;  /* 2^(NbClusters) - 1 */
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

void DistancesVectors(double ** Vectors, int NbVectors, double *** Distances, int SizeOfVector, FILE ** f){
	int j,l;
	
    for (j = 0; j < NbVectors; j++) {
		Distances[0][j][j] = 0.0;
		Distances[1][j][j] = 0.0;
		for (l = j+1; l < NbVectors; l++) {
			Distances[0][j][l] = EuclideanDistance(Vectors[j], Vectors[l], SizeOfVector);
			Distances[1][j][l] = ManhattanDistance(Vectors[j], Vectors[l], SizeOfVector);
			Distances[0][l][j] = Distances[0][j][l];
			Distances[1][l][j] = Distances[1][j][l];
		}
	}

	fprintf((*f),"\n==============  Euclidean distances ===========\n\n");
    for (j = 0; j < NbVectors; j++) {
		for (l = j; l < NbVectors; l++) {
			fprintf((*f),"D(%i,%i) = %.3f\n",j+1,l+1,Distances[0][j][l]);
		}
	}
	
	fprintf((*f),"\n==============  Manhattan distances ===========\n\n");
    for (j = 0; j < NbVectors; j++) {
		for (l = j; l < NbVectors; l++) {
			fprintf((*f),"D(%i,%i) = %.3f\n",j+1,l+1,Distances[1][j][l]);
		}
	}

	return;
}
