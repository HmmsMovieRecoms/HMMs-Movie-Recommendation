
#####  Directory 'ExampleArticle'  #####
__________________________________________________________________________________________________

This directory contains the codes and data of the example dataset presented in the 
Tables 2 and 7 of [1] where three users u, v and w watched the movies x1 to x7.
__________________________________________________________________________________________________

==How to compile and run the program to obtain the results of [1] 

1-Compilation: 
   gcc -Wall -o method_hybrid.exe method_hybrid.c
   
2-Execution command line:
   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     MoviesFile u_userFile u_dataFile NbFeatures_CB 
					 MCsFile_CB u_dataSequentialFile 
		  
3-Example of execution: 
   method_hybrid.exe 7 3 4 50 0 MCs_CF.txt movies.txt u_user.txt u_data.txt 4 MCs_CB.txt u_data_sequential_reduced.txt  
__________________________________________________________________________________________________

==Results

1-Result files :

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
   
2-Data displayed on the screen:

   - Statistical data for the dataset.
   - Vector computation time cost of the CF and the CBF sub-systems.
__________________________________________________________________________________________________

[1] J. CHIASSEUP KEUDJEU, O. KENGNI NGANGMO, SYLVAIN ILOGA and T. BOUETOU BOUETOU,
    "Efficient movie recommendation based on the user sequential preferences and sequential profiles using HMMs",
	(Under review).
__________________________________________________________________________________________________
