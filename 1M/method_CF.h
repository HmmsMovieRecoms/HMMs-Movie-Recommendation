#define MaxUsers MaxLenMC                /* Max number of users */ 
#define MaxMovies 3960                   /* Max number of movies */ 
#define MaxWatchedMovies 100	         /* Max number of movies watched by a user */
#define MaxUsersWatching 100	         /* Max number of users who watched a movie */
#define MaxOccupations 22                /* Max number of occupations */
#define MaxZipCodeGroups 12              /* Max number of Zipcode groups */ 
#define MaxFeatures 4                    /* Max number of features (CF or CB) */
#define MaxGenres 25                     /* Max number of movie genres */ 

// A movie watched by a user
typedef struct WatchedMovie{      
	int Movie_Id;                    	 
	float Score;  
	int Timestamp;
} WatchedMovie;

// A user
typedef struct User{      
	int User_Id;  
	int ageRange;
	char gender;
	int occup;
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
	double Importance; 
	float PredictedScore;
} MovieRecom;

/*
	Computes the vote averages and the vote counts of each movie
*/
int Get_VoteAverages_VoteCounts(char * WatchedMoviesFileName, float * TheVoteAverages, int * TheVoteCounts){
	FILE *f;
	int i,j,user_id,movie_id,timestamp;
	char c;
	float rating;
	
	for(i=0;(i<MaxMovies);i++){
		TheVoteAverages[i] = 0.0;
		TheVoteCounts[i] = 0;
	}

	f = fopen(WatchedMoviesFileName, "rt");
	if(!f){
		printf("%s\n",WatchedMoviesFileName);
		return -1;
	} 
	
	// Skipping the first line
	do{
		fscanf(f,"%c",&c);
	}while(c != '\n');
	
	i = 0;
	while(!feof(f)){
		fscanf(f,"%i;%i;%f;%i\n",&movie_id,&user_id,&rating,&timestamp);
		TheVoteCounts[movie_id] += 1;
		TheVoteAverages[movie_id] += rating;
		i++;
	}
	fclose(f);

	for(j=1;(j<=MaxMovies);j++){
		if(TheVoteCounts[j] != 0){
			TheVoteAverages[j] /= TheVoteCounts[j];
		}
	}
	return i;
}

/*
	Edits the movie data file by adding the vote averages and the vote counts
*/
int EditMoviesFile(char * MoviesFileName, char * EditedMoviesFileName, char * RatingFileName){
	FILE *f, *g;
	int i,movie_id,release_date, * TheVoteCounts;
	float * TheVoteAverages;
	char c;
	
	f = fopen(MoviesFileName, "rt");
	if(!f){
		printf("%s\n",MoviesFileName);
		return -1;
	} 
		
	g = fopen(EditedMoviesFileName, "wt");
	if(!g){
		printf("%s\n",EditedMoviesFileName);
		return -1;
	} 
	
	TheVoteCounts =  (int *)malloc(MaxMovies*sizeof(int));
	TheVoteAverages =  (float *)malloc(MaxMovies*sizeof(float));
	
	Get_VoteAverages_VoteCounts(RatingFileName,TheVoteAverages,TheVoteCounts);

	i = 0;
	fscanf(f,"%i,%i,",&movie_id,&release_date);
	while(!feof(f)){
		fprintf(g,"%i,%i,",movie_id,release_date);
		fprintf(g,"%.2f,%i,",TheVoteAverages[movie_id],TheVoteCounts[movie_id]);
		//printf("%i;%.2f;%i\n",movie_id,TheVoteAverages[movie_id],TheVoteCounts[movie_id]);
		do{
			fscanf(f,"%c",&c);
			fprintf(g,"%c",c);
		}while(c != '\n');
		i++;
		if(!feof(f)){
			fscanf(f,"%i,%i,",&movie_id,&release_date);
		}
	}
	fclose(f);
	fclose(g);
	
	free(TheVoteCounts);
	free(TheVoteAverages);
	return i;
}


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

// Reads user data
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
		fscanf(f,"%i|%c|%i|%i|",&(TheUsers[i].User_Id),&(TheUsers[i].gender),&(TheUsers[i].ageRange),&(TheUsers[i].occup));
		if(!feof(f)){
			j = 0;
			do{
				fscanf(f,"%c",&c);
				if(c != '\n'){
					TheUsers[i].zipCode[j] = c;
					j++;
				}
			}while((c != '\n')&&(j != 5));
			TheUsers[i].zipCode[j] = 0;
			while(c != '\n'){
				fscanf(f,"%c",&c);
			}
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
				fprintf(g,"(%i,%.0f)",LastWatchedMovies[k].Movie_Id,LastWatchedMovies[k].Score);
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
				fprintf(g,"(%i,%.0f)",LastWatchedMovies[k].Movie_Id,LastWatchedMovies[k].Score);
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
				
				fprintf(g,"%i(%i,%.0f)\n",user_id,movie_id,rating);
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
				fprintf(g,"(%i,%.0f)",TheUsersWatching[k].User_Id,TheUsersWatching[k].Score);
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
				fprintf(g,"(%i,%.0f)",TheUsersWatching[k].User_Id,TheUsersWatching[k].Score);
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
				
				fprintf(g,"%i(%i,%.0f)\n",movie_id,user_id,rating);
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
int SymbolOf_Age(int AgeRange){
	int symbol;
	
	switch(AgeRange){
		case 1:{
			symbol = 1;
			break;
		}
		case 18:{
			symbol = 2;
			break;
		}
		case 25:{
			symbol = 3;
			break;
		}
		case 35:{
			symbol = 4;
			break;
		}
		case 45:{
			symbol = 5;
			break;
		}
		case 50:{
			symbol = 6;
			break;
		}
		case 56:{
			symbol = 7;
			break;
		}
		default:{
			symbol = 1;
			break;
		}
	}
	return symbol;
}

int FrequencyOf_Occupations(User * TheUsers, int NbUsers, int FreqOfThe_Occupations[MaxOccupations]){
	int i, Nb_Occupations;
	
	for(i=0;(i < MaxOccupations);i++){
		FreqOfThe_Occupations[i] = 0;
	}
	
	Nb_Occupations = 0;
	for(i=0;(i < NbUsers);i++){
		if(FreqOfThe_Occupations[TheUsers[i].occup] == 0){
			Nb_Occupations++;
		}
		FreqOfThe_Occupations[TheUsers[i].occup] += 1;
	}
	
	/**
	for(i=0;(i < Nb_Occupations);i++){
		printf("%i|%i\n",i,FreqOfThe_Occupations[i]);
	}
	**/
	
	return Nb_Occupations;
}


// Symbol corresponding to any occupation
int SymbolOf_Occupation(int Occupation, int FreqOfThe_Occupations[MaxOccupations], int NbOccupations){
	int symbol, Freq;
	
	Freq = FreqOfThe_Occupations[Occupation];

	if(Freq <= 50)symbol = 1;                          
	else if((51 <= Freq)&&(Freq <= 100))symbol = 2;      
	else if((101 <= Freq)&&(Freq <= 200))symbol = 3;      
	else if((201 <= Freq)&&(Freq <= 300))symbol = 4;      
	else if((301 <= Freq)&&(Freq <= 400))symbol = 5;      
	else if((401 <= Freq)&&(Freq <= 600))symbol = 6;      
	else if(601 <= Freq)symbol = 7;                     
	
	return symbol;
}

int FrequencyOf_ZipCodeGroups(User * TheUsers, int NbUsers, char FirstChar_ZipCodeGroup[MaxZipCodeGroups], int FreqOfThe_ZipCodeGroup[MaxZipCodeGroups]){
	int i, j, Nb_ZipCodeGroups;
	char c;
	
	Nb_ZipCodeGroups = 0;
	for(i=0;(i < NbUsers);i++){
		c = TheUsers[i].zipCode[0];
		j=0;
		while((j < Nb_ZipCodeGroups)&&(c != FirstChar_ZipCodeGroup[j])){
			j++;
		}
		if(j < Nb_ZipCodeGroups){
			FreqOfThe_ZipCodeGroup[j] += 1;
		}else{
			FirstChar_ZipCodeGroup[j] = c;
			FreqOfThe_ZipCodeGroup[j] = 1;
			Nb_ZipCodeGroups++;
		}
	}
	
	/**
	for(i=0;(i < Nb_ZipCodeGroups);i++){
		printf("%c|%i\n",FirstChar_ZipCodeGroup[i],FreqOfThe_ZipCodeGroup[i]);
	}
	**/
	
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

	if(Freq <= 400)symbol = 1;                          
	else if((401 <= Freq)&&(Freq <= 500))symbol = 2;      
	else if((501 <= Freq)&&(Freq <= 700))symbol = 3;      
	else if(701 <= Freq)symbol = 4;                     
	
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
void ConstructMCsOfMovie(Movie m, MC * TheMCs, int ** Sequences, int NbFeatures, User * TheUsers, int NbUsers, int FreqOfThe_Occupations[MaxOccupations], int NbOccupations, char FirstChar_ZipCodeGroup[MaxZipCodeGroups], int FreqOfThe_ZipCodeGroup[MaxZipCodeGroups], int Nb_ZipCodeGroups, FILE ** f){
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
			TheMCs[0].Elements[i].Symbol = SymbolOf_Age(TheUsers[UserIndex].ageRange);
			TheMCs[1].Elements[i].Symbol = (TheUsers[UserIndex].gender == 'M') ? 1 : 2;
			TheMCs[2].Elements[i].Symbol = SymbolOf_Occupation(TheUsers[UserIndex].occup,FreqOfThe_Occupations,NbOccupations);
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
				/* Age range */
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
				Lambda[IndexFeature].M = 4; 
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

