#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <ctype.h>

#include "hmm.h"
#include "method_CF.h"
#include "method_CB.h"
#include "cores.h"


/**
      ---------------------   MOVIELENS-1M   ---------------------
________________________________________________________________________________________________   
   Compilation command line (200 Mb of stack memory are required) :

   gcc -Wl,--stack,209715200 -o method_hybrid.exe method_hybrid.c -lkernel32
________________________________________________________________________________________________   
   Execution command line:

   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     movieFile user_File u_dataFile NbFeatures_CB K N
					 MCsFile_CB users_SequentialFile movies_SequentialFile 
________________________________________________________________________________________________   
   For each experiment e_i, with (i = 1,2,...,5): 
      - Set (MaxIterations = 50) for the Baum-Welch algorithm.
	  - Set (K = 50) neighbours for the K-NN algorithm
      - Set (TmaxMovies = 20*i) and (TmaxUsers = 20*i)
	  - Set (N = 10,20) for the Top-N recommendation, 
________________________________________________________________________________________________   
   Examples of executions for the Top-10

   method_hybrid.exe 20 20 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 40 40 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 60 60 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 80 80 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 100 100 4 50 0 MCs_CF.txt movies.data users.data ratings 50 5 10 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
________________________________________________________________________________________________   
   Examples of executions for the Top-20

   method_hybrid.exe 20 20 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 40 40 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 60 60 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 80 80 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 100 100 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CB.txt u_users_sequential.txt u_movies_sequential.txt  
________________________________________________________________________________________________   
**/

int main(int NbArg, char ** MesArg) {
	
	/**
		Exclusive access to a UNIQUE logic core on the processor
	**/
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	DWORD selected_core;
	// Selecting of the logic core
	if(sysinfo.dwNumberOfProcessors > 4){
		selected_core = 4;
	}else{
		selected_core = 0;
	}	
	// Attaching the program to the selected logic core
	if(SelectUniqueCore(selected_core) != 0){
		printf("Impossible to select the logic core %lu\n",selected_core);
		return -1;
	}
	// Attempting an exclusive access to the selected logic core
	if(ExclusiveAccessToCore(selected_core) != 0){
		printf("Exclusive access the logic core %lu is impossible\n",selected_core);
		return -1;
	}
	
	/* General variables */
	FILE *f, *g[MaxFeatures], *h;
	int i,j,k,l,NbMovies, NbUsers,TmaxWatchedMovies, SaveHMMs;
	int MaxIterations, ** Sequences;
	char Index[5], HMM_Init_FileName[100],HMM_FileName[100],FeatureVector_FileName[100];
	char FinalVectors_FileName[100];
	double Vect[MaxFeatures][MaxSymbols], epsilon = 1e-10;
	MC * TheMCs;
	HMM Lambda_Init[MaxFeatures],Lambda[MaxFeatures];
	double Duration;
    LARGE_INTEGER freq, start, end, start_all, end_all;
	int NbUsersTmp,NbMoviesTmp,NbRatings;
	Movie * TheMovies, * TheMoviesTmp;
	User * TheUsers, * TheUsersTmp;
	
	/* CF variables */
	int Nb_ZipCodeGroups,FreqOfThe_ZipCodeGroup[MaxZipCodeGroups];
	int NbFeatures_CF, FreqOfThe_Occupations[MaxOccupations];
	int TmaxUsersWatching, NbOccupations, NbMissingMovies, MissingMovies[50];
	int NbMissingVotes, MissingVotes[200];
	char FirstChar_ZipCodeGroup[MaxZipCodeGroups];
	double ** MovieVectors;
	
	/* CB variables */
	int NbClusters, NbFeatures_CB, NbGenres, FrequencyOfGenres[MaxGenres];
	int TheGenres[MaxGenres];
	double ** UserVectors;

	/* Hybrid variables */
	MovieRecom ** TheMovieRecom;
	int * NbMovieRecom, ** TheNeighbours, BadRecoms, NbNeighbours, MaxRecoms;
	double Metrics[6],std_dev[3];
	double ** Distances_Movies, Dmax_Movies;
	FILE *w, *z, *p, *q, *r;
	
	/********************************************************************************  
	   
	                              Starting of the CF sub-system
	   
	********************************************************************************/

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start_all);
    QueryPerformanceCounter(&start);

	/**
		Starting the heap memory allocation for the CF sub-system
	**/
	TheMovies = (Movie *)malloc(MaxMovies*sizeof(Movie));
	TheMoviesTmp = (Movie *)malloc(MaxMovies*sizeof(Movie));
	for(i=0;(i<MaxMovies);i++){
		TheMovies[i].TheUsersWatching = (UsersWatching *)malloc(MaxUsers*sizeof(UsersWatching));
		TheMoviesTmp[i].TheUsersWatching = (UsersWatching *)malloc(MaxUsers*sizeof(UsersWatching));
	}

	TheUsers = (User *)malloc(MaxUsers*sizeof(User));
	TheUsersTmp = (User *)malloc(MaxUsers*sizeof(User));
	for(i=0;(i<MaxUsers);i++){
		TheUsers[i].LastWatchedMovies = (WatchedMovie *)malloc(MaxWatchedMovies*sizeof(WatchedMovie));
		TheUsersTmp[i].LastWatchedMovies = (WatchedMovie *)malloc(MaxWatchedMovies*sizeof(WatchedMovie));
	}

	TheMCs = (MC *)malloc(MaxFeatures*sizeof(MC));
	for(i=0;(i<MaxFeatures);i++){
		TheMCs[i].Elements = (Couple *)malloc(MaxLenMC*sizeof(Couple));
	}

	Sequences = (int **)malloc(MaxFeatures*sizeof(int*));
	for(i=0;(i<MaxFeatures);i++){
		Sequences[i] = (int *)malloc(MaxLenMC*sizeof(int));
	}

	MovieVectors = (double **)malloc(MaxMovies*sizeof(double*));
	for(i=0;(i<MaxMovies);i++){
		MovieVectors[i] = (double *)malloc(SizeOfMovieVector*sizeof(double));
	}
	/**
		End of the heap memory allocation for the CF sub-system
	**/	
	
	TmaxWatchedMovies = atoi(MesArg[1]);
	if((TmaxWatchedMovies > MaxWatchedMovies)||(TmaxWatchedMovies <= 10)){
		TmaxWatchedMovies = MaxWatchedMovies;
	}

	TmaxUsersWatching = atoi(MesArg[2]);
	if((TmaxUsersWatching > MaxUsersWatching)||(TmaxUsersWatching <= 10)){
		TmaxUsersWatching = MaxUsersWatching;
	}

	NbFeatures_CF = atoi(MesArg[3]);
	MaxIterations = atoi(MesArg[4]);
	SaveHMMs = atoi(MesArg[5]);
	
	/* Opening all the files for the CF method */
	
	f = fopen(MesArg[6], "wt");
	if(!f){
		printf("MCs file for the CF method\n");
		return -1;
	} 
	
	for(j=0;(j < NbFeatures_CF);j++){
		
		itoa(j+1,Index,10);
		strcpy(FeatureVector_FileName,"FeatureVectors_CF_");
		strcat(FeatureVector_FileName,Index);
		strcat(FeatureVector_FileName,".bin");
		
		g[j] = fopen(FeatureVector_FileName, "wt");
		if(!g[j]){
			printf("FeatureVectors_%i file for the CF method\n",j+1);
			return -1;
		} 
	}

	strcpy(FinalVectors_FileName,"MovieVectors.txt");
	h = fopen(FinalVectors_FileName, "wt");
	if(!h){
		printf("%s file\n",FinalVectors_FileName);
		return -1;
	} 

    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nInitialization time = %.2f s\n",Duration);
	
    QueryPerformanceCounter(&start);
		
	printf("\n======= Reading the DATASET =======\n");

	/* Insertion of the vote averages and the vote counts in the dataset */ 
	//EditMoviesFile("movies_original.data","movies.data","ratings_movies.csv");

	NbMovies = ReadMovies(MesArg[7],TheMovies);
	printf("%i Movies\n",NbMovies);
	
	NbUsers = ReadUsers(MesArg[8],TheUsers);
	printf("%i Users\n",NbUsers);
	
	
	i = ReadRatings_Users(MesArg[9],TheUsersTmp,&NbUsersTmp,TmaxWatchedMovies,MesArg[14]);
	j = ReadRatings_Movies(MesArg[9],TheMoviesTmp,&NbMoviesTmp,TmaxUsersWatching,MesArg[15]);
	if(i != j){
		return -1;
	}else{
		NbRatings = i;
	}
	printf("%i Ratings\n",NbRatings);
	
	i = NbUsers - NbUsersTmp;
	j = NbMovies - NbMoviesTmp;
	if(i != 0){
		printf("%i users watched no movie\n",i);
	}
	if(j != 0){
		printf("%i movies with no votes\n",j);
	}
	
	UpdateUsers(TheUsersTmp,TheUsers,NbUsers);
	UpdateMovies(TheMoviesTmp,TheMovies,NbMovies);
	
	NbOccupations = FrequencyOf_Occupations(TheUsers,NbUsers,FreqOfThe_Occupations);
	printf("%i Occupations\n",NbOccupations);
	
	Nb_ZipCodeGroups = FrequencyOf_ZipCodeGroups(TheUsers,NbUsers,FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup);
	printf("%i ZipCode Groups\n",Nb_ZipCodeGroups);
	
	NbGenres = FrequencyOf_Genres(TheMovies,NbMovies,FrequencyOfGenres,TheGenres);
	printf("%i Genres\n",NbGenres);

    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nData access time = %.2f s\n",Duration);
	
    QueryPerformanceCounter(&start);

	printf("\n======= Learning the SEQUENTIAL USER PROFILES =======\n");
	
	for(i=0;(i < NbMovies);i++){
		if((i != 0)&&(i % 10 == 0)){
			printf(".");
		}
		fprintf(h,"%i,[",TheMovies[i].Movie_Id);
		
		/* Construction of the Markov chains of each movie*/
		ConstructMCsOfMovie(TheMovies[i],TheMCs,Sequences,NbFeatures_CF,TheUsers,NbUsers,FreqOfThe_Occupations,NbOccupations,FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup,Nb_ZipCodeGroups,&f);
		
		/*  Construction of the initial HMMs of each movie */
		InitialHMMsOfMovie(TheMCs,NbFeatures_CF,Lambda_Init);

		l = 0;
		for(j=0;(j < NbFeatures_CF);j++){
			
			/* Training each HMM if possible */
			if(TheMCs[j].NbCouples >= 2){
				Baum_Welch_Mono(Lambda_Init[j],Sequences[j],TheMovies[i].NbUsersWatching,epsilon,MaxIterations,&(Lambda[j]));
			}else{
				CopyHMM(&(Lambda[j]),Lambda_Init[j]);
			}

			/* Computing each feature vector */
			VectorHMM(Lambda[j],Vect[j]);

			fprintf(g[j],"%i,[",TheMovies[i].Movie_Id);
			for(k=0;(k < Lambda[j].M);k++){
				/* Saving the feature vector components */
				fprintf(g[j],"%g",Vect[j][k]);
				if(k != Lambda[j].M-1){
					fprintf(g[j],",");
				}else{
					fprintf(g[j],"]\n");
				}
				
				/* Saving the final vector components */
				MovieVectors[i][l++] = Vect[j][k];
				fprintf(h,"%g",Vect[j][k]);
				if((j != NbFeatures_CF-1)||(k != Lambda[j].M-1)){
					fprintf(h,",");
				}else{
					fprintf(h,"]\n");
				}
			}
			
			
			/* Saving the HMMs */
			if(SaveHMMs == 1){
				itoa(TheMovies[i].Movie_Id,Index,10);
				strcpy(HMM_Init_FileName,"HMM_Init_movie");
				strcpy(HMM_FileName,"HMM_movie");
				strcat(HMM_Init_FileName,Index);
				strcat(HMM_FileName,Index);
				
				itoa(j+1,Index,10);
				strcat(HMM_Init_FileName,"_feature");
				strcat(HMM_FileName,"_feature");
				strcat(HMM_Init_FileName,Index);
				strcat(HMM_FileName,Index);
				
				strcat(HMM_Init_FileName,".dat");
				strcat(HMM_FileName,".dat");

				SaveHMMtxt(Lambda_Init[j],HMM_Init_FileName);
				SaveHMMtxt(Lambda[j],HMM_FileName);
			}
		}
	}	


	/* Closing all the result files for the CF method */
	fclose(f);
	for(j=0;(j < NbFeatures_CF);j++){
		fclose(g[j]);
	}	
	fclose(h);
	
	/**
		Starting the heap memory liberation for the CF sub-system
	**/

	for(i=0;(i<MaxMovies);i++){
		free(TheMoviesTmp[i].TheUsersWatching);
	}
	free(TheMoviesTmp);

	for(i=0;(i<MaxUsers);i++){
		free(TheUsersTmp[i].LastWatchedMovies);
	}
	free(TheUsersTmp);

	/**
		End of the heap memory liberation for the CF sub-system
	**/

    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nHMM training and movie vectors computation time = %.2f s\n",Duration);

	/********************************************************************************  
	   
	                              Starting of the CB sub-system
	   
	********************************************************************************/

    QueryPerformanceCounter(&start);

	/**
		Starting the heap memory allocation for the CB sub-system
	**/

	UserVectors = (double **)malloc(MaxUsers*sizeof(double*));
	for(i=0;(i<MaxUsers);i++){
		UserVectors[i] = (double *)malloc(SizeOfUserVector*sizeof(double));
	}
	
	/**
		End of the heap memory allocation for the CB sub-system
	**/
	
	NbClusters = 5;
	NbFeatures_CB = atoi(MesArg[10]);
	
	/* Opening all the files for the CB method */
	
	f = fopen(MesArg[13], "wt");
	if(!f){
		printf("MCs file for the CB method\n");
		return -1;
	} 
	
	for(j=0;(j < NbFeatures_CB);j++){
		
		itoa(j+1,Index,10);
		strcpy(FeatureVector_FileName,"FeatureVectors_CB_");
		strcat(FeatureVector_FileName,Index);
		strcat(FeatureVector_FileName,".dat");
		
		g[j] = fopen(FeatureVector_FileName, "wt");
		if(!g[j]){
			printf("FeatureVectors_%i file for the CB method\n",j+1);
			return -1;
		} 
	}
	
	strcpy(FinalVectors_FileName,"UserVectors.txt");
	h = fopen(FinalVectors_FileName, "wt");
	if(!h){
		printf("%s file\n",FinalVectors_FileName);
		return -1;
	} 

	printf("\n======= Learning the SEQUENTIAL USER PREFERENCES =======\n");

	for(i=0;(i < NbUsers);i++){
		if((i != 0)&&(i % 10 == 0)){
			printf(".");
		}
		fprintf(h,"%i,[",TheUsers[i].User_Id);
		
		ConstructMCsOfUser(TheUsers[i],TheMCs,Sequences,NbFeatures_CB,TheMovies,NbMovies,FrequencyOfGenres,TheGenres,NbClusters,&f);
		
		/* Computing the initial HMMs */
		InitialHMMsOfUser(TheMCs,NbFeatures_CB,Lambda_Init);
		
		l = 0;
		for(j=0;(j < NbFeatures_CB);j++){
			
			/* Training each HMM if possible */
			if(TheMCs[j].NbCouples >= 2){
				Baum_Welch_Mono(Lambda_Init[j],Sequences[j],TheUsers[i].NbWatchedMovies,epsilon,MaxIterations,&(Lambda[j]));
			}else{
				CopyHMM(&(Lambda[j]),Lambda_Init[j]);
			}

			/* Computing each feature vector */
			VectorHMM(Lambda[j],Vect[j]);

			fprintf(g[j],"%i,[",TheUsers[i].User_Id);
			for(k=0;(k < Lambda[j].M);k++){
				/* Saving the feature vector components */
				fprintf(g[j],"%g",Vect[j][k]);
				if(k != Lambda[j].M-1){
					fprintf(g[j],",");
				}else{
					fprintf(g[j],"]\n");
				}
				
				/* Saving the final vector components */
				UserVectors[i][l++] = Vect[j][k];
				fprintf(h,"%g",Vect[j][k]);
				if((j != NbFeatures_CB-1)||(k != Lambda[j].M-1)){
					fprintf(h,",");
				}else{
					fprintf(h,"]\n");
				}
			}

			/* Saving the HMMs */
			if(SaveHMMs == 1){
				itoa(TheUsers[i].User_Id,Index,10);
				strcpy(HMM_Init_FileName,"HMM_Init_user");
				strcpy(HMM_FileName,"HMM_user");
				strcat(HMM_Init_FileName,Index);
				strcat(HMM_FileName,Index);
				
				itoa(j+1,Index,10);
				strcat(HMM_Init_FileName,"_feature");
				strcat(HMM_FileName,"_feature");
				strcat(HMM_Init_FileName,Index);
				strcat(HMM_FileName,Index);
				
				strcat(HMM_Init_FileName,".dat");
				strcat(HMM_FileName,".dat");

				SaveHMMtxt(Lambda_Init[j],HMM_Init_FileName);
				SaveHMMtxt(Lambda[j],HMM_FileName);
			}
		}
	}

	/**
		Starting the heap memory liberation for the CB sub-system
	**/

	for(i=0;(i<MaxMovies);i++){
		free(TheMovies[i].TheUsersWatching);
	}
	free(TheMovies);

	for(i=0;(i<MaxFeatures);i++){
		free(TheMCs[i].Elements);
	}
	free(TheMCs);


	for(i=0;(i<MaxFeatures);i++){
		free(Sequences[i]);
	}
	free(Sequences);
	
	/**
		End of the heap memory liberation for the CB sub-system
	**/
	
    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nHMM training and user vectors computation time = %.2f s\n",Duration);

	/********************************************************************************  
	   
	                              Starting of the HYBRID RECOMMENDATION
	   
	********************************************************************************/
    QueryPerformanceCounter(&start);
	
	/**
		Starting the heap memory allocation for the Hybrid recommendation
	**/

	TheMovieRecom = (MovieRecom **)malloc(MaxUsers*sizeof(MovieRecom*));
	for(i=0;(i<MaxUsers);i++){
		TheMovieRecom[i] = (MovieRecom *)malloc(MaxNeighbours*TmaxWatchedMovies*sizeof(MovieRecom));
	}

	NbMovieRecom = (int *)malloc(MaxUsers*sizeof(int));

	TheNeighbours = (int **)malloc(MaxUsers*sizeof(int*));
	for(j=0;(j<MaxUsers);j++){
		TheNeighbours[j] = (int *)malloc(MaxNeighbours*sizeof(int));
	}

	Distances_Movies = (double **)malloc(MaxMovies*sizeof(double*));
	for(j=0;(j<MaxMovies);j++){
		Distances_Movies[j] = (double *)malloc(MaxMovies*sizeof(double));
	}

	/**
		End of the heap memory allocation for the Hybrid recommendation
	**/

	printf("\n======= Starting the HYBRID RECOMMENDATION process =======\n");
	
	// Creation of the result files
	w = fopen("Neighbours.txt", "wt");
	if(!w){
		printf("Neighbours file\n");
		return -1;
	} 
	
	z = fopen("Recoms_importance.txt", "wt");
	if(!z){
		printf("Recommendations with importance file\n");
		return -1;
	} 
	
	p = fopen("Simple_recoms.txt", "wt");
	if(!p){
		printf("Simple recommendations file\n");
		return -1;
	} 

	q = fopen("Bad_Recoms.txt", "wt");
	if(!q){
		printf("Bad recommendations file\n");
		return -1;
	} 

	r = fopen("Recoms_score.txt", "wt");
	if(!r){
		printf("Recommendations with score file\n");
		return -1;
	} 

	// Reading the number of neighbours for the KNN algorithm
	NbNeighbours = atoi(MesArg[11]);
	if(NbNeighbours > MaxNeighbours){
		NbNeighbours = MaxNeighbours;
	}
	
	// Reading the maximum number N of recommendations for the Top-N
	MaxRecoms = atoi(MesArg[12]); 
	if(MaxRecoms > 100){
		MaxRecoms = 100;
	}

	fprintf(w,"KNN with the Euclidean distance\n\n");
	fprintf(w,"user_id,[list of neighbours]\n\n");
	fprintf(z,"KNN with the Euclidean distance\n\n");
	fprintf(z,"user_id,[list of recommended movies with their importance] --> (list of watched movies)\n\n");
	fprintf(p,"KNN with the Euclidean distance\n\n");
	fprintf(p,"user_id,[list of recommended movies] --> (list of watched movies)\n\n");
	fprintf(q,"KNN with the Euclidean distance\n\n");
	fprintf(q,"user_id,[list of bad recommendations]\n\n");
	fprintf(r,"KNN with the Euclidean distance\n\n");
	fprintf(r,"user_id,[list of recommended movies with their predicted scores] --> [list of watched movies with their effective scores]\n\n");

	/* KNN algorithm */
	KNN(NbNeighbours,UserVectors,NbUsers,TheNeighbours);
	
	/* Saving the result of the KNN algorithm */
	for (i = 0; i < NbUsers; i++) {
		
		fprintf(w,"%i,[",TheUsers[i].User_Id);
		for(j=0;(j < NbNeighbours-1);j++){
			fprintf(w,"%i,",TheUsers[TheNeighbours[i][j]].User_Id);
		}	
		fprintf(w,"%i]\n",TheUsers[TheNeighbours[i][j]].User_Id);
	}
	
    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nKNN time = %.2f s\n",Duration);
	QueryPerformanceCounter(&start);

	/* Calculating the initial movie recommendations*/
	DistancesMovieVectors(MovieVectors,NbMovies,Distances_Movies,&Dmax_Movies);
	InitMovieRecom(TheMovieRecom,NbMovieRecom,TheNeighbours,NbNeighbours,TheUsers,NbUsers,Distances_Movies,Dmax_Movies);

	/* Sorting the movie recommendations according to their importance */
	SortMovieRecom_by_Importance_CB(TheMovieRecom,NbMovieRecom,NbUsers);

	/* Saving the result of the movie recommendations*/
	for (i = 0; i < NbUsers; i++) {
		
		fprintf(z,"%i,[",TheUsers[i].User_Id);
		fprintf(r,"%i,[",TheUsers[i].User_Id);
		fprintf(p,"%i,[",TheUsers[i].User_Id);
		for(k=0;(k < MaxRecoms-1);k++){
			fprintf(z,"(%i,%g)",TheMovieRecom[i][k].Movie_Id,TheMovieRecom[i][k].Importance);
			fprintf(r,"(%i,%g)",TheMovieRecom[i][k].Movie_Id,TheMovieRecom[i][k].PredictedScore);
			fprintf(p,"%i,",TheMovieRecom[i][k].Movie_Id);
		}
		fprintf(z,"(%i,%g)] --> (",TheMovieRecom[i][k].Movie_Id,TheMovieRecom[i][k].Importance);
		fprintf(r,"(%i,%g)] --> (",TheMovieRecom[i][k].Movie_Id,TheMovieRecom[i][k].PredictedScore);
		fprintf(p,"%i] --> (",TheMovieRecom[i][k].Movie_Id);

		for(j=0;(j < TheUsers[i].NbWatchedMovies-1);j++){
			fprintf(z,"%i,",TheUsers[i].LastWatchedMovies[j].Movie_Id);
			fprintf(r,"(%i,%.1f),",TheUsers[i].LastWatchedMovies[j].Movie_Id,TheUsers[i].LastWatchedMovies[j].Score);
			fprintf(p,"%i,",TheUsers[i].LastWatchedMovies[j].Movie_Id);
		}
		fprintf(z,"%i)\n",TheUsers[i].LastWatchedMovies[j].Movie_Id);
		fprintf(r,"(%i,%.1f)]\n",TheUsers[i].LastWatchedMovies[j].Movie_Id,TheUsers[i].LastWatchedMovies[j].Score);
		fprintf(p,"%i)\n",TheUsers[i].LastWatchedMovies[j].Movie_Id);
	}
	fprintf(w,"_______________________________________________________________\n");
	fprintf(z,"_______________________________________________________________\n");
	fprintf(r,"_______________________________________________________________\n");
	fprintf(p,"_______________________________________________________________\n");

	QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("Recommendation time = %.2f s\n",Duration);
		
	QueryPerformanceCounter(&start);
	BadRecoms = ComputeMetrics_CB(TheUsers,NbUsers,TheMovieRecom,NbMovieRecom,MaxRecoms,Distances_Movies,Dmax_Movies,Metrics,std_dev,&q);
	fprintf(q,"_______________________________________________________________\n");

	QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("Metrics computation time = %.2f s\n\n",Duration);
	
	printf("F1 = %.3f\n",Metrics[0]);
	printf("Precision = %.3f (+/-) %.2f\n",Metrics[1],std_dev[1]);
	printf("Recall = %.3f (+/-) %.2f\n",Metrics[2],std_dev[2]);
	printf("RMSE = %.3f\n",Metrics[3]);
	printf("MAE = %.3f\n",Metrics[4]);
	printf("NDCG = %.3f\n\n",Metrics[5]);
	//printf("BadRecoms = %i\n\n",BadRecoms);

	/* Closing all the result files for the CB method */
	fclose(f);
	for(j=0;(j < NbFeatures_CB);j++){
		fclose(g[j]);
	}	
	fclose(h);
	fclose(w);
	fclose(z);
	fclose(p);
	fclose(q);
	fclose(r);

	/**
		Starting the heap memory liberation for the Hybrid recommendation
	**/

	for(i=0;(i<MaxUsers);i++){
		free(TheMovieRecom[i]);
	}
	free(TheMovieRecom);

	free(NbMovieRecom);

	for(j=0;(j<MaxUsers);j++){
		free(TheNeighbours[j]);
	}
	free(TheNeighbours);

	for(j=0;(j<MaxMovies);j++){
		free(Distances_Movies[j]);
	}
	free(Distances_Movies);

	// variables of the CF sub-system
	for(i=0;(i<MaxMovies);i++){
		free(MovieVectors[i]);
	}
	free(MovieVectors);

	// variables of the CB sub-system
	for(i=0;(i<MaxUsers);i++){
		free(UserVectors[i]);
	}
	free(UserVectors);

	for(i=0;(i<MaxUsers);i++){
		free(TheUsers[i].LastWatchedMovies);
	}
	free(TheUsers);

	/**
		End of the heap memory liberation for the Hybrid recommendation
	**/
	
    QueryPerformanceCounter(&end_all);
	Duration = (double)(end_all.QuadPart - start_all.QuadPart) / freq.QuadPart;
	printf("Overall duration = %.2f s\n",Duration);
	
	return 0;
}
