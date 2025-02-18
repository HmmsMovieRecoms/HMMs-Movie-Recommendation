#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <time.h>

#include "hmm.h"
#include "method_CB.h"
#include "method_CF.h"

/*
________________________________________________________________________________________________   
   Compilation command line:

   gcc -Wall -o method_hybrid.exe method_hybrid.c
________________________________________________________________________________________________   
   Execution command line:

   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     MoviesFile u_userFile u_dataFile NbFeatures_CB 
					 MCsFile_CB u_dataSequentialFile 
________________________________________________________________________________________________   
   Example of execution: 
   
   method_hybrid.exe 7 3 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 MCs_CBF.txt u_data_sequential_reduced.txt  
________________________________________________________________________________________________   
*/

int main(int NbArg, char ** MesArg) {
	
	/* Hybrid variables */
	FILE *f, *g[MaxFeatures], *h, *w;
	int i,j,k,l,NbMovies, NbUsers,TmaxWatchedMovies, SaveHMMs;
	int MaxIterations;
	char Index[5], HMM_Init_FileName[100],HMM_FileName[100],FeatureVector_FileName[100];
	char FinalVectors_FileName[100];
	double Vect[MaxFeatures][MaxSymbols], epsilon = 1e-10;
	HMM Lambda_Init[MaxFeatures],Lambda[MaxFeatures];

	clock_t overall_t;
	double OverallDuration;
	
	/********************************************************************************  
	   
	                              Starting of the CF sub-system
	   
	********************************************************************************/
	
	/* CF variables */
	Movie_CF TheMovies_CF[MaxMovies];
	User_CF TheUsers_CF[MaxUsers], TheUsersTmp_CF[MaxUsers];
	MC_CF TheMCsOfMovie[MaxFeatures];
	int NbUsersTmp_CF,NbRatings_CF, Nb_ZipCodeGroups,FreqOfThe_ZipCodeGroup[MaxZipCodeGroups];
	int NbSymbols_CF, NbFeatures_CF, SequencesOfMovie_CF[MaxFeatures][MaxUsers], FreqOfThe_Occupations[MaxOccupations];
	int TmaxUsersWatching, NbOccupations;
	char TheOccupations[MaxOccupations][50], FirstChar_ZipCodeGroup[MaxZipCodeGroups];
	double MovieVectors[MaxMovies][MaxFeatures*MaxSymbols];
	double Distances_Movies[2][MaxMovies][MaxMovies];

	srand(time(NULL));
	
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

	printf("\n======= Learning the SEQUENTIAL USER PROFILES =======\n");

	overall_t = clock(); 
		
	NbMovies = ReadMovies_CF(MesArg[7],TheMovies_CF);
	printf("%i Movies\n",NbMovies);
	
	NbUsers = ReadUsers_CF(MesArg[8],TheUsers_CF);
	printf("%i Users\n",NbUsers);
	
	NbRatings_CF = ReadWatchedMovies_CF(MesArg[9],TheUsersTmp_CF,&NbUsersTmp_CF,TmaxWatchedMovies);
	printf("%i Ratings\n",NbRatings_CF);
	
	if(NbUsersTmp_CF != NbUsers){
		return -1;
	}
	
	UpdateUsers_CF(TheUsersTmp_CF,NbUsersTmp_CF,TheUsers_CF,NbUsers);
	
	UpdateMovies_CF(TheUsers_CF,NbUsers,TheMovies_CF,NbMovies,TmaxUsersWatching);
	
	NbOccupations = FrequencyOf_Occupations(TheOccupations,FreqOfThe_Occupations);
	printf("%i Occupations\n",NbOccupations);
	
	Nb_ZipCodeGroups = FrequencyOf_ZipCodeGroups(FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup);
	printf("%i ZipCode Groups\n",Nb_ZipCodeGroups);
	
	for(i=0;(i < NbMovies);i++){
		printf(".");
		fprintf(h,"%i,[",TheMovies_CF[i].Movie_Id);
		
		/* Construction of the Markov chains of each movie*/
		ConstructMCsOfMovie_CF(TheMovies_CF[i],TheMCsOfMovie,SequencesOfMovie_CF,NbFeatures_CF,TheUsers_CF,NbUsers,TheOccupations,FreqOfThe_Occupations,NbOccupations,FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup,Nb_ZipCodeGroups,&f);
		
		/*  Construction of the initial HMMs of each movie */
		NbSymbols_CF = InitialHMMsOfMovie_CF(TheMCsOfMovie,NbFeatures_CF,Lambda_Init);

		l = 0;
		for(j=0;(j < NbFeatures_CF);j++){
			
			/* Training each HMM if possible */
			if(TheMCsOfMovie[j].NbCouples >= 2){
				Baum_Welch_Mono_CF(Lambda_Init[j],SequencesOfMovie_CF[j],TheMovies_CF[i].NbUsersWatching,epsilon,MaxIterations,&(Lambda[j]));
			}else{
				CopyHMM(&(Lambda[j]),Lambda_Init[j]);
			}

			/* Computing each feature vector */
			VectorHMM(Lambda[j],Vect[j]);

			fprintf(g[j],"%i,[",TheMovies_CF[i].Movie_Id);
			for(k=0;(k < Lambda[j].M);k++){
				/* Saving the feature vector components */
				fprintf(g[j],"%.2f",Vect[j][k]);
				if(k != Lambda[j].M-1){
					fprintf(g[j],",");
				}else{
					fprintf(g[j],"]\n");
				}
				
				/* Saving the final vector components */
				MovieVectors[i][l++] = Vect[j][k];
				fprintf(h,"%.2f",Vect[j][k]);
				if((j != NbFeatures_CF-1)||(k != Lambda[j].M-1)){
					fprintf(h,",");
				}else{
					fprintf(h,"]\n");
				}
			}
			
			
			/* Saving the HMMs */
			if(SaveHMMs == 1){
				itoa(TheMovies_CF[i].Movie_Id,Index,10);
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
	
	DistancesMovieVectors_CF(MovieVectors,NbMovies,NbSymbols_CF,Distances_Movies,&w);
	
	overall_t = clock() - overall_t;
	OverallDuration = ((double)overall_t)*1000.0/CLOCKS_PER_SEC;
	printf("\nVector computation time = %g ms\n",OverallDuration);


	/* Closing all the result files for the CF method */
	fclose(f);
	for(j=0;(j < NbFeatures_CF);j++){
		fclose(g[j]);
	}	
	fclose(h);
	fclose(w);

	
	/********************************************************************************  
	   
	                              Starting of the CB sub-system
	   
	********************************************************************************/
	
	/* CB variables */
	Movie_CB TheMovies_CB[MaxMovies];
	User_CB TheUsers_CB[MaxUsers];
	MC_CB TheMCsOfUser[MaxFeatures];
	int NbClusters, NbFeatures_CB, NbGenres, FrequencyOfGenres[MaxGenres];
	int SequencesOfUser_CB[MaxFeatures][MaxWatchedMovies];
	int NbSymbols_CB, TheGenres[MaxGenres];
	double UserVectors[MaxUsers][MaxFeatures*MaxSymbols];
	double Distances_Users[2][MaxUsers][MaxUsers];
	
	
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
		strcpy(FeatureVector_FileName,"FeatureVectors_CBF_");
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
	
	printf("\n======= Learning the SEQUENTIAL USER PREFERENCES =======\n");

	overall_t = clock(); 
	
	NbMovies = ReadMovies_CB(MesArg[7],TheMovies_CB);
	printf("%i Movies\n",NbMovies);
	
	NbUsers = ReadUsers_CB(MesArg[12],TheUsers_CB,TmaxWatchedMovies);
	printf("%i Users\n",NbUsers);
	
	NbGenres = FrequencyOf_Genres(FrequencyOfGenres,TheGenres);
	printf("%i Genres\n",NbGenres);
	
	for(i=0;(i < NbUsers);i++){
		printf(".");
		fprintf(h,"%i,[",TheUsers_CB[i].User_Id);
		
		ConstructMCsOfUser_CB(TheUsers_CB[i],TheMCsOfUser,SequencesOfUser_CB,NbFeatures_CB,TheMovies_CB,FrequencyOfGenres,TheGenres,NbClusters,&f);
		
		/* Computing the initial HMMs */
		NbSymbols_CB = InitialHMMsOfUser_CB(TheMCsOfUser,NbFeatures_CB,Lambda_Init);
		
		l = 0;
		for(j=0;(j < NbFeatures_CB);j++){
			
			/* Training each HMM if possible */
			if(TheMCsOfUser[j].NbCouples >= 2){
				Baum_Welch_Mono_CB(Lambda_Init[j],SequencesOfUser_CB[j],TheUsers_CB[i].NbWatchedMovies,epsilon,MaxIterations,&(Lambda[j]));
			}else{
				CopyHMM(&(Lambda[j]),Lambda_Init[j]);
			}

			/* Computing each feature vector */
			VectorHMM(Lambda[j],Vect[j]);

			fprintf(g[j],"%i,[",TheUsers_CB[i].User_Id);
			for(k=0;(k < Lambda[j].M);k++){
				/* Saving the feature vector components */
				fprintf(g[j],"%.2f",Vect[j][k]);
				if(k != Lambda[j].M-1){
					fprintf(g[j],",");
				}else{
					fprintf(g[j],"]\n");
				}
				
				/* Saving the final vector components */
				UserVectors[i][l++] = Vect[j][k];
				fprintf(h,"%.2f",Vect[j][k]);
				if((j != NbFeatures_CB-1)||(k != Lambda[j].M-1)){
					fprintf(h,",");
				}else{
					fprintf(h,"]\n");
				}
			}
			
			/* Saving the HMMs */
			if(SaveHMMs == 1){
				itoa(TheUsers_CB[i].User_Id,Index,10);
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
	
	DistancesUserVectors_CB(UserVectors,NbUsers,NbSymbols_CB,Distances_Users,&w);
	
	overall_t = clock() - overall_t;
	OverallDuration = ((double)overall_t)*1000.0/CLOCKS_PER_SEC;
	printf("\nVector computation time = %g ms\n",OverallDuration);

	
	/* Closing all the result files for the CB method */
	fclose(f);
	for(j=0;(j < NbFeatures_CB);j++){
		fclose(g[j]);
	}	
	fclose(h);
	fclose(w);

		
	return 0;
}