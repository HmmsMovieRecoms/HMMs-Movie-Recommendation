
#####  Directory '100K'  #####
__________________________________________________________________________________________________

This directory contains the Experimental codes and data used in [1] 
for the dataset 'MovieLens-100K'. 

The sub-directory 'OtherFiles' contains files related to the frequencies
of some movie/user features, as well as the clusterings/samplings
rules used in the dataset 'MovieLens-100K'.
__________________________________________________________________________________________________

==How to compile and run the program to obtain the results of [1] ? 
__________________________________________________________________________________________________

1-Compilation (200 Mb of stack memory are required): 
   gcc  -Wl,--stack,209715200 -o method_hybrid.exe method_hybrid.c -lkernel32
________________________________________________________________________________________________   

2-Execution command line

   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     MoviesFile u_userFile u_dataFile NbFeatures_CB NbNeighbours MaxRecoms
					 MCsFile_CB users_SequentialFile movies_SequentialFile 
		  
________________________________________________________________________________________________   
3-Execution settings

   For each experiment e_i, with (i = 1,2,...,5): 
      - Set (MaxIterations = 50) for the Baum-Welch algorithm.
	  - Set (K = 50) neighbours for the K-NN algorithm
      - Set (TmaxMovies = 20*i) and (TmaxUsers = 20*i)
	  - Set (N = 10,20) for the Top-N recommendation, 
________________________________________________________________________________________________   
4-Examples of executions for the Top-10
   
   method_hybrid.exe 20 20 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 40 40 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 60 60 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 80 80 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 100 100 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 10 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
________________________________________________________________________________________________   
5-Examples of executions for the Top-20
   
   method_hybrid.exe 20 20 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt
   method_hybrid.exe 40 40 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 60 60 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 80 80 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
   method_hybrid.exe 100 100 4 50 0 MCs_CF.txt movies.data users.data ratings 4 50 20 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
________________________________________________________________________________________________   

6-Input files :

   - 'movies_original.data':  the original movie features in the system 
                              without the vote counts and the vote averages.
   - 'movies.data':           all the movie features in the system 
                              including the vote counts and the vote averages.
   - 'users.data':            all the user features in the system.
   - 'ratings_movies.csv':    the ratings sorted by (movie_id->timestamp->rating->user_id) 
   - 'ratings_users.csv':     the ratings sorted by (user_id->timestamp->rating->movie_id) 
__________________________________________________________________________________________________

7-Output files :

   - 'MCs_CF.txt':   the Markov chains of the movies.
   - 'MCs_CBF.txt':  the Markov chains of the users.
   - 'FeatureVectors_CBF_$i$.dat': the single feature vectors of the users for the 
                                   movie feature number $i$.
   - 'FeatureVectors_CF_$i$.bin': the single feature vectors of the movies for the 
                                   user feature number $i$.
   - 'UserVectors.txt':      the final feature vectors of the users.
   - 'MovieVectors.txt':     the final feature vectors of the movies.
   - 'u_movies_sequential.txt':  the movies represented as sequences of users.
   - 'u_users_sequential.txt':   the users represented as sequences of movies 
   - 'Neighbours.txt':        the K neighbours of the users.
   - 'Recoms_importance.txt': the list of movies recommended to the users with their importance.
   - 'Recoms_score.txt':      the list of movies recommended to the users with their predicted scores.
   - 'Simple_recoms.txt':     the list of movies recommended to the users.
   - 'Bad_Recoms.txt':        the list of bad recommendations, these are the movies 
                              that have been recommended to users but not watched
							  by these users. 
__________________________________________________________________________________________________
   
8-Data displayed on the screen:

   - Statistics from the dataset.
   - Time costs.
   - Accuracy metrics (F1, precision, recall, RMSE, MAE, NDCG).
__________________________________________________________________________________________________

[1] J. CHIASSEUP KEUDJEU, O. KENGNI NGANGMO, SYLVAIN ILOGA and T. BOUETOU BOUETOU,
    "Hybrid Movie Recommendation Based On Recent User Sequential Preferences And Sequential Profiles using HMMs",
	(Under review).
__________________________________________________________________________________________________
