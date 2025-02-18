#include "hmm_CF.h"

#define MaxOccupations 22
#define MaxZipCodeGroups 20

// A movie watched by a user
typedef struct WatchedMovie_CF{      
	int Movie_Id;                    	 
	float Score;  
	int Timestamp;
} WatchedMovie_CF;

// A user
typedef struct User_CF{      
	int User_Id;  
	int age;
	char gender;
	char occupation[50];
	char zipCode[6];
	int NbWatchedMovies;   	
	WatchedMovie_CF LastWatchedMovies[MaxMovies];
} User_CF;

// A user who watched a movie
typedef struct UsersWatching_CF{      
	int User_Id;                    	 
	float Score;                         
	int Timestamp;
} UsersWatching_CF;

// A movie
typedef struct Movie_CF{      
	int Movie_Id;                    	 
	char Title[100];
	int NbUsersWatching;	
	UsersWatching_CF TheUsersWatching[MaxUsers];
} Movie_CF;


// A movie recommended to a user
typedef struct MovieRecom_CF{      
	int Movie_Id; 
	int Frequency;
	double Distance;
	double Importance; 
} MovieRecom_CF;

// An observation of a Markov chain
typedef struct Couple_CF{      
	int State;                    	 
	int Symbol;                         
} Couple_CF;

// A Markov chain
typedef struct MarkovChain_CF{      
	int NbCouples;                
	Couple_CF Elements[MaxUsers];        
} MC_CF;

// Displays the content of a Markov chain
void DisplayMC_CF(MC_CF Delta){
	int i;
	
	for(i=0;(i<Delta.NbCouples);i++){
		printf("(%i,%i)\t",Delta.Elements[i].State,Delta.Elements[i].Symbol);
	}
	printf("\n");
	return;
}

// Prints the content of a Markov chain in a file
void DisplayMCfile_CF(MC_CF Delta, FILE ** f){
	int i;
	
	for(i=0;(i<Delta.NbCouples);i++){
		fprintf((*f),"(%i,%i)",Delta.Elements[i].State,Delta.Elements[i].Symbol);
	}
	fprintf((*f),"\n");
	return;
}

// Reads the movie data 
int ReadMovies_CF(char * MoviesFileName, Movie_CF TheMovies[MaxMovies]){
	FILE * f;
	int i,k,Release_date,Vote_count,Genre;
	float Vote_average;
	char c;
	
	f = fopen(MoviesFileName, "rt");
	if(!f){
		printf("%s\n",MoviesFileName);
		return -1;
	} 
	i = 0;

	while(!feof(f)){
		fscanf(f,"%i,%i,",&(TheMovies[i].Movie_Id),&Release_date);
		fscanf(f,"%g,%i,{",&Vote_average,&Vote_count);
		do{
			fscanf(f,"%i%c",&Genre,&c);
			if((c != ',')&&(c != '}')){
				printf("Movie genre format [line %i] [movie_id %i]\n",i+1,TheMovies[i].Movie_Id);
				return -1;
			}
		}while(c != '}');
		
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
int ReadUsers_CF(char * UsersFileName, User_CF TheUsers[MaxUsers]){
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

// Function of partitionnement for the QuickSort
int PartitionWatchedMovies_Timestamp_CF(WatchedMovie_CF TheWatchedMovies[MaxMovies], int low, int high) {
    float pivot = TheWatchedMovies[high].Timestamp; // pivot
    int i = (low - 1); // Index of the lowest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is lower than pivot
        if (TheWatchedMovies[j].Timestamp <= pivot) {
            i++; // Increments the index of the lowest element 
			
            swapInt(&TheWatchedMovies[i].Movie_Id, &TheWatchedMovies[j].Movie_Id);
            swapInt(&TheWatchedMovies[i].Timestamp, &TheWatchedMovies[j].Timestamp);
            swapFloat(&TheWatchedMovies[i].Score, &TheWatchedMovies[j].Score);
        }
    }
    swapInt(&TheWatchedMovies[i+1].Movie_Id, &TheWatchedMovies[high].Movie_Id);
    swapInt(&TheWatchedMovies[i+1].Timestamp, &TheWatchedMovies[high].Timestamp);
    swapFloat(&TheWatchedMovies[i+1].Score, &TheWatchedMovies[high].Score);
    return (i + 1);
}


// QuickSort of the movies watched by a user -- by Timestamp
void QuicksortWatchedMovies_Timestamp_CF(WatchedMovie_CF TheWatchedMovies[MaxMovies], int low, int high) {
	int i,pi,piMax, piMin;
    if (low < high) {
        pi = PartitionWatchedMovies_Timestamp_CF(TheWatchedMovies, low, high);
		piMax = pi;
		piMin = pi;
		i = 0;
		while(i < piMin){
			if(TheWatchedMovies[i].Timestamp == TheWatchedMovies[pi].Timestamp){
				swapInt(&TheWatchedMovies[i].Movie_Id, &TheWatchedMovies[piMin-1].Movie_Id);
				swapInt(&TheWatchedMovies[i].Timestamp, &TheWatchedMovies[piMin-1].Timestamp);
				swapFloat(&TheWatchedMovies[i].Score, &TheWatchedMovies[piMin-1].Score);
				piMin--;
				i = 0;
			}else{
				i++;
			}
		}
		QuicksortWatchedMovies_Timestamp_CF(TheWatchedMovies, low, piMin - 1);
        QuicksortWatchedMovies_Timestamp_CF(TheWatchedMovies, piMax + 1, high);
    }
}


int ReadWatchedMovies_CF(char * WatchedMoviesFileName, User_CF TheUsersTmp[MaxUsers], int * NbUsers, int TmaxWatchedMovies){
	FILE * f;
	int i,j,k,Nb_users,user_id,movie_id,timestamp,new_user;
	int NbWatchedMovies[MaxUsers], TheUsersId[MaxUsers];
	char c;
	float rating;
	WatchedMovie_CF LastWatchedMovies[MaxUsers][MaxMovies];
	
	for(i=0;(i<MaxUsers);i++){
		NbWatchedMovies[i] = 0;
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
	Nb_users = 0;
	while(!feof(f)){
		fscanf(f,"%i\t%i\t%f\t%i\n",&user_id,&movie_id,&rating,&timestamp);
		LastWatchedMovies[user_id][NbWatchedMovies[user_id]].Movie_Id = movie_id;
		LastWatchedMovies[user_id][NbWatchedMovies[user_id]].Score = rating;
		LastWatchedMovies[user_id][NbWatchedMovies[user_id]].Timestamp = timestamp;
		(NbWatchedMovies[user_id])++;
		i++;
		
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
	}
	fclose(f);
	
	for(j=1;(j<=Nb_users);j++){
		QuicksortWatchedMovies_Timestamp_CF(LastWatchedMovies[j],0,NbWatchedMovies[j]-1);
		if((TmaxWatchedMovies <= 0)||(NbWatchedMovies[j] <= TmaxWatchedMovies)){
			TheUsersTmp[j-1].User_Id = j;
			TheUsersTmp[j-1].NbWatchedMovies = NbWatchedMovies[j];
			for(k=0;(k < NbWatchedMovies[j]); k++){
				TheUsersTmp[j-1].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[j][k].Movie_Id;
				TheUsersTmp[j-1].LastWatchedMovies[k].Score = LastWatchedMovies[j][k].Score;
				TheUsersTmp[j-1].LastWatchedMovies[k].Timestamp = LastWatchedMovies[j][k].Timestamp;
			}
		}else{
			TheUsersTmp[j-1].User_Id = j;
			TheUsersTmp[j-1].NbWatchedMovies = TmaxWatchedMovies;
			for(k=0;(k < TmaxWatchedMovies); k++){
				TheUsersTmp[j-1].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[j][(NbWatchedMovies[j] - TmaxWatchedMovies) + k].Movie_Id;
				TheUsersTmp[j-1].LastWatchedMovies[k].Score = LastWatchedMovies[j][(NbWatchedMovies[j] - TmaxWatchedMovies) + k].Score;
				TheUsersTmp[j-1].LastWatchedMovies[k].Timestamp = LastWatchedMovies[j][(NbWatchedMovies[j] - TmaxWatchedMovies) + k].Timestamp;
			}
		}
	}
	
/**	
	for(j=0;(j<Nb_users);j++){
//		printf("%i",TheUsersTmp[j].User_Id);
//		printf("%i,%i,",TheUsersTmp[j].User_Id,TheUsersTmp[j].NbWatchedMovies);
		for(k=0;(k < TheUsersTmp[j].NbWatchedMovies); k++){
			printf("%i\t%i\t%.0f\t%i\n",TheUsersTmp[j].User_Id,TheUsersTmp[j].LastWatchedMovies[k].Movie_Id,TheUsersTmp[j].LastWatchedMovies[k].Score,TheUsersTmp[j].LastWatchedMovies[k].Timestamp);
//			printf("(%i,%.0f)",TheUsersTmp[j].LastWatchedMovies[k].Movie_Id,TheUsersTmp[j].LastWatchedMovies[k].Score);
//			printf("(%i,%.0f,%i)",TheUsersTmp[j].LastWatchedMovies[k].Movie_Id,TheUsersTmp[j].LastWatchedMovies[k].Score,TheUsersTmp[j].LastWatchedMovies[k].Timestamp);
		}
//		printf("\n");
	}
	
**/	
	(*NbUsers) = Nb_users;
	return i;
}

void UpdateUsers_CF(User_CF TheUsersTmp[MaxUsers], int NbUsersTmp, User_CF TheUsers[MaxUsers], int NbUsers){
	int i,j,k;
	
	for(i=0;(i < NbUsers);i++){
		j = 0;
		while((j < NbUsersTmp)&&(TheUsersTmp[j].User_Id != TheUsers[i].User_Id)){
			j++;
		}
		if(j < NbUsersTmp){
			TheUsers[i].NbWatchedMovies = TheUsersTmp[j].NbWatchedMovies;
			for(k=0;(k < TheUsersTmp[j].NbWatchedMovies); k++){
				TheUsers[i].LastWatchedMovies[k].Movie_Id = TheUsersTmp[j].LastWatchedMovies[k].Movie_Id;
				TheUsers[i].LastWatchedMovies[k].Score = TheUsersTmp[j].LastWatchedMovies[k].Score;
				TheUsers[i].LastWatchedMovies[k].Timestamp = TheUsersTmp[j].LastWatchedMovies[k].Timestamp;
			}
		}
	}
	
	/**
	printf("\n_________________________________________________________________________\n");
	for(i=0;(i < NbUsers);i++){
		printf("%i,%i,%c,%s,%s,%i,",TheUsers[i].User_Id,TheUsers[i].age,TheUsers[i].gender,TheUsers[i].occupation,TheUsers[i].zipCode,TheUsers[i].NbWatchedMovies);
		for(k=0;(k < TheUsers[i].NbWatchedMovies); k++){
			printf("(%i,%.0f,%i)",TheUsers[i].LastWatchedMovies[k].Movie_Id,TheUsers[i].LastWatchedMovies[k].Score,TheUsers[i].LastWatchedMovies[k].Timestamp);
		}
		printf("\n");
	}
	printf("\n_________________________________________________________________________\n");
	**/
	
	return;
}

// Function of partitionnement for the QuickSortMovies
int PartitionUsersWatching_Timestamp_CF(UsersWatching_CF TheUsersWatching[MaxUsers], int low, int high) {
    float pivot = TheUsersWatching[high].Timestamp; // pivot
    int i = (low - 1); // Index of the lowest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is lower than pivot
        if (TheUsersWatching[j].Timestamp <= pivot) {
            i++; // Increments the index of the lowest element 
			
            swapInt(&TheUsersWatching[i].User_Id, &TheUsersWatching[j].User_Id);
            swapInt(&TheUsersWatching[i].Timestamp, &TheUsersWatching[j].Timestamp);
            swapFloat(&TheUsersWatching[i].Score, &TheUsersWatching[j].Score);
        }
    }
    swapInt(&TheUsersWatching[i+1].User_Id, &TheUsersWatching[high].User_Id);
    swapInt(&TheUsersWatching[i+1].Timestamp, &TheUsersWatching[high].Timestamp);
    swapFloat(&TheUsersWatching[i+1].Score, &TheUsersWatching[high].Score);
    return (i + 1);
}


// QuickSort of the users who watched a movie -- by Timestamp
void QuicksortUsersWatching_Timestamp_CF(UsersWatching_CF TheUsersWatching[MaxUsers], int low, int high) {
	int i,pi,piMax, piMin;
    if (low < high) {
        pi = PartitionUsersWatching_Timestamp_CF(TheUsersWatching, low, high);
		piMax = pi;
		piMin = pi;
		i = 0;
		while(i < piMin){
			if(TheUsersWatching[i].Timestamp == TheUsersWatching[pi].Timestamp){
				swapInt(&TheUsersWatching[i].User_Id, &TheUsersWatching[piMin-1].User_Id);
				swapInt(&TheUsersWatching[i].Timestamp, &TheUsersWatching[piMin-1].Timestamp);
				swapFloat(&TheUsersWatching[i].Score, &TheUsersWatching[piMin-1].Score);
				piMin--;
				i = 0;
			}else{
				i++;
			}
		}
		QuicksortUsersWatching_Timestamp_CF(TheUsersWatching, low, piMin - 1);
        QuicksortUsersWatching_Timestamp_CF(TheUsersWatching, piMax + 1, high);
    }
}


void UpdateMovies_CF(User_CF TheUsers[MaxUsers], int NbUsers, Movie_CF TheMovies[MaxMovies], int NbMovies, int TmaxUsersWatching){
	int i,k, movie_id,NbUsersWatching[MaxMovies];
	UsersWatching_CF TheUsersWatching[MaxMovies][MaxUsers];
	
	for(i=1;(i<=NbMovies);i++){
		NbUsersWatching[i] = 0;
	}
	
	for(i=0;(i < NbUsers);i++){
		for(k=0;(k < TheUsers[i].NbWatchedMovies); k++){
			movie_id = TheUsers[i].LastWatchedMovies[k].Movie_Id;
			TheUsersWatching[movie_id][NbUsersWatching[movie_id]].User_Id = TheUsers[i].User_Id;
			TheUsersWatching[movie_id][NbUsersWatching[movie_id]].Score = TheUsers[i].LastWatchedMovies[k].Score;
			TheUsersWatching[movie_id][NbUsersWatching[movie_id]].Timestamp = TheUsers[i].LastWatchedMovies[k].Timestamp;
			NbUsersWatching[movie_id]++;
		}
	}
	
	for(i=1;(i <= NbMovies);i++){
		QuicksortUsersWatching_Timestamp_CF(TheUsersWatching[i],0,NbUsersWatching[i]-1);
		
		if((TmaxUsersWatching <= 0)||(NbUsersWatching[i] <= TmaxUsersWatching)){
			TheMovies[i-1].NbUsersWatching = NbUsersWatching[i];
			for(k=0;(k < NbUsersWatching[i]); k++){
				TheMovies[i-1].TheUsersWatching[k].User_Id = TheUsersWatching[i][k].User_Id;
				TheMovies[i-1].TheUsersWatching[k].Score = TheUsersWatching[i][k].Score;
				TheMovies[i-1].TheUsersWatching[k].Timestamp = TheUsersWatching[i][k].Timestamp;
			}
		}else{
			TheMovies[i-1].NbUsersWatching = TmaxUsersWatching;
			for(k=0;(k < NbUsersWatching[i]); k++){
				TheMovies[i-1].TheUsersWatching[k].User_Id = TheUsersWatching[i][(NbUsersWatching[i] - TmaxUsersWatching) + k].User_Id;
				TheMovies[i-1].TheUsersWatching[k].Score = TheUsersWatching[i][(NbUsersWatching[i] - TmaxUsersWatching) + k].Score;
				TheMovies[i-1].TheUsersWatching[k].Timestamp = TheUsersWatching[i][(NbUsersWatching[i] - TmaxUsersWatching) + k].Timestamp;
			}
		}
		
	}
	
/**	
	for(i=0;(i < NbMovies);i++){
		printf("%i",TheMovies[i].Movie_Id);
//		printf("%i,%i,",TheMovies[i].Movie_Id,TheMovies[i].NbUsersWatching);
		for(k=0;(k < TheMovies[i].NbUsersWatching); k++){
			printf("(%i,%.0f)",TheMovies[i].TheUsersWatching[k].User_Id,TheMovies[i].TheUsersWatching[k].Score);
//			printf("(%i,%.0f,%i)",TheMovies[i].TheUsersWatching[k].User_Id,TheMovies[i].TheUsersWatching[k].Score,TheMovies[i].TheUsersWatching[k].Timestamp);
//			printf("%i\t%i\t%.0f\t%i\n",TheMovies[i].Movie_Id,TheMovies[i].TheUsersWatching[k].User_Id,TheMovies[i].TheUsersWatching[k].Score,TheMovies[i].TheUsersWatching[k].Timestamp);
		}
//		if(TheMovies[i].NbUsersWatching == 0){
//			printf("%i\t-1\t-1\t-1\n",TheMovies[i].Movie_Id);
//		}
		printf("\n");
	}
**/
	
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
int IndexOfUser_CF(int User_Id, User_CF TheUsers[MaxUsers], int NbUsers){
	int i;
	
	for(i=0;(i < NbUsers);i++){
		if(TheUsers[i].User_Id == User_Id){
			return i;
		}
	}
	return -1;
}


// Constructs the Markov chain associated with each movie m
void ConstructMCsOfMovie_CF(Movie_CF m, MC_CF TheMCs[MaxFeatures], int SequencesOfMovie[MaxFeatures][MaxUsers], int NbFeatures, User_CF TheUsers[MaxUsers], int NbUsers, char TheOccupations[MaxOccupations][50], int FreqOfThe_Occupations[MaxOccupations], int NbOccupations, char FirstChar_ZipCodeGroup[MaxZipCodeGroups], int FreqOfThe_ZipCodeGroup[MaxZipCodeGroups], int Nb_ZipCodeGroups, FILE ** f){
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
		UserIndex = IndexOfUser_CF(m.TheUsersWatching[i].User_Id,TheUsers,NbUsers);
		if(UserIndex != -1){
			TheMCs[0].Elements[i].Symbol = SymbolOf_Age(TheUsers[UserIndex].age);
			TheMCs[1].Elements[i].Symbol = (TheUsers[UserIndex].gender == 'M') ? 1 : 2;
			TheMCs[2].Elements[i].Symbol = SymbolOf_Occupation(TheUsers[UserIndex].occupation,TheOccupations,FreqOfThe_Occupations,NbOccupations);
			TheMCs[3].Elements[i].Symbol = SymbolOf_ZipCode(TheUsers[UserIndex].zipCode,FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup,Nb_ZipCodeGroups);
			
			for(j=0;(j < NbFeatures);j++){
				SequencesOfMovie[j][i] = TheMCs[j].Elements[i].Symbol - 1;
			}
		}
	}

	fprintf(*f,"_______________________________________________________________\n");
	for(j=0;(j < NbFeatures);j++){
		fprintf(*f,"delta(m_%i,%i):\t",m.Movie_Id,j+1);
		DisplayMCfile_CF(TheMCs[j],f);
	}
	
	return;
}


// Constructs all the initial HMMs associated with each movie
int InitialHMMsOfMovie_CF(MC_CF TheMCsOfMovie[MaxFeatures], int NbFeatures, HMM Lambda[MaxFeatures]){
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


void DistancesMovieVectors_CF(double TheMovieVectors[MaxMovies][MaxFeatures*MaxSymbols], int NbMovies, int NbComponents, double Distances[2][MaxMovies][MaxMovies], FILE ** f){
	int j,l;
	
    for (j = 0; j < NbMovies; j++) {
		for (l = 0; l < NbMovies; l++) {
			if(j != l){
				Distances[0][j][l] = EuclideanDistance(TheMovieVectors[j], TheMovieVectors[l], NbComponents);
				Distances[1][j][l] = ManhattanDistance(TheMovieVectors[j], TheMovieVectors[l], NbComponents);
			}else{
				Distances[0][j][l] = 0.0;
				Distances[1][j][l] = 0.0;
			}
		}
	}

	fprintf((*f),"\n==============  Euclidean distances ===========\n\n");
    for (j = 0; j < NbMovies; j++) {
		for (l = j; l < NbMovies; l++) {
			fprintf((*f),"D(%i,%i) = %g\n",j+1,l+1,Distances[0][j][l]);
		}
	}
	
	fprintf((*f),"\n==============  Manhattan distances ===========\n\n");
    for (j = 0; j < NbMovies; j++) {
		for (l = j; l < NbMovies; l++) {
			fprintf((*f),"D(%i,%i) = %g\n",j+1,l+1,Distances[1][j][l]);
		}
	}
	
	return;
}

