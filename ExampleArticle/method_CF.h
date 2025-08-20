#define MaxUsers MaxLenMC                /* Max number of users */ 
#define MaxMovies 10                     /* Max number of movies */ 
#define MaxWatchedMovies 10	             /* Max number of movies watched by a user */
#define MaxUsersWatching 10	             /* Max number of users who watched a movie */
#define MaxOccupations 22                /* Max number of occupations */
#define MaxZipCodeGroups 20              /* Max number of Zipcode groups */ 
#define MaxFeatures 4                    /* Max number of features (CF or CB) */
#define MaxGenres 25

// A movie watched by a user
typedef struct WatchedMovie{      
	int Movie_Id;                    	 
	float Score;  
	int Timestamp;
} WatchedMovie;

// A user
typedef struct User{      
	int User_Id;  
	int age;
	char gender;
	char occupation[50];
	char zipCode[6];
	int NbWatchedMovies;   	
	WatchedMovie * LastWatchedMovies;
} User;

// A user who watched a movie
typedef struct UsersWatching{      
	int User_Id;                    	 
	float Score;                         
	int Timestamp;
} UsersWatching;

// A movie
typedef struct Movie{      
	int Movie_Id;                    	 
	int Release_date;  
	float Vote_average;
	int Vote_count;
	int Genres[MaxGenres];
	int NbGenres;
	char Title[100];
	int NbUsersWatching;	
	UsersWatching * TheUsersWatching;
} Movie;

// A movie recommended to a user
typedef struct MovieRecom{      
	int Movie_Id; 
	int Frequency;
	double Distance;
	double Importance; 
	float PredictedScore;
} MovieRecom;


// Reads the movie data 
int ReadMovies(char * MoviesFileName, Movie * TheMovies){
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
int ReadUsers(char * UsersFileName, User * TheUsers){
	FILE * f;
	int i,j;
	char c;
	
	f = fopen(UsersFileName, "rt");
	if(!f){
		printf("%s\n",UsersFileName);
		return -1;
	} 
	i = 0;
	
	while(!feof(f)){
		fscanf(f,"%i|%i|%c|",&(TheUsers[i].User_Id),&(TheUsers[i].age),&(TheUsers[i].gender));
		if(!feof(f)){
			j = 0;
			do{
				fscanf(f,"%c",&c);
				if(c != '|'){
					TheUsers[i].occupation[j] = c;
					j++;
				}
			}while(c != '|');
			TheUsers[i].occupation[j] = 0;
		
			j = 0;
			do{
				fscanf(f,"%c",&c);
				if(c != '\n'){
					TheUsers[i].zipCode[j] = c;
					j++;
				}
			}while(c != '\n');
			TheUsers[i].zipCode[j] = 0;
			i++;
		}
	}
	fclose(f);
	return i;
}

// Reads the ratings data ordered by user ids 
int ReadRatings_Users(char * RatingsFileName, User * TheUsersTmp, int * NbUsers, int TmaxWatchedMovies, char * UsersSequentialFileName){
	FILE *f, *g;
	int i,j,k,user_id,movie_id,timestamp;
	char c, FileName[100];
	float rating;

	int Nb_users,new_user,NbWatchedMovies,CurrentUserId,*TheUsersId;
	WatchedMovie * LastWatchedMovies;

	TheUsersId  = (int *)malloc(MaxUsers*sizeof(int));
	LastWatchedMovies = (WatchedMovie *)malloc(MaxMovies*sizeof(WatchedMovie));

	strcpy(FileName,RatingsFileName);
	strcat(FileName,"_users.csv");
	f = fopen(FileName, "rt");
	if(!f){
		printf("%s\n",FileName);
		return -1;
	} 

	g = fopen(UsersSequentialFileName, "wt");
	if(!g){
		printf("%s\n",UsersSequentialFileName);
		return -1;
	} 

	// Skipping the first line
	do{
		fscanf(f,"%c",&c);
	}while(c != '\n');
	
	i = 0;
	Nb_users = 0;
	fscanf(f,"%i;%i;%f;%i\n",&user_id,&movie_id,&rating,&timestamp);
	CurrentUserId = user_id;
	NbWatchedMovies = 0;
	
	while(!feof(f)){
		if(user_id != CurrentUserId){
			j = CurrentUserId;
			TheUsersTmp[j-1].User_Id = j;
			if(NbWatchedMovies <= TmaxWatchedMovies){
				TheUsersTmp[j-1].NbWatchedMovies = NbWatchedMovies;
				for(k=0;(k < NbWatchedMovies); k++){
					TheUsersTmp[j-1].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[k].Movie_Id;
					TheUsersTmp[j-1].LastWatchedMovies[k].Score = LastWatchedMovies[k].Score;
					TheUsersTmp[j-1].LastWatchedMovies[k].Timestamp = LastWatchedMovies[k].Timestamp;
				}
			}else{
				TheUsersTmp[j-1].NbWatchedMovies = TmaxWatchedMovies;
				for(k=0;(k < TmaxWatchedMovies); k++){
					TheUsersTmp[j-1].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[(NbWatchedMovies - TmaxWatchedMovies) + k].Movie_Id;
					TheUsersTmp[j-1].LastWatchedMovies[k].Score = LastWatchedMovies[(NbWatchedMovies - TmaxWatchedMovies) + k].Score;
					TheUsersTmp[j-1].LastWatchedMovies[k].Timestamp = LastWatchedMovies[(NbWatchedMovies - TmaxWatchedMovies) + k].Timestamp;
				}
			}

			fprintf(g,"%i",j);
			for(k=0;(k < NbWatchedMovies); k++){
				fprintf(g,"(%i,%.1f)",LastWatchedMovies[k].Movie_Id,LastWatchedMovies[k].Score);
			}
			fprintf(g,"\n");

			for(k=CurrentUserId+1;(k <= user_id-1); k++){
				fprintf(g,"%i\n",k);
				TheUsersTmp[k-1].User_Id = k;
				TheUsersTmp[k-1].NbWatchedMovies = 0;
			}

			CurrentUserId = user_id;
			NbWatchedMovies = 0;
		}

		LastWatchedMovies[NbWatchedMovies].Movie_Id = movie_id;
		LastWatchedMovies[NbWatchedMovies].Score = rating;
		LastWatchedMovies[NbWatchedMovies].Timestamp = timestamp;
		(NbWatchedMovies)++;
		
		j = 0;
		new_user = 1;
		do{
			if(j < Nb_users){
				if(TheUsersId[j] == user_id){
					new_user = 0;
				}
			}
			j++;
		}while((j<Nb_users)&&(new_user == 1));
		
		if(new_user == 1){
			TheUsersId[Nb_users] = user_id;
			Nb_users++;
		}

		i++;
		fscanf(f,"%i;%i;%f;%i\n",&user_id,&movie_id,&rating,&timestamp);
		if(feof(f)){
			if(user_id == CurrentUserId){
				LastWatchedMovies[NbWatchedMovies].Movie_Id = movie_id;
				LastWatchedMovies[NbWatchedMovies].Score = rating;
				LastWatchedMovies[NbWatchedMovies].Timestamp = timestamp;
				(NbWatchedMovies)++;
			}

			j = CurrentUserId;
			TheUsersTmp[j-1].User_Id = j;
			if(NbWatchedMovies <= TmaxWatchedMovies){
				TheUsersTmp[j-1].NbWatchedMovies = NbWatchedMovies;
				for(k=0;(k < NbWatchedMovies); k++){
					TheUsersTmp[j-1].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[k].Movie_Id;
					TheUsersTmp[j-1].LastWatchedMovies[k].Score = LastWatchedMovies[k].Score;
					TheUsersTmp[j-1].LastWatchedMovies[k].Timestamp = LastWatchedMovies[k].Timestamp;
				}
			}else{
				TheUsersTmp[j-1].NbWatchedMovies = TmaxWatchedMovies;
				for(k=0;(k < TmaxWatchedMovies); k++){
					TheUsersTmp[j-1].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[(NbWatchedMovies - TmaxWatchedMovies) + k].Movie_Id;
					TheUsersTmp[j-1].LastWatchedMovies[k].Score = LastWatchedMovies[(NbWatchedMovies - TmaxWatchedMovies) + k].Score;
					TheUsersTmp[j-1].LastWatchedMovies[k].Timestamp = LastWatchedMovies[(NbWatchedMovies - TmaxWatchedMovies) + k].Timestamp;
				}
			}

			fprintf(g,"%i",j);
			for(k=0;(k < NbWatchedMovies); k++){
				fprintf(g,"(%i,%.1f)",LastWatchedMovies[k].Movie_Id,LastWatchedMovies[k].Score);
			}
			fprintf(g,"\n");
			
			if(user_id != CurrentUserId){
				for(k=CurrentUserId+1;(k <= user_id-1); k++){
					fprintf(g,"%i\n",k);
					TheUsersTmp[k-1].User_Id = k;
					TheUsersTmp[k-1].NbWatchedMovies = 0;
				}
				TheUsersTmp[user_id-1].User_Id = user_id;
				TheUsersTmp[user_id-1].NbWatchedMovies = 1;
				TheUsersTmp[user_id-1].LastWatchedMovies[0].Movie_Id = movie_id;
				TheUsersTmp[user_id-1].LastWatchedMovies[0].Score = rating;
				TheUsersTmp[user_id-1].LastWatchedMovies[0].Timestamp = timestamp;
				
				fprintf(g,"%i(%i,%.1f)\n",user_id,movie_id,rating);
				Nb_users++;
			}
			i++;
		}
	}
	fclose(f);
	fclose(g);

	free(TheUsersId);
	free(LastWatchedMovies);
	
	(*NbUsers) = Nb_users;
	return i;
}


// Reads the ratings data ordered by movie ids 
int ReadRatings_Movies(char * RatingsFileName, Movie * TheMoviesTmp, int * NbMovies, int TmaxUsersWatching, char * MoviesSequentialFileName){
	FILE *f, *g;
	int i,j,k,user_id,movie_id,timestamp;
	char c, FileName[100];
	float rating;

	int Nb_movies,new_movie,NbUsersWatching,CurrentMovieId,*TheMoviesId;
	UsersWatching * TheUsersWatching;

	TheMoviesId  = (int *)malloc(MaxMovies*sizeof(int));
	TheUsersWatching = (UsersWatching *)malloc(MaxUsers*sizeof(UsersWatching));

	strcpy(FileName,RatingsFileName);
	strcat(FileName,"_movies.csv");
	f = fopen(FileName, "rt");
	if(!f){
		printf("%s\n",FileName);
		return -1;
	} 

	g = fopen(MoviesSequentialFileName, "wt");
	if(!g){
		printf("%s\n",MoviesSequentialFileName);
		return -1;
	} 

	// Skipping the first line
	do{
		fscanf(f,"%c",&c);
	}while(c != '\n');
	
	i = 0;
	Nb_movies = 0;
	fscanf(f,"%i;%i;%f;%i\n",&movie_id,&user_id,&rating,&timestamp);
	CurrentMovieId = movie_id;
	NbUsersWatching = 0;
	
	while(!feof(f)){
		if(movie_id != CurrentMovieId){
			j = CurrentMovieId;
			TheMoviesTmp[j-1].Movie_Id = j;
			if(NbUsersWatching <= TmaxUsersWatching){
				TheMoviesTmp[j-1].NbUsersWatching = NbUsersWatching;
				for(k=0;(k < NbUsersWatching); k++){
					TheMoviesTmp[j-1].TheUsersWatching[k].User_Id = TheUsersWatching[k].User_Id;
					TheMoviesTmp[j-1].TheUsersWatching[k].Score = TheUsersWatching[k].Score;
					TheMoviesTmp[j-1].TheUsersWatching[k].Timestamp = TheUsersWatching[k].Timestamp;
				}
			}else{
				TheMoviesTmp[j-1].NbUsersWatching = TmaxUsersWatching;
				for(k=0;(k < TmaxUsersWatching); k++){
					TheMoviesTmp[j-1].TheUsersWatching[k].User_Id = TheUsersWatching[(NbUsersWatching - TmaxUsersWatching) + k].User_Id;
					TheMoviesTmp[j-1].TheUsersWatching[k].Score = TheUsersWatching[(NbUsersWatching - TmaxUsersWatching) + k].Score;
					TheMoviesTmp[j-1].TheUsersWatching[k].Timestamp = TheUsersWatching[(NbUsersWatching - TmaxUsersWatching) + k].Timestamp;
				}
			}

			fprintf(g,"%i",j);
			for(k=0;(k < NbUsersWatching); k++){
				fprintf(g,"(%i,%.1f)",TheUsersWatching[k].User_Id,TheUsersWatching[k].Score);
			}
			fprintf(g,"\n");

			for(k=CurrentMovieId+1;(k <= movie_id-1); k++){
				fprintf(g,"%i\n",k);
				TheMoviesTmp[k-1].Movie_Id = k;
				TheMoviesTmp[k-1].NbUsersWatching = 0;
			}
			CurrentMovieId = movie_id;
			NbUsersWatching = 0;
		}

		TheUsersWatching[NbUsersWatching].User_Id = user_id;
		TheUsersWatching[NbUsersWatching].Score = rating;
		TheUsersWatching[NbUsersWatching].Timestamp = timestamp;
		(NbUsersWatching)++;
		
		j = 0;
		new_movie = 1;
		do{
			if(j < Nb_movies){
				if(TheMoviesId[j] == movie_id){
					new_movie = 0;
				}
			}
			j++;
		}while((j<Nb_movies)&&(new_movie == 1));
		
		if(new_movie == 1){
			TheMoviesId[Nb_movies] = movie_id;
			Nb_movies++;
		}

		i++;
		fscanf(f,"%i;%i;%f;%i\n",&movie_id,&user_id,&rating,&timestamp);
		if(feof(f)){
			if(movie_id == CurrentMovieId){
				TheUsersWatching[NbUsersWatching].User_Id = user_id;
				TheUsersWatching[NbUsersWatching].Score = rating;
				TheUsersWatching[NbUsersWatching].Timestamp = timestamp;
				(NbUsersWatching)++;
			}

			j = CurrentMovieId;
			TheMoviesTmp[j-1].Movie_Id = j;
			if(NbUsersWatching <= TmaxUsersWatching){
				TheMoviesTmp[j-1].NbUsersWatching = NbUsersWatching;
				for(k=0;(k < NbUsersWatching); k++){
					TheMoviesTmp[j-1].TheUsersWatching[k].User_Id = TheUsersWatching[k].User_Id;
					TheMoviesTmp[j-1].TheUsersWatching[k].Score = TheUsersWatching[k].Score;
					TheMoviesTmp[j-1].TheUsersWatching[k].Timestamp = TheUsersWatching[k].Timestamp;
				}
			}else{
				TheMoviesTmp[j-1].NbUsersWatching = TmaxUsersWatching;
				for(k=0;(k < TmaxUsersWatching); k++){
					TheMoviesTmp[j-1].TheUsersWatching[k].User_Id = TheUsersWatching[(NbUsersWatching - TmaxUsersWatching) + k].User_Id;
					TheMoviesTmp[j-1].TheUsersWatching[k].Score = TheUsersWatching[(NbUsersWatching - TmaxUsersWatching) + k].Score;
					TheMoviesTmp[j-1].TheUsersWatching[k].Timestamp = TheUsersWatching[(NbUsersWatching - TmaxUsersWatching) + k].Timestamp;
				}
			}

			fprintf(g,"%i",j);
			for(k=0;(k < NbUsersWatching); k++){
				fprintf(g,"(%i,%.1f)",TheUsersWatching[k].User_Id,TheUsersWatching[k].Score);
			}
			fprintf(g,"\n");
			
			if(movie_id != CurrentMovieId){
				for(k=CurrentMovieId+1;(k <= movie_id-1); k++){
					fprintf(g,"%i\n",k);
					TheMoviesTmp[k-1].Movie_Id = k;
					TheMoviesTmp[k-1].NbUsersWatching = 0;
				}
				TheMoviesTmp[movie_id-1].Movie_Id = movie_id;
				TheMoviesTmp[movie_id-1].NbUsersWatching = 1;
				TheMoviesTmp[movie_id-1].TheUsersWatching[0].User_Id = user_id;
				TheMoviesTmp[movie_id-1].TheUsersWatching[0].Score = rating;
				TheMoviesTmp[movie_id-1].TheUsersWatching[0].Timestamp = timestamp;
				
				fprintf(g,"%i(%i,%.1f)\n",movie_id,user_id,rating);
				Nb_movies++;
			}
			i++;
		}
	}
	fclose(f);
	fclose(g);

	free(TheMoviesId);
	free(TheUsersWatching);
	
	(*NbMovies) = Nb_movies;
	return i;
}

void UpdateUsers(User * TheUsersTmp, User * TheUsers, int NbUsers){
	int i,j,k;
	
	for(i=0;(i < NbUsers);i++){
		j = 0;
		while(TheUsersTmp[j].User_Id != TheUsers[i].User_Id){
			j++;
		}
		TheUsers[i].NbWatchedMovies = TheUsersTmp[j].NbWatchedMovies;
		for(k=0;(k < TheUsersTmp[j].NbWatchedMovies); k++){
			TheUsers[i].LastWatchedMovies[k].Movie_Id = TheUsersTmp[j].LastWatchedMovies[k].Movie_Id;
			TheUsers[i].LastWatchedMovies[k].Score = TheUsersTmp[j].LastWatchedMovies[k].Score;
			TheUsers[i].LastWatchedMovies[k].Timestamp = TheUsersTmp[j].LastWatchedMovies[k].Timestamp;
		}
	}
	return;
}

void UpdateMovies(Movie * TheMoviesTmp, Movie * TheMovies, int NbMovies){
	int i,j,k;
	
	for(i=0;(i < NbMovies);i++){
		j = 0;
		while(TheMoviesTmp[j].Movie_Id != TheMovies[i].Movie_Id){
			j++;
		}
		TheMovies[i].NbUsersWatching = TheMoviesTmp[j].NbUsersWatching;
		for(k=0;(k < TheMoviesTmp[j].NbUsersWatching); k++){
			TheMovies[i].TheUsersWatching[k].User_Id = TheMoviesTmp[j].TheUsersWatching[k].User_Id;
			TheMovies[i].TheUsersWatching[k].Score = TheMoviesTmp[j].TheUsersWatching[k].Score;
			TheMovies[i].TheUsersWatching[k].Timestamp = TheMoviesTmp[j].TheUsersWatching[k].Timestamp;
		}
	}
	return;
}

// Sampling of the Score to obtain the corresponding state
int StateOf_CF(float Score){
	int state;
	
	if((0.0 <= Score)&&(Score <= 1.0))state = 1;
	else if((1.0 < Score)&&(Score <= 2.0))state = 2; 
	else if((2.0 < Score)&&(Score <= 3.0))state = 3; 
	else if((3.0 < Score)&&(Score <= 4.0))state = 4; 
	else if(4.0 < Score)state = 5; 
	
	return state;
}

// Sampling of the Age to obtain the corresponding symbol
int SymbolOf_Age(int Age){
	int symbol;
	
	if(Age <= 11)symbol = 1;                          // Child
	else if((12 <= Age)&&(Age <= 18))symbol = 2;      // Adolescent
	else if((19 <= Age)&&(Age <= 30))symbol = 3;      // Young
	else if((31 <= Age)&&(Age <= 45))symbol = 4;      // Adult
	else if((46 <= Age)&&(Age <= 60))symbol = 5;      // Mature
	else if((61 <= Age)&&(Age <= 75))symbol = 6;      // Old
	else if(76 <= Age)symbol = 7;                     // Advanced age
	
	return symbol;
}

int FrequencyOf_Occupations(char TheOccupations[MaxOccupations][50], int FreqOfThe_Occupations[MaxOccupations]){
	int i, Nb_Occupations;
	char Index[5];
	int Frequency[20] = {27,105,45,32,79,196,12,95,31,18,66,51,7,28,67,26,9,16,14,12};
	
	Nb_Occupations = 20;
	for(i=0;(i < Nb_Occupations);i++){
		FreqOfThe_Occupations[i] = Frequency[i];
		itoa(i+1,Index,10);
		strcpy(TheOccupations[i],"o");
		strcat(TheOccupations[i],Index);
	}
	
	return Nb_Occupations;
}


// Symbol corresponding to any occupation
int SymbolOf_Occupation(char Occupation[50], char TheOccupations[MaxOccupations][50], int FreqOfThe_Occupations[MaxOccupations], int NbOccupations){
	int i, symbol, Freq;
	
	i = 0;
	while((i < NbOccupations)&&(strcmp(Occupation,TheOccupations[i]) != 0)){
		i++;
	}
	if(i >= NbOccupations){
		return -1;
	}

	Freq = FreqOfThe_Occupations[i];

	if(Freq <= 10)symbol = 1;                          
	else if((11 <= Freq)&&(Freq <= 20))symbol = 2;      
	else if((21 <= Freq)&&(Freq <= 40))symbol = 3;      
	else if((41 <= Freq)&&(Freq <= 70))symbol = 4;      
	else if((71 <= Freq)&&(Freq <= 100))symbol = 5;      
	else if((101 <= Freq)&&(Freq <= 150))symbol = 6;      
	else if(151 <= Freq)symbol = 7;                     
	
	return symbol;
}

int FrequencyOf_ZipCodeGroups(char FirstChar_ZipCodeGroup[MaxZipCodeGroups], int FreqOfThe_ZipCodeGroup[MaxZipCodeGroups]){
	int i, Nb_ZipCodeGroups;
	int Frequency[18] = {96,97,101,62,77,110,78,67,56,160,2,1,2,2,2,1,2,20};
	char FirstChar_ZipCode[18] = {'0','1','2','3','4','5','6','7','8','9','E','K','L','M','N','R','T','V'};
	
	Nb_ZipCodeGroups = 18;
	for(i=0;(i < Nb_ZipCodeGroups);i++){
		FreqOfThe_ZipCodeGroup[i] = Frequency[i];
		FirstChar_ZipCodeGroup[i] = FirstChar_ZipCode[i];
	}
	
	return Nb_ZipCodeGroups;
}


// Sampling of the ZipCode to obtain the corresponding symbol
int SymbolOf_ZipCode(char ZipCode[6], char FirstChar_ZipCodeGroup[MaxZipCodeGroups], int FreqOfThe_ZipCodeGroup[MaxZipCodeGroups], int Nb_ZipCodeGroups){
	int i, symbol, Freq;
	
	i = 0;
	while((i < Nb_ZipCodeGroups)&&(ZipCode[0] != FirstChar_ZipCodeGroup[i])){
		i++;
	}
	if(i >= Nb_ZipCodeGroups){
		return -1;
	}

	Freq = FreqOfThe_ZipCodeGroup[i];

	if(Freq <= 10)symbol = 1;                          
	else if((11 <= Freq)&&(Freq <= 50))symbol = 2;      
	else if((51 <= Freq)&&(Freq <= 70))symbol = 3;      
	else if((71 <= Freq)&&(Freq <= 100))symbol = 4;      
	else if((101 <= Freq)&&(Freq <= 150))symbol = 5;      
	else if(151 <= Freq)symbol = 6;                     
	
	return symbol;
}


// Index of a user in the array 'TheUsers' according to its 'User_Id'
int IndexOfUser(int User_Id, User * TheUsers, int NbUsers){
	int i;
	
	for(i=0;(i < NbUsers);i++){
		if(TheUsers[i].User_Id == User_Id){
			return i;
		}
	}
	return -1;
}


// Constructs the Markov chain associated with each movie m
void ConstructMCsOfMovie(Movie m, MC * TheMCs, int ** Sequences, int NbFeatures, User * TheUsers, int NbUsers, char TheOccupations[MaxOccupations][50], int FreqOfThe_Occupations[MaxOccupations], int NbOccupations, char FirstChar_ZipCodeGroup[MaxZipCodeGroups], int FreqOfThe_ZipCodeGroup[MaxZipCodeGroups], int Nb_ZipCodeGroups, FILE ** f){
	int i, j, UserIndex, state;
	
	for(j=0;(j < NbFeatures);j++){
		TheMCs[j].NbCouples = m.NbUsersWatching;
	}

	for(i=0;(i < m.NbUsersWatching);i++){
		/* States */
		state = StateOf_CF(m.TheUsersWatching[i].Score);
		for(j=0;(j < NbFeatures);j++){
			TheMCs[j].Elements[i].State = state;
		}
		
		/* Symbols */
		UserIndex = IndexOfUser(m.TheUsersWatching[i].User_Id,TheUsers,NbUsers);
		if(UserIndex != -1){
			TheMCs[0].Elements[i].Symbol = SymbolOf_Age(TheUsers[UserIndex].age);
			TheMCs[1].Elements[i].Symbol = (TheUsers[UserIndex].gender == 'M') ? 1 : 2;
			TheMCs[2].Elements[i].Symbol = SymbolOf_Occupation(TheUsers[UserIndex].occupation,TheOccupations,FreqOfThe_Occupations,NbOccupations);
			TheMCs[3].Elements[i].Symbol = SymbolOf_ZipCode(TheUsers[UserIndex].zipCode,FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup,Nb_ZipCodeGroups);
			
			for(j=0;(j < NbFeatures);j++){
				Sequences[j][i] = TheMCs[j].Elements[i].Symbol - 1;
			}
		}
	}

	fprintf(*f,"_______________________________________________________________\n");
	for(j=0;(j < NbFeatures);j++){
		fprintf(*f,"delta(m_%i,%i):\t",m.Movie_Id,j+1);
		DisplayMCfile(TheMCs[j],f);
	}
	
	return;
}


// Constructs all the initial HMMs associated with each movie
int InitialHMMsOfMovie(MC * TheMCsOfMovie, int NbFeatures, HMM Lambda[MaxFeatures]){
	int NbSymbols, i,j,k,IndexFeature,State,NextState,Symbol,From[MaxStates],UseOfState[MaxStates];
	double Sum;
	
	NbSymbols = 0;
	
	for(IndexFeature=0;(IndexFeature < NbFeatures);IndexFeature++){
		
		Lambda[IndexFeature].N = 5; 
		
		switch(IndexFeature){
			case 0:{
				/* Age */
				Lambda[IndexFeature].M = 7; 
				break;				
			}
			case 1:{
				/* Gender */
				Lambda[IndexFeature].M = 2; 
				break;				
			}
			case 2:{
				/* Occupation */
				Lambda[IndexFeature].M = 7; 
				break;				
			}
			case 3:{
				/* ZipCode */
				Lambda[IndexFeature].M = 6; 
				break;				
			}
			default:{
				return -1;				
			}
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
		
		if(TheMCsOfMovie[IndexFeature].NbCouples == 1){
			State = TheMCsOfMovie[IndexFeature].Elements[0].State - 1;
			Symbol = TheMCsOfMovie[IndexFeature].Elements[0].Symbol - 1;

			UseOfState[State] += 1;
			Lambda[IndexFeature].Pi[State] += 1.0;
			Lambda[IndexFeature].B[State][Symbol] += 1.0;
		}else if(TheMCsOfMovie[IndexFeature].NbCouples >= 2){
			for(j=0;(j < TheMCsOfMovie[IndexFeature].NbCouples);j++){
				if(j != TheMCsOfMovie[IndexFeature].NbCouples -1){
					State = TheMCsOfMovie[IndexFeature].Elements[j].State - 1;
					NextState = TheMCsOfMovie[IndexFeature].Elements[j+1].State - 1;
					Symbol = TheMCsOfMovie[IndexFeature].Elements[j].Symbol - 1;

					From[State] += 1;
					UseOfState[State] += 1;
					Lambda[IndexFeature].A[State][NextState] += 1.0;
					Lambda[IndexFeature].B[State][Symbol] += 1.0;
				
					if(j == 0){
						Lambda[IndexFeature].Pi[State] += 1.0;
					}
				}else{
					State = TheMCsOfMovie[IndexFeature].Elements[j].State - 1;
					Symbol = TheMCsOfMovie[IndexFeature].Elements[j].Symbol - 1;

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

