
#####  Directory 'ExampleArticle'  #####
__________________________________________________________________________________________________

This directory contains the codes and data of the example dataset presented in the 
Tables 2 and 7 of [1] where three users u, v and w watched the movies x1 to x7.
__________________________________________________________________________________________________

==How to compile and run the program to obtain the results of [1] ? 
__________________________________________________________________________________________________

1-Compilation: 
   gcc -Wall -o method_hybrid.exe method_hybrid.c
__________________________________________________________________________________________________
   
2-Execution command line:
   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     MoviesFile u_userFile u_dataFile NbFeatures_CB 
					 MCsFile_CB users_SequentialFile movies_SequentialFile 
__________________________________________________________________________________________________
		  
3-Example of execution: 
   method_hybrid.exe 7 3 4 50 0 MCs_CF.txt movies.data users.data ratings 4 MCs_CBF.txt u_users_sequential.txt u_movies_sequential.txt  
__________________________________________________________________________________________________

4-Input files :

   - 'movies.data':  the movie features in the system.
   - 'users.data':   the user features in the system.
   - 'ratings_movies.csv': the ratings sorted by (movie_id->timestamp->rating->user_id) 
   - 'ratings_users.csv':  the ratings sorted by (user_id->timestamp->rating->movie_id) 
__________________________________________________________________________________________________

5-Output files :

   - 'MCs_CF.txt':   the Markov chains of the movies.
   - 'MCs_CBF.txt':  the Markov chains of the users.
   - 'FeatureVectors_CBF_$i$.dat': the single feature vectors of the users for the 
                                   movie feature number $i$.
   - 'FeatureVectors_CF_$i$.bin': the single feature vectors of the movies for the 
                                   user feature number $i$.
   - 'UserVectors.txt':  the final feature vectors of the users.
   - 'MovieVectors.txt': the final feature vectors of the movies.
   - 'Distances_Movies.txt': the Euclidean and Manhattan distances between the movies.
   - 'Distances_Users.txt':  the Euclidean and Manhattan distances between the users.
   - 'u_movies_sequential.txt':  the movies represented as sequences of users.
   - 'u_users_sequential.txt':   the users represented as sequences of movies 
__________________________________________________________________________________________________

6-Data displayed on the screen:

   - Statistics from the dataset.
   - Time costs.
__________________________________________________________________________________________________

[1] J. CHIASSEUP KEUDJEU, O. KENGNI NGANGMO, SYLVAIN ILOGA and T. BOUETOU BOUETOU,
    "Hybrid Movie Recommendation Based On Recent User Sequential Preferences And Sequential Profiles using HMMs",
	(Under review).
__________________________________________________________________________________________________
