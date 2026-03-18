//2886315

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>
#define BILLION  1000000000L;
//int floatcompare(const void *p1, const void *p2){
//	float i=*((float *)p1);
//	float j=*((float *)p2);
//	if(i>j)
//		return (1);
//	if(i<j)
//		return (-1);
//	return (0);
//
//}

int partition(int p, int r, float *data){
	float x=data[p];
	int k=p;
	int l=r+1;
	float t;
	while(1){
		do
			k++;
		while((data[k] <= x) && (k<r));
		do
			l--;
		while(data[l] > x);
		while(k<l){
			t=data[k];
			data[k]=data[l];
			data[l]=t;
			do
				k++;
			while(data[k]<=x);
			do
				l--;
			while(data[l]>x);
		}
		t=data[p];
		data[p]=data[l];
		data[l]=t;
		return l;
	}
}

void seq_qsort(int p, int r, float *data){
	if(p < r){
		int q=partition(p, r, data);
		seq_qsort(p, q-1, data);
		seq_qsort(q+1, r, data);
	}
}

void par_qsort_sec(int p, int r, float *data, int low_limit){
    if (p >= r) return;

	int q = partition(p, r, data);
	if (r - p < low_limit) {
		par_qsort_sec(p, q-1, data, low_limit);
		par_qsort_sec(q+1, r, data, low_limit);
		return;
	}

	#pragma omp parallel
	#pragma omp sections nowait
	{
		#pragma omp section
		par_qsort_sec(p, q-1, data, low_limit);
		#pragma omp section
		par_qsort_sec(q+1, r, data, low_limit);
	}
}

void par_qsort_task(int p, int r, float *data, int low_limit){
    if (p < r) {
        int q = partition(p, r, data);

        if (r - p > low_limit) {
            #pragma omp task shared(data)
            par_qsort_task(p, q-1, data, low_limit);

            #pragma omp task shared(data)
            par_qsort_task(q+1, r, data, low_limit);

        } else {
            seq_qsort(p, q-1, data);
		    seq_qsort(q+1, r, data);
        }
    }
}

void validate_sort(int n, float *data){
	int i, flag = 1;
	for(i=0;i<n-1;i++){
		if(data[i] > data[i+1]){
            flag = 0;
		}
	}
    if(flag)
        printf("Validate passed.\n");
    else
        printf("Validate failed. \n");    
}

int main(int argc, char *argv[]){
	int i, n, low_limit;
	float *data;
	//clock_t start, end;
//	struct timespec start, end;
    double start, s_time=0.0, p_time_1=0.0, p_time_2=0.0;
	if(argc != 3){
		printf("a.out num_elems low_limit\n");
		return 1;
	}
	n=atoi(argv[1]);
	low_limit=atoi(argv[2]);
	/*generate the array*/
	data=(float*)malloc(sizeof(float)*n);
    srand(2024);
	for(i=0; i<n; i++){
		data[i]=1.1*rand()*5000/RAND_MAX;
	}

	start=omp_get_wtime();
	seq_qsort(0,n-1,&data[0]);
	s_time=omp_get_wtime() - start;

	for(i=0; i<n; i++){
		data[i]=1.1*rand()*5000/RAND_MAX;
	}

    printf("Sections/section parallel sorting: ");
	omp_set_nested(1);
    start=omp_get_wtime();
	
    par_qsort_sec(0, n-1, &data[0], low_limit);
    
	p_time_1=omp_get_wtime() - start;
    omp_set_nested(0);
	validate_sort(n, &data[0]);
    
    for(i=0; i<n; i++){
		data[i]=1.1*rand()*5000/RAND_MAX;
	}

    printf("Task parallel sorting: ");
    start=omp_get_wtime();
    
	#pragma omp parallel
	{
		#pragma omp single
		{
			par_qsort_task(0, n-1, &data[0], low_limit);
		}
	}
	
	p_time_2=omp_get_wtime() - start;
	validate_sort(n, &data[0]);

	printf("Sequential time: %lf s\n", s_time);
	printf("SECTIONS/SECTION Parallel time: %lf s\n", p_time_1);
    printf("TASK Parallel time: %lf s\n", p_time_2);
	printf("SECTIONS/SECTION Parallel speedup: %f s\n", s_time/p_time_1);
    printf("TASK Parallel speedup: %f s\n", s_time/p_time_2);
	printf("Done\n");
	free(data);
	return 0;
}