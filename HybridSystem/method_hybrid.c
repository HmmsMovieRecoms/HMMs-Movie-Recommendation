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

   gcc -Wall -Wl,--stack,1536870912 -o method_hybrid.exe method_hybrid.c
________________________________________________________________________________________________   
   Execution command line:

   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     MoviesFile u_userFile u_dataFile NbFeatures_CB NbNeighbours MaxRecoms
					 MCsFile_CB u_dataSequentialFile 
________________________________________________________________________________________________   
   Experiments e1 to e6 (Top-20, 30 neighbours, 50 iterations for Baum-Welch):
   
   method_hybrid.exe 100 100 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 20 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 200 200 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 20 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 300 300 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 20 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 400 400 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 20 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 500 500 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 20 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe -1 -1 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 20 MCs_CBF.txt u_data_sequential_reduced.txt  
________________________________________________________________________________________________   
   Experiments e1 to e6 (Top-10, 30 neighbours, 50 iterations for Baum-Welch):
   
   method_hybrid.exe 100 100 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 10 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 200 200 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 10 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 300 300 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 10 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 400 400 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 10 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe 500 500 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 10 MCs_CBF.txt u_data_sequential_reduced.txt  
   method_hybrid.exe -1 -1 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 30 10 MCs_CBF.txt u_data_sequential_reduced.txt  
________________________________________________________________________________________________   
*/

int main(int NbArg, char ** MesArg) {
	
	/* Hybrid variables */
	FILE *f, *g[MaxFeatures], *h;
	int i,j,k,l,NbMovies, NbUsers,TmaxWatchedMovies, SaveHMMs;
	int MaxIterations;
	char Index[5], HMM_Init_FileName[100],HMM_FileName[100],FeatureVector_FileName[100];
	char FinalVectors_FileName[100];
	double Vect[MaxFeatures][MaxSymbols], epsilon = 1e-10;
	HMM Lambda_Init[MaxFeatures],Lambda[MaxFeatures];

	clock_t partial_t;
	double Partial_Duration, Total_Duration[2];
	
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

	/**
	strcpy(FinalVectors_FileName,"MovieVectors_");
	if(TmaxWatchedMovies > 0){
		itoa(TmaxWatchedMovies,Index,10);
	}else{
		strcpy(Index,"all");
	}
	strcat(FinalVectors_FileName,Index);
	strcat(FinalVectors_FileName,"_");
	if(TmaxUsersWatching > 0){
		itoa(TmaxUsersWatching,Index,10);
	}else{
		strcpy(Index,"all");
	}
	strcat(FinalVectors_FileName,Index);
	strcat(FinalVectors_FileName,".txt");
	**/
	
	strcpy(FinalVectors_FileName,"MovieVectors.txt");
	h = fopen(FinalVectors_FileName, "wt");
	if(!h){
		printf("%s file\n",FinalVectors_FileName);
		return -1;
	} 

	printf("\n======= Learning the SEQUENTIAL USER PROFILES =======\n");

	partial_t = clock(); 
		
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
	
	NbOccupations = FrequencyOf_Occupations(TheUsers_CF,NbUsers,TheOccupations,FreqOfThe_Occupations);
	printf("%i Occupations\n",NbOccupations);
	
	Nb_ZipCodeGroups = FrequencyOf_ZipCodeGroups(TheUsers_CF,NbUsers,FirstChar_ZipCodeGroup,FreqOfThe_ZipCodeGroup);
	printf("%i ZipCode Groups\n",Nb_ZipCodeGroups);
	
	for(i=0;(i < NbMovies);i++){
		if((i != 0)&&(i % 10 == 0)){
			printf(".");
		}
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

	partial_t = clock() - partial_t;
	Partial_Duration = ((double)partial_t)*1000.0/CLOCKS_PER_SEC;
	Total_Duration[0] = Partial_Duration;
	Total_Duration[1] = Partial_Duration;
	printf("\nVector computation time = %g ms\n",Partial_Duration);


	/* Closing all the result files for the CF method */
	fclose(f);
	for(j=0;(j < NbFeatures_CF);j++){
		fclose(g[j]);
	}	
	fclose(h);
	
	/********************************************************************************  
	   
	                              Starting of the CB sub-system
	   
	********************************************************************************/
	
	/* CB variables */
	Movie_CB TheMovies_CB[MaxMovies];
	User_CB TheUsers_CB[MaxUsers];
	MC_CB TheMCsOfUser[MaxFeatures];
	MovieRecom_CB TheMovieRecom_CB[MaxUsers][MaxNeighbours*MaxMovies];
	FILE *w, *z, *p, *q, *r;
	int NbClusters, NbFeatures_CB, NbNeighbours, MaxRecoms, NbGenres, FrequencyOfGenres[MaxGenres];
	int SequencesOfUser_CB[MaxFeatures][MaxMovies], NbMovieRecom_CB[MaxUsers], NbComponents;
	int NbSymbols_CB, Dist, TheNeighbours[2][MaxUsers][MaxNeighbours], BadRecoms, TheGenres[MaxGenres];
	double UserVectors[MaxUsers][MaxFeatures*MaxSymbols], NeighbourDistances_CB[2][MaxUsers][MaxNeighbours];
	double Metrics[2][5],std_dev[2][3];
	double Distances[2][MaxMovies][MaxMovies], Distance_Max[2];
	float PredictScore;
	
	
	NbClusters = 4;
	NbFeatures_CB = atoi(MesArg[10]);
	NbNeighbours = atoi(MesArg[11]);
	
	if(NbNeighbours > MaxNeighbours){
		NbNeighbours = MaxNeighbours;
	}
	
	MaxRecoms = atoi(MesArg[12]); 
	if(MaxRecoms > 40){
		MaxRecoms = 40;
	}
	
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

	/**
	strcpy(FinalVectors_FileName,"UserVectors_");
	if(TmaxWatchedMovies > 0){
		itoa(TmaxWatchedMovies,Index,10);
	}else{
		strcpy(Index,"all");
	}
	strcat(FinalVectors_FileName,Index);
	strcat(FinalVectors_FileName,".txt");
	**/
	
	strcpy(FinalVectors_FileName,"UserVectors.txt");
	h = fopen(FinalVectors_FileName, "wt");
	if(!h){
		printf("%s file\n",FinalVectors_FileName);
		return -1;
	} 
	
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

	printf("\n======= Learning the SEQUENTIAL USER PREFERENCES =======\n");

	partial_t = clock(); 
	
	NbMovies = ReadMovies_CB(MesArg[7],TheMovies_CB);
	printf("%i Movies\n",NbMovies);
	
	NbUsers = ReadUsers_CB(MesArg[14],TheUsers_CB,TmaxWatchedMovies);
	printf("%i Users\n",NbUsers);
	
	NbGenres = FrequencyOf_Genres(TheMovies_CB,NbMovies,FrequencyOfGenres,TheGenres);
	printf("%i Genres\n",NbGenres);
	
	NbComponents = NbSymbols_CF;
	DistancesMovieVectors_CB(MovieVectors,NbMovies,NbComponents,Distances,Distance_Max);
	

	for(i=0;(i < NbUsers);i++){
		if((i != 0)&&(i % 10 == 0)){
			printf(".");
		}
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
	
	partial_t = clock() - partial_t;
	Partial_Duration = ((double)partial_t)*1000.0/CLOCKS_PER_SEC;
	Total_Duration[0] += Partial_Duration;
	Total_Duration[1] += Partial_Duration;
	printf("\nVector computation time = %g ms\n",Partial_Duration);

	
	/********************************************************************************  
	   
	                              Starting of the HYBRID RECOMMENDATION
	   
	********************************************************************************/
	

	printf("\n======= Starting the HYBRID RECOMMENDATION process =======\n");

	/* Calculating the indices of the (NbNeighbours) nearest neighbours of each user */
	
	for(Dist = 0; Dist <= 1; Dist++){
		if(Dist == 0){
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
		}else{
			fprintf(w,"KNN with the Manhattan distance\n\n");
			fprintf(w,"user_id,[list of neighbours]\n\n");
			fprintf(z,"KNN with the Manhattan distance\n\n");
			fprintf(z,"user_id,[list of recommended movies with their importance] --> (list of watched movies)\n\n");
			fprintf(p,"KNN with the Manhattan distance\n\n");
			fprintf(p,"user_id,[list of recommended movies] --> (list of watched movies)\n\n");
			fprintf(q,"KNN with the Manhattan distance\n\n");
			fprintf(q,"user_id,[list of bad recommendations]\n\n");
			fprintf(r,"KNN with the Manhattan distance\n\n");
			fprintf(r,"user_id,[list of recommended movies with their predicted scores] --> [list of watched movies with their effective scores]\n\n");
		}

		partial_t = clock(); 

		/* KNN algorithm */
		KNN_CB(NbNeighbours,NbUsers,NbSymbols_CB,Dist,UserVectors,TheNeighbours[Dist],NeighbourDistances_CB[Dist]);

		/* Calculating the initial movie recommendations*/
		InitMovieRecom_CB(TheMovieRecom_CB,NbMovieRecom_CB,TheNeighbours[Dist],NbNeighbours,TheUsers_CB,NbUsers,Distances[Dist]);

		/* Sorting the movie recommendations by frequencies */
		SortMovieRecom_by_Importance_CB(TheMovieRecom_CB,NbMovieRecom_CB,NbUsers);

		partial_t = clock() - partial_t;
		Partial_Duration = ((double)partial_t)*1000.0/CLOCKS_PER_SEC;
		Total_Duration[Dist] += Partial_Duration;
		printf("\nRecommendation time[%i] = %g ms\n",Dist,Partial_Duration);

		/* Saving the result of the KNN algorithm */
		for (i = 0; i < NbUsers; i++) {
		
			fprintf(w,"%i,[",TheUsers_CB[i].User_Id);
			for(j=0;(j < NbNeighbours-1);j++){
				fprintf(w,"%i,",TheUsers_CB[TheNeighbours[Dist][i][j]].User_Id);
			}	
			fprintf(w,"%i]\n",TheUsers_CB[TheNeighbours[Dist][i][j]].User_Id);
		}
		
		/* Saving the result of the initial movie recommendations*/
		for (i = 0; i < NbUsers; i++) {
		
			fprintf(z,"%i,[",TheUsers_CB[i].User_Id);
			fprintf(r,"%i,[",TheUsers_CB[i].User_Id);
			fprintf(p,"%i,[",TheUsers_CB[i].User_Id);
			for(k=0;(k < MaxRecoms-1);k++){
				fprintf(z,"(%i,%g)",TheMovieRecom_CB[i][k].Movie_Id,TheMovieRecom_CB[i][k].Importance);
				PredictScore = PredictedScore(TheMovieRecom_CB[i][k].Movie_Id,i,TheUsers_CB,Distances[Dist],Distance_Max[Dist]);
				fprintf(r,"(%i,%.1f)",TheMovieRecom_CB[i][k].Movie_Id,PredictScore);
				fprintf(p,"%i,",TheMovieRecom_CB[i][k].Movie_Id);
			}
			fprintf(z,"(%i,%g)] --> (",TheMovieRecom_CB[i][k].Movie_Id,TheMovieRecom_CB[i][k].Importance);
			PredictScore = PredictedScore(TheMovieRecom_CB[i][k].Movie_Id,i,TheUsers_CB,Distances[Dist],Distance_Max[Dist]);
			fprintf(r,"(%i,%.1f)] --> (",TheMovieRecom_CB[i][k].Movie_Id,PredictScore);
			fprintf(p,"%i] --> (",TheMovieRecom_CB[i][k].Movie_Id);

			for(j=0;(j < TheUsers_CB[i].NbWatchedMovies-1);j++){
				fprintf(z,"%i,",TheUsers_CB[i].LastWatchedMovies[j].Movie_Id);
				fprintf(r,"(%i,%.1f),",TheUsers_CB[i].LastWatchedMovies[j].Movie_Id,TheUsers_CB[i].LastWatchedMovies[j].Score);
				fprintf(p,"%i,",TheUsers_CB[i].LastWatchedMovies[j].Movie_Id);
			}
			fprintf(z,"%i)\n",TheUsers_CB[i].LastWatchedMovies[j].Movie_Id);
			fprintf(r,"(%i,%.1f)]\n",TheUsers_CB[i].LastWatchedMovies[j].Movie_Id,TheUsers_CB[i].LastWatchedMovies[j].Score);
			fprintf(p,"%i)\n",TheUsers_CB[i].LastWatchedMovies[j].Movie_Id);

		}
		fprintf(w,"_______________________________________________________________\n");
		fprintf(z,"_______________________________________________________________\n");
		fprintf(r,"_______________________________________________________________\n");
		fprintf(p,"_______________________________________________________________\n");
		
		BadRecoms = ComputeMetrics_CB(TheUsers_CB,NbUsers,TheMovieRecom_CB,MaxRecoms,Distances[Dist],Distance_Max[Dist],Metrics[Dist],std_dev[Dist],&q);
		fprintf(q,"_______________________________________________________________\n");

		printf("F1[%i] = %.5f (+/-) %g\n",Dist,Metrics[Dist][0],std_dev[Dist][0]);
		printf("Precision[%i] = %.5f (+/-) %g\n",Dist,Metrics[Dist][1],std_dev[Dist][1]);
		printf("Recall[%i] = %.5f (+/-) %g\n",Dist,Metrics[Dist][2],std_dev[Dist][2]);
		printf("RMSE[%i] = %.5f\n",Dist,Metrics[Dist][3]);
		printf("MAE[%i] = %.5f\n",Dist,Metrics[Dist][4]);
		printf("BadRecoms[%i] = %i\n",Dist,BadRecoms);
		printf("Total duration[%i] = %g ms ==> (%g s)\n",Dist,Total_Duration[Dist],Total_Duration[Dist]/1000);
	}

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
	

	return 0;
}