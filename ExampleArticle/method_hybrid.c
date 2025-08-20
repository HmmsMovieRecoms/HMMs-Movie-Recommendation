#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <ctype.h>

#include "hmm.h"
#include "method_CF.h"
#include "method_CB.h"


/**
________________________________________________________________________________________________   
   Compilation command line:

   gcc -o method_hybrid.exe method_hybrid.c -lkernel32
________________________________________________________________________________________________   
   Execution command line:

   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     MoviesFile u_userFile u_dataFile NbFeatures_CB 
					 MCsFile_CB users_SequentialFile movies_SequentialFile 
________________________________________________________________________________________________   
   Example of execution
   
   method_hybrid.exe 7 3 4 50 0 MCs_CF.txt movies.data users.data ratings 4 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
________________________________________________________________________________________________   
**/

int main(int NbArg, char ** MesArg) {
		
	/* General variables */
	FILE *f, *g[MaxFeatures], *h, *w;
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
	double *** Distances_Movies, *** Distances_Users;
	
	/* CF variables */
	int Nb_ZipCodeGroups,FreqOfThe_ZipCodeGroup[MaxZipCodeGroups];
	int NbSymbols_CF, NbFeatures_CF, FreqOfThe_Occupations[MaxOccupations];
	int TmaxUsersWatching, NbOccupations;
	char TheOccupations[MaxOccupations][50], FirstChar_ZipCodeGroup[MaxZipCodeGroups];
	double ** MovieVectors;
	
	/* CB variables */
	int NbClusters, NbFeatures_CB, NbGenres, FrequencyOfGenres[MaxGenres];
	int NbSymbols_CB, TheGenres[MaxGenres];
	double ** UserVectors;
	
	/********************************************************************************  
	   
	                              Starting of the CF sub-system
	   
	********************************************************************************/

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    QueryPerformanceCounter(&start_all);

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
		TheUsers[i].LastWatchedMovies = (WatchedMovie *)malloc(MaxMovies*sizeof(WatchedMovie));
		TheUsersTmp[i].LastWatchedMovies = (WatchedMovie *)malloc(MaxMovies*sizeof(WatchedMovie));
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

	Distances_Movies = (double ***)malloc(2*sizeof(double**));
	for(i=0;(i<2);i++){
		Distances_Movies[i] = (double **)malloc(MaxMovies*sizeof(double*));
		for(j=0;(j<MaxMovies);j++){
			Distances_Movies[i][j] = (double *)malloc(SizeOfMovieVector*sizeof(double));
		}
	}
	/**
		End of the heap memory allocation for the CF sub-system
	**/

	TmaxWatchedMovies = atoi(MesArg[1]);
	if(TmaxWatchedMovies > MaxWatchedMovies){
		TmaxWatchedMovies = MaxWatchedMovies;
	}

	TmaxUsersWatching = atoi(MesArg[2]);
	if(TmaxUsersWatching > MaxUsersWatching){
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

	w = fopen("Distances_Movies.txt", "wt");
	if(!w){
		printf("Distances_Movies file\n");
		return -1;
	} 

	printf("\n======= Reading the DATASET =======\n");
		
	NbMovies = ReadMovies(MesArg[7],TheMovies);
	printf("%i Movies\n",NbMovies);
	
	NbUsers = ReadUsers(MesArg[8],TheUsers);
	printf("%i Users\n",NbUsers);
	
	i = ReadRatings_Users(MesArg[9],TheUsersTmp,&NbUsersTmp,TmaxWatchedMovies,MesArg[12]);
	j = ReadRatings_Movies(MesArg[9],TheMoviesTmp,&NbMoviesTmp,TmaxUsersWatching,MesArg[13]);
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
	
	NbOccupations = FrequencyOf_Occupations(TheOccupations,FreqOfThe_Occupations);
	printf("%i Occupations\n",NbOccupations);
	
	Nb_ZipCodeGroups = FrequencyOf_ZipCodeGroups(FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup);
	printf("%i ZipCode Groups\n",Nb_ZipCodeGroups);
	
	NbGenres = FrequencyOf_Genres(FrequencyOfGenres,TheGenres);
	printf("%i Genres\n",NbGenres);

    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nData access time = %g s\n",Duration);
	
    QueryPerformanceCounter(&start);

	printf("\n======= Learning the SEQUENTIAL USER PROFILES =======\n");
	
	for(i=0;(i < NbMovies);i++){
		printf(".");
		fprintf(h,"%i,[",TheMovies[i].Movie_Id);
		
		/* Construction of the Markov chains of each movie*/
		ConstructMCsOfMovie(TheMovies[i],TheMCs,Sequences,NbFeatures_CF,TheUsers,NbUsers,TheOccupations,FreqOfThe_Occupations,NbOccupations,FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup,Nb_ZipCodeGroups,&f);
		
		/*  Construction of the initial HMMs of each movie */
		NbSymbols_CF = InitialHMMsOfMovie(TheMCs,NbFeatures_CF,Lambda_Init);

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
	
	/* Computing and saving the distance between the movie vectors */
	DistancesVectors(MovieVectors,NbMovies,Distances_Movies,SizeOfMovieVector,&w);

	/* Closing all the result files for the CF method */
	fclose(f);
	for(j=0;(j < NbFeatures_CF);j++){
		fclose(g[j]);
	}	
	fclose(h);
	fclose(w);
	
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

	for(i=0;(i<MaxMovies);i++){
		free(MovieVectors[i]);
	}
	free(MovieVectors);

	for(i=0;(i<2);i++){
		for(j=0;(j<MaxMovies);j++){
			free(Distances_Movies[i][j]);
		}
		free(Distances_Movies[i]);
	}
	free(Distances_Movies);

	/**
		End of the heap memory liberation for the CF sub-system
	**/

    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nHMM training and movie vectors computation time = %g s\n",Duration);
	
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

	Distances_Users = (double ***)malloc(2*sizeof(double**));
	for(i=0;(i<2);i++){
		Distances_Users[i] = (double **)malloc(MaxUsers*sizeof(double*));
		for(j=0;(j<MaxUsers);j++){
			Distances_Users[i][j] = (double *)malloc(SizeOfUserVector*sizeof(double));
		}
	}
	
	/**
		End of the heap memory allocation for the CB sub-system
	**/
	
	NbClusters = 4;
	NbFeatures_CB = atoi(MesArg[10]);
	
	/* Opening all the files for the CB method */
	
	f = fopen(MesArg[11], "wt");
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
	
	w = fopen("Distances_Users.txt", "wt");
	if(!w){
		printf("Distances_Users file\n");
		return -1;
	} 
	fprintf(w,"'u' = 1\n");
	fprintf(w,"'v' = 2\n");
	fprintf(w,"'w' = 3\n");
	fprintf(w,"____________________________________________________\n");

	printf("\n======= Learning the SEQUENTIAL USER PREFERENCES =======\n");

	for(i=0;(i < NbUsers);i++){
		printf(".");
		fprintf(h,"%i,[",TheUsers[i].User_Id);
		
		ConstructMCsOfUser(TheUsers[i],TheMCs,Sequences,NbFeatures_CB,TheMovies,NbMovies,FrequencyOfGenres,TheGenres,NbClusters,&f);
		
		/* Computing the initial HMMs */
		NbSymbols_CB = InitialHMMsOfUser(TheMCs,NbFeatures_CB,Lambda_Init);
		
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
	
	/* Computing and saving the distance between the user vectors */
	DistancesVectors(UserVectors,NbUsers,Distances_Users,SizeOfUserVector,&w);

	/* Closing all the result files for the CB method */
	fclose(f);
	for(j=0;(j < NbFeatures_CB);j++){
		fclose(g[j]);
	}	
	fclose(h);
	fclose(w);

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
	
	for(i=0;(i<MaxUsers);i++){
		free(UserVectors[i]);
	}
	free(UserVectors);

	for(i=0;(i<MaxUsers);i++){
		free(TheUsers[i].LastWatchedMovies);
	}
	free(TheUsers);

	for(i=0;(i<2);i++){
		for(j=0;(j<MaxUsers);j++){
			free(Distances_Users[i][j]);
		}
		free(Distances_Users[i]);
	}
	free(Distances_Users);
	
	/**
		End of the heap memory liberation for the CB sub-system
	**/
	
    QueryPerformanceCounter(&end);
	Duration = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
	printf("\nHMM training and user vectors computation time = %g s\n",Duration);
	
    QueryPerformanceCounter(&end_all);
	Duration = (double)(end_all.QuadPart - start_all.QuadPart) / freq.QuadPart;
	printf("Overall duration = %g s\n",Duration);

	return 0;
}