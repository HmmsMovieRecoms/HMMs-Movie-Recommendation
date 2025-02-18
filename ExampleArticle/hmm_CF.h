// Forward algorithm
void Forward_CF(HMM Lambda, int O[MaxUsers], int t_bar, long double Alpha[MaxUsers][MaxStates]){
	int i, j, t;
	long double som_alpha;


	for(j = 0; j < Lambda.N; j++)
	{
		Alpha[0][j] = Lambda.B[j][O[0]] * Lambda.Pi[j];			
	}

	for(t = 0; t < t_bar - 1; t++)
	{

		for(j = 0; j < Lambda.N; j++)
		{

			som_alpha = 0.0;

			for(i = 0; i < Lambda.N; i++)
			{
				som_alpha += Alpha[t][i] * Lambda.A[i][j];
			}
			
			Alpha[t+1][j] = som_alpha * Lambda.B[j][O[t+1]];		
		}

	}

}


// Backward algorithm
void Backward_CF(HMM Lambda, int O[MaxUsers], int t_bar, long double Beta[MaxUsers][MaxStates], int T){
	int i, j, t;
	long double som_beta;



	for(i = 0; i < Lambda.N; i++)
	{
		Beta[T-1][i] = 1;				
	}

	for(t = T - 1; t >= t_bar; t--)
	{
		for(i = 0; i < Lambda.N; i++)
		{
			som_beta = 0.0;
			for(j = 0; j < Lambda.N; j++)
			{
				som_beta += Lambda.A[i][j] * Lambda.B[j][O[t]] * Beta[t][j];
			}
			
			Beta[t-1][i] = som_beta;			

		}
	}
}


// Forward_Backward algorithm
double Forward_Backward_CF(HMM Lambda, int O[MaxUsers], int t_bar, int T){
    int j;
    double Proba_Observ = 0.0;
	long double Alpha[MaxUsers][MaxStates], Beta[MaxUsers][MaxStates];
	
	Forward_CF(Lambda,O,t_bar,Alpha);
	Backward_CF(Lambda,O,t_bar,Beta,T);
    for(j = 0; j < Lambda.N; j++)
	{

		Proba_Observ += (double)(Alpha[t_bar][j] * Beta[t_bar][j]);
	}

	return Proba_Observ;
}


// Computes Xi
void CalculXi_CF(HMM Lambda, int O[MaxUsers], int T, long double Alpha[MaxUsers][MaxStates], long double Beta[MaxUsers][MaxStates], double Proba_Observ, long double Xi[MaxUsers][MaxStates][MaxStates]){
	int i, j, t;

	for(t = 0; t < T - 1; t++)
	{
		for(i = 0; i < Lambda.N; i++)
		{
			for(j = 0; j < Lambda.N; j++)
			{
				Xi[t][i][j] = Alpha[t][i] * Lambda.A[i][j] * Lambda.B[j][O[t + 1]] * Beta[t + 1][j];
				/* Pour eviter une division par zero */
				Xi[t][i][j] /= (Proba_Observ + MinDouble);
			}
		}
	}
}


// Computes Gamma
void CalculGamma_CF(HMM Lambda, int O[MaxUsers], int T, long double Alpha[MaxUsers][MaxStates], long double Beta[MaxUsers][MaxStates], double Proba_Observ, long double Gamma[MaxUsers][MaxStates]){
	int j, t;

	for(t = 0; t < T; t++)
	{
		for(j = 0; j < Lambda.N; j++)
		{
			/* Rabiner Eq. 27 */	
			Gamma[t][j] = Alpha[t][j] * Beta[t][j];
			/* Pour eviter une division par zero */
			Gamma[t][j] /= (Proba_Observ + MinDouble);
		}
	}
}

// Baum_Welch algorithm
int Baum_Welch_Mono_CF(HMM Lambda_Init, int O[MaxUsers], int T, double epsilon, int MaxIterations, HMM * Lambda){
    HMM Lambda_Old, Lambda_New;
   int i, j, t, t_bar,l, iterations;
	double Proba_Old, Proba_New;
	long double Alpha[MaxUsers][MaxStates],  Beta[MaxUsers][MaxStates], Alpha_bar[MaxUsers][MaxStates],  Beta_bar[MaxUsers][MaxStates], Gamma[MaxUsers][MaxStates],Xi[MaxUsers][MaxStates][MaxStates];
	long double som_xi, som_gamma, s;
	
	CopyHMM(&Lambda_Old,Lambda_Init); 
	
	Forward_CF(Lambda_Old,O,T,Alpha);
	Backward_CF(Lambda_Old,O,1,Beta,T);
	
	t_bar = rand()%(T-1);
		
	Proba_Old = 0.0;
	for(j = 0; j < Lambda_Old.N; j++)
	{
		Proba_Old += (double)(Alpha[t_bar][j] * Beta[t_bar][j]);
	}
	Proba_New = Proba_Old;	
			
	iterations = 0;					
	do
	{
		iterations++;

		Proba_Old = Proba_New;	

		CalculGamma_CF(Lambda_Old, O, T, Alpha, Beta, Proba_Old, Gamma);	
		CalculXi_CF(Lambda_Old, O, T, Alpha, Beta, Proba_Old, Xi);						

		Lambda_New.N = Lambda_Old.N;
		Lambda_New.M = Lambda_Old.M;
		
        for(i = 0; i < Lambda_Old.N; i++)			
		{
			Lambda_New.Pi[i] = (double)Gamma[0][i];
		}

		for(i = 0; i < Lambda_Old.N; i++)			
		{
			for(j = 0; j < Lambda_Old.N; j++)
			{
				som_xi = 0.0;
				som_gamma = 0.0;

				for(t = 0; t < T - 1; t++)
				{
					som_xi += Xi[t][i][j];
					som_gamma += Gamma[t][i];
				}
				Lambda_New.A[i][j] = (double)(som_xi / (som_gamma + MinDouble));
			}

		}
	
		for(j = 0; j < Lambda_Old.N; j++)				
		{

			for(l = 0; l < Lambda_Old.M; l++)
			{
				s = 0.0;
				som_gamma = 0.0;
				
				for(t = 0; t < T; t++)
				{
					if(O[t] == l)
					{
						s += Gamma[t][j];
					}
				}


				for(t = 0; t < T; t++)
				{
					som_gamma += Gamma[t][j];
				}
				Lambda_New.B[j][l] = (double)(s / (som_gamma + MinDouble));
			}
		}

		Forward_CF(Lambda_New,O,T,Alpha_bar);
		Backward_CF(Lambda_New,O,1,Beta_bar,T);
		
		Proba_New = 0.0;
		for(j = 0; j < Lambda_New.N; j++)
		{
			Proba_New += (double)(Alpha_bar[t_bar][j] * Beta_bar[t_bar][j]);
		}

		if((Proba_New - Proba_Old) > epsilon)
		{
			CopyHMM(&Lambda_Old,Lambda_New);
			for(t = 0; t < T; t++)
			{
				for(i = 0; i < Lambda_New.N; i++)
				{
					Alpha[t][i] = Alpha_bar[t][i]; 
					Beta[t][i] = Beta_bar[t][i];
				}
			}
		}
	}while(((Proba_New - Proba_Old) > epsilon) && iterations <= MaxIterations);

	AdjustHMM(&Lambda_New); 
	CleanHMM(&Lambda_New); 

	StationaryDistribution(&Lambda_New);

	CopyHMM(Lambda,Lambda_New);
	
	return iterations;

}
