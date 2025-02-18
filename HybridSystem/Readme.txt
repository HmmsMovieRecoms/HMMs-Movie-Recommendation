
#####  Directory 'HybridSystem'  #####
__________________________________________________________________________________________________

This directory contains the Experimental codes and data used in [1] for the 'MovieLens-100K' dataset
__________________________________________________________________________________________________

==How to compile and run the program to obtain the results of [1] 

1-Compilation: 
   gcc -Wall -Wl,--stack,1536870912 -o method_hybrid.exe method_hybrid.c
   
2-Execution command line:
   method_hybrid.exe TmaxMovies TmaxUsers NbFeatures_CF MaxIterations SaveHMMs MCsFile_CF
                     MoviesFile u_userFile u_dataFile NbFeatures_CB NbNeighbours MaxRecoms
					 MCsFile_CB u_dataSequentialFile 
		  
3-Example of execution: 
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

==Results

1-Result files :

   - 'MCs_CF.txt':   the Markov chains of the movies.
   - 'MCs_CBF.txt':  the Markov chains of the users.
   - 'FeatureVectors_CBF_$i$.dat': the single feature vectors of the users for the 
                                   movie feature number $i$.
   - 'FeatureVectors_CF_$i$.bin': the single feature vectors of the movies for the 
                                   user feature number $i$.
   - 'UserVectors.txt':      the final feature vectors of the users.
   - 'MovieVectors.txt':     the final feature vectors of the movies.
   - 'Distances_Movies.txt': the Euclidean and Manhattan distances between the movies.
   - 'Distances_Users.txt':  the Euclidean and Manhattan distances between the users.
   - 'Neighbours.txt':        the K neighbours of the users.
   - 'Recoms_importance.txt': the list of movies recommended to the users with their importance.
   - 'Recoms_score.txt':      the list of movies recommended to the users with their predicted scores.
   - 'Simple_recoms.txt':     the list of movies recommended to the users.
   - 'Bad_Recoms.txt':        the list of bad recommendations, these are the movies 
                              that have been recommended to users but not watched
							  by these users. 
   
2-Data displayed on the screen:

   - Statistical data for the dataset.
   - Vector computation time cost of the CF and the CBF sub-systems.
   - Recommendation time cost for the Euclidean distance (index 0)
     and for the Manhattan distance (index 1).
   - Hybrid system metrics (F1, precision, recall, RMSE, MAE) for the 
     Euclidean distance (index 0) and for the Manhattan distance (index 1).
   - Overall time cost.
__________________________________________________________________________________________________

[1] J. CHIASSEUP KEUDJEU, O. KENGNI NGANGMO, SYLVAIN ILOGA and T. BOUETOU BOUETOU,
    "Efficient movie recommendation based on the user sequential preferences and sequential profiles using HMMs",
	(Under review).
__________________________________________________________________________________________________
