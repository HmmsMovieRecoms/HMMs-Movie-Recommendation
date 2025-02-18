#include "hmm_CB.h"

#define MaxGenres 25
#define MaxClustersGenres 5


// A movie
typedef struct Movie_CB{      
	int Movie_Id;                    	 
	int Release_date;  
	float Vote_average;
	int Vote_count;
	int Genres[MaxGenres];
	int NbGenres;
	char Title[100];
} Movie_CB;


// A movie watched by a user
typedef struct WatchedMovie_CB{      
	int Movie_Id;                    	 
	float Score;                         
} WatchedMovie_CB;

// A movie recommended to a user
typedef struct MovieRecom_CB{      
	int Movie_Id; 
	int Frequency;
	double Importance; 
} MovieRecom_CB;

// A user
typedef struct User_CB{      
	int User_Id;  
	int NbWatchedMovies;	
	WatchedMovie_CB LastWatchedMovies[MaxMovies];
} User_CB;

// An observation of a Markov chain
typedef struct Couple_CB{      
	int State;                    	 
	int Symbol;                         
} Couple_CB;

// A Markov chain
typedef struct MarkovChain_CB{      
	int NbCouples;                
	Couple_CB Elements[MaxMovies];        
} MC_CB;

// Displays the content of a Markov chain
void DisplayMC_CB(MC_CB Delta){
	int i;
	
	for(i=0;(i<Delta.NbCouples);i++){
		printf("(%i,%i)\t",Delta.Elements[i].State,Delta.Elements[i].Symbol);
	}
	printf("\n");
	return;
}

// Prints the content of a Markov chain in a file
void DisplayMCfile_CB(MC_CB Delta, FILE ** f){
	int i;
	
	for(i=0;(i<Delta.NbCouples);i++){
		fprintf((*f),"(%i,%i)",Delta.Elements[i].State,Delta.Elements[i].Symbol);
	}
	fprintf((*f),"\n");
	return;
}

// Reads the movie data 
int ReadMovies_CB(char * MoviesFileName, Movie_CB TheMovies[MaxMovies]){
	FILE * f;
	int i,j,k,Genre;
	char c;
	
	f = fopen(MoviesFileName, "rt");
	if(!f){
		printf("%s\n",MoviesFileName);
		return -1;
	} 
	i = 0;

	while(!feof(f)){
		fscanf(f,"%i,%i,",&(TheMovies[i].Movie_Id),&(TheMovies[i].Release_date));
		fscanf(f,"%g,%i,{",&(TheMovies[i].Vote_average),&(TheMovies[i].Vote_count));
		j = 0;
		do{
			fscanf(f,"%i%c",&Genre,&c);
			TheMovies[i].Genres[j] = Genre;
			j++;
			if((c != ',')&&(c != '}')){
				printf("Movie genre format [line %i] [movie_id %i]\n",i+1,TheMovies[i].Movie_Id);
				return -1;
			}
		}while(c != '}');
		TheMovies[i].NbGenres = j;
		
		fscanf(f,",#");
		k = 0;
		do{
			fscanf(f,"%c",&c);
			if(c != '#'){
				TheMovies[i].Title[k] = c;
				k++;
			}
		}while(c != '#');
		TheMovies[i].Title[k] = 0;
		fscanf(f,"\n");
		i++;
	}

	fclose(f);
	return i;
}

// Reads the user data 
int ReadUsers_CB(char * UsersFileName, User_CB TheUsers[MaxUsers], int Tmax){
	FILE * f;
	int i,j,k;
	char c;
	WatchedMovie_CB LastWatchedMovies[MaxMovies];
	
	f = fopen(UsersFileName, "rt");
	if(!f){
		printf("%s\n",UsersFileName);
		return -1;
	} 
	i = 0;
	
	while(!feof(f)){
		fscanf(f,"%i",&(TheUsers[i].User_Id));
		if(!feof(f)){
			j = 0;
			do{
				fscanf(f,"%c",&c);
				if(c == '('){
					fscanf(f,"%i,%f)",&(LastWatchedMovies[j].Movie_Id),&(LastWatchedMovies[j].Score));
					j++;
				}else if(c != '\n'){
					printf("User format\n");
					return -1;
				}
			}while(c != '\n');
			if((Tmax <= 0)||(j <= Tmax)){
			//if(j <= Tmax){
				TheUsers[i].NbWatchedMovies = j;
				for(k=0;(k < j); k++){
					TheUsers[i].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[k].Movie_Id;
					TheUsers[i].LastWatchedMovies[k].Score = LastWatchedMovies[k].Score;
				}
			}else{
				TheUsers[i].NbWatchedMovies = Tmax;
				for(k=0;(k < Tmax); k++){
					TheUsers[i].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[(j-Tmax)+k].Movie_Id;
					TheUsers[i].LastWatchedMovies[k].Score = LastWatchedMovies[(j-Tmax)+k].Score;
				}
			}
			i++;
		}
	}
	fclose(f);
	return i;
}

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


int FrequencyOf_Genres(Movie_CB TheMovies[MaxMovies], int NbMovies, int FrequencyOfGenres[MaxGenres], int TheGenres[MaxGenres]){
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

// Constructs the Markov chain associated with each user u
void ConstructMCsOfUser_CB(User_CB u, MC_CB TheMCs[MaxFeatures], int SequencesOfUser[MaxFeatures][MaxMovies], int NbFeatures, Movie_CB TheMovies[MaxMovies], int FrequencyOfGenres[MaxGenres], int TheGenres[MaxGenres], int NbClustersGenres, FILE ** f){
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
		MovieIndex = u.LastWatchedMovies[i].Movie_Id-1;
		if(MovieIndex != -1){
			TheMCs[0].Elements[i].Symbol = SymbolOf_Release_date(TheMovies[MovieIndex].Release_date);
			TheMCs[1].Elements[i].Symbol = SymbolOf_Vote_average(TheMovies[MovieIndex].Vote_average);
			TheMCs[2].Elements[i].Symbol = SymbolOf_Vote_count(TheMovies[MovieIndex].Vote_count);
			TheMCs[3].Elements[i].Symbol = SymbolOf_Genres(TheMovies[MovieIndex].Genres,TheMovies[MovieIndex].NbGenres,FrequencyOfGenres,TheGenres,NbClustersGenres);
			
			for(j=0;(j < NbFeatures);j++){
				SequencesOfUser[j][i] = TheMCs[j].Elements[i].Symbol - 1;
			}
		}
	}

	fprintf(*f,"_______________________________________________________________\n");
	for(j=0;(j < NbFeatures);j++){
		fprintf(*f,"delta(u_%i,%i):\t",u.User_Id,j+1);
		DisplayMCfile_CB(TheMCs[j],f);
	}
	
	return;
}


// Constructs all the initial HMMs associated with each user
int InitialHMMsOfUser_CB(MC_CB TheMCsOfUser[MaxFeatures], int NbFeatures, HMM Lambda[MaxFeatures]){
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

// Function of partitionnement for the QuickSortKNN
int PartitionKNN_CB(double distances[MaxUsers], int indices[MaxUsers], int low, int high) {
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
void QuicksortKNN_CB(double distances[MaxUsers], int indices[MaxUsers], int low, int high) {
    if (low < high) {
        int pi = PartitionKNN_CB(distances, indices, low, high);
        QuicksortKNN_CB(distances, indices, low, pi - 1);
        QuicksortKNN_CB(distances, indices, pi + 1, high);
    }
}



// KNN algorithm 
void KNN_CB(int NbNeighbours, int NbVectors, int NbComponents, int Dist, double FinalVect[MaxUsers][MaxFeatures*MaxSymbols], int TheNeighbours[MaxUsers][MaxNeighbours], double NeighbourDistances[MaxUsers][MaxNeighbours]) {
    int i,j,m, indices[MaxUsers];
	double 	distances[MaxUsers];

    for (i = 0; i < NbVectors; i++) {

        // computes the distance between the i-th vector and all the system vectors
        for (j = 0; j < NbVectors; j++) {
            if (i != j) {
				if(Dist == 0){
					distances[j] = EuclideanDistance(FinalVect[i], FinalVect[j], NbComponents);
				}else {
					distances[j] = ManhattanDistance(FinalVect[i], FinalVect[j], NbComponents);
				}
            } else {
				// An infinite distance between the i-th vector and itself
                distances[j] = DBL_MAX; // 
            }
            indices[j] = j;
        }

        // QuickSort of the distances and the indices
		QuicksortKNN_CB(distances,indices,0,NbVectors-1);
        // Records the K nearest neighbors and the corresponding distances
        for (m = 0; m < NbNeighbours; m++) {
            TheNeighbours[i][m] = indices[m];
			NeighbourDistances[i][m] = distances[m];
        }
    }
	return;
}

void DistancesMovieVectors_CB(double TheMovieVectors[MaxMovies][MaxFeatures*MaxSymbols], int NbMovies, int NbComponents, double Distances[2][MaxMovies][MaxMovies], double Distance_Max[2]){
	int j,l;
	
	/**
    for (j = 0; j < NbMovies; j++) {
		printf("%i,[",j+1);
		for (k = 0; k < NbComponents-1; k++) {
			printf("%g,",TheMovieVectors[j][k]);
		}	
		printf("%g]\n",TheMovieVectors[j][NbComponents-1]);
	}
	**/
	Distance_Max[0] = 0.0;
	Distance_Max[1] = 0.0;
    for (j = 0; j < NbMovies; j++) {
		for (l = 0; l < NbMovies; l++) {
			if(j != l){
				Distances[0][j][l] = EuclideanDistance(TheMovieVectors[j], TheMovieVectors[l], NbComponents);
				Distances[1][j][l] = ManhattanDistance(TheMovieVectors[j], TheMovieVectors[l], NbComponents);
			}else{
				Distances[0][j][l] = 0.0;
				Distances[1][j][l] = 0.0;
			}
			if(Distances[0][j][l] > Distance_Max[0]){
				Distance_Max[0] = Distances[0][j][l];
			}
			if(Distances[1][j][l] > Distance_Max[1]){
				Distance_Max[1] = Distances[1][j][l];
			}
		}
	}

	return;
}

// Index of a recommended movie in the array 'TheMovieRecom' according to its 'Movie_Id'
int IndexOfMovieRecom_CB(int Movie_Id, MovieRecom_CB TheMovieRecom[MaxNeighbours*MaxMovies], int NbMovieRecom){
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
void InitMovieRecom_CB(MovieRecom_CB TheMovieRecom[MaxUsers][MaxNeighbours*MaxMovies], int NbMovieRecom[MaxUsers], int TheNeighbours[MaxUsers][MaxNeighbours],  int NbNeighbours, User_CB TheUsers[MaxUsers], int NbUsers, double Distances[MaxMovies][MaxMovies]){
	int i,j,k,l,m,n;
	
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
				n = IndexOfMovieRecom_CB(m,TheMovieRecom[i],NbMovieRecom[i]);
				if(n == -1){
					/* Inserting a new recommended movie */
					TheMovieRecom[i][NbMovieRecom[i]].Movie_Id = m;
					TheMovieRecom[i][NbMovieRecom[i]].Frequency = 1;
					TheMovieRecom[i][NbMovieRecom[i]].Importance = 0.0;
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
				TheMovieRecom[i][j].Importance *= TheMovieRecom[i][j].Frequency;
				TheMovieRecom[i][j].Importance = log(TheMovieRecom[i][j].Importance);
			}
		}
	}
	
	return;
}


// Function of partitionnement for the QuickSortMovies
int PartitionMovies_CB(MovieRecom_CB TheMovieRecom[MaxNeighbours*MaxMovies], int low, int high) {
    float pivot = TheMovieRecom[high].Importance; // pivot
    int i = (low - 1); // Index of the highest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is greater than pivot
        if (TheMovieRecom[j].Importance > pivot) {
            i++; // Increments the index of the highest element 
			
            swapInt(&TheMovieRecom[i].Movie_Id, &TheMovieRecom[j].Movie_Id);
            swapInt(&TheMovieRecom[i].Frequency, &TheMovieRecom[j].Frequency);
            swapDouble(&TheMovieRecom[i].Importance, &TheMovieRecom[j].Importance);
        }
    }
    swapInt(&TheMovieRecom[i+1].Movie_Id, &TheMovieRecom[high].Movie_Id);
    swapInt(&TheMovieRecom[i+1].Frequency, &TheMovieRecom[high].Frequency);
    swapDouble(&TheMovieRecom[i+1].Importance, &TheMovieRecom[high].Importance);
    return (i + 1);
}


// QuickSort Movies
void QuicksortMovies_CB(MovieRecom_CB TheMovieRecom[MaxNeighbours*MaxMovies], int low, int high) {
    if (low < high) {
        int pi = PartitionMovies_CB(TheMovieRecom, low, high);
        QuicksortMovies_CB(TheMovieRecom, low, pi - 1);
        QuicksortMovies_CB(TheMovieRecom, pi + 1, high);
    }
}

// Sort the recommended movies in decrease order of their importance 
void SortMovieRecom_by_Importance_CB(MovieRecom_CB TheMovieRecom[MaxUsers][MaxNeighbours*MaxMovies], int NbMovieRecom[MaxUsers], int NbUsers){
	int i;
	
	for (i = 0; i < NbUsers; i++) {
		QuicksortMovies_CB(TheMovieRecom[i],0,NbMovieRecom[i]-1);
	}
	return;
}


float PredictedScore(int Movie_Id, int User_Index, User_CB TheUsers[MaxUsers], double Distances[MaxMovies][MaxMovies], double Distance_Max){
	double distMin, dist;
	int i,j, IndexMin;
	float Score;
	
	if(TheUsers[User_Index].NbWatchedMovies == 0){
		return 0.0;
	}
	
	j = TheUsers[User_Index].LastWatchedMovies[0].Movie_Id-1;
	distMin = Distances[Movie_Id-1][j];
	IndexMin = 0;
	
	for(i=1;(i < TheUsers[User_Index].NbWatchedMovies);i++){
		j = TheUsers[User_Index].LastWatchedMovies[i].Movie_Id-1;
		dist = Distances[Movie_Id-1][j];
		if(dist < distMin){
			distMin = dist;
			IndexMin = i;
		}
	}
	
	Score = TheUsers[User_Index].LastWatchedMovies[IndexMin].Score;
	Score = Score*(1.0 - (distMin/Distance_Max));
	return Score;
}

// Computes the F1-measure, the Precision, the Recall, the RMSE and the MAE of the system
int ComputeMetrics_CB(User_CB TheUsers[MaxUsers], int NbUsers, MovieRecom_CB TheMovieRecom[MaxUsers][MaxNeighbours*MaxMovies], int MaxRecoms, double Distances[MaxMovies][MaxMovies], double Distance_Max, double Metrics[5], double std_dev[3], FILE ** f){
	double F1_user[MaxUsers], Precision_user[MaxUsers], Recall_user[MaxUsers], Intersect, diff;
	int i,j,k,l,n,BadRecoms,GoodRecoms, Good[MaxMovies];
	float PredictScore;
	
	Metrics[0] = 0.0;  // overall F1
	Metrics[1] = 0.0;  // overall Precision
	Metrics[2] = 0.0;  // overall Recall
	Metrics[3] = 0.0;  // RMSE
	Metrics[4] = 0.0;  // MAE

	BadRecoms = 0;
	n = 0;
	
	for (i = 0; i < NbUsers; i++) {
		Intersect = 0.0;
		GoodRecoms = 0;
		
		for(j=0;(j < MaxRecoms);j++){
			Good[j] = 0;
		}
		
		for(j=0;(j < TheUsers[i].NbWatchedMovies);j++){
			for (k = 0; (k < MaxRecoms); k++) {
				if(TheUsers[i].LastWatchedMovies[j].Movie_Id == TheMovieRecom[i][k].Movie_Id){
					Intersect += 1.0;
					GoodRecoms++;
					Good[k] = 1;
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
				PredictScore = PredictedScore(TheMovieRecom[i][j].Movie_Id,i,TheUsers,Distances,Distance_Max);
				if(l == 0){
					fprintf((*f),"%i,[(%i,%.1f)",TheUsers[i].User_Id,TheMovieRecom[i][j].Movie_Id,PredictScore);
				}else{
					fprintf((*f),",(%i,%.1f)",TheMovieRecom[i][j].Movie_Id,PredictScore);
				}
				l++;
			}
		}
		if(l != 0){
			fprintf((*f),"]\n");
		}
		
		Precision_user[i] = (Intersect / MaxRecoms);
		Recall_user[i] = (Intersect / TheUsers[i].NbWatchedMovies);
		F1_user[i] = (2.0*Precision_user[i]*Recall_user[i])/(Precision_user[i] + Recall_user[i] + MinDouble);
		
		Metrics[0] += F1_user[i];
		Metrics[1] += Precision_user[i];
		Metrics[2] += Recall_user[i];
		
		BadRecoms += (MaxRecoms - GoodRecoms);
	}	
	
	for (j = 0; j <= 2; j++) {
		Metrics[j] = (Metrics[j]/NbUsers); 
		std_dev[j] = 0.0;
	}
	Metrics[3] = sqrt(Metrics[3]/n);
	Metrics[4] = Metrics[4]/n;
	
	for (i = 0; i < NbUsers; i++) {
		std_dev[0] += (F1_user[i] - Metrics[0])*(F1_user[i] - Metrics[0]);
		std_dev[1] += (Precision_user[i] - Metrics[1])*(Precision_user[i] - Metrics[1]);
		std_dev[2] += (Recall_user[i] - Metrics[2])*(Recall_user[i] - Metrics[2]);
	}
	
	
	for (j = 0; j <= 2; j++) {
		std_dev[j] /= NbUsers;
		std_dev[j] = sqrt(std_dev[j]);
	}
	
	return BadRecoms;
}
