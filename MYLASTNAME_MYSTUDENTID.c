    #include <pthread.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>

    #define MAX_LINE 1024
    int numbers[16000];
    char temp[MAX_LINE];
    int count = -1;
    int offset = 0;
    double ans_ga[16],ans_aa[16],ans_sum[16];
    int out_fd;

    double ga(int *nums){
        double ret = 1;
        for (int i=0;i<1000;i++){
            ret *=pow(nums[i],1.0/1000);
        }
        return ret;
    }

    double sum(int *nums){
        double ret = 0;
        for (int i=0;i<1000;i++){
            ret +=nums[i];
        }
        return ret;
    }

    double aa(int *nums){
        double ret = sum(nums);
        return ret / 1000;
    }

    void child_thread(int *n){
        pthread_t id_ga,id_aa,id_sum;
        int i = 0;
        int status;
        int nums[1000];
        double ret_ga,ret_aa,ret_sum;
        memcpy(nums,numbers+((*n)*1000),sizeof(int)*1000);
        status = pthread_create(&id_ga, NULL, (void *)ga, nums);
        if (status != 0){
            printf("create ga pthread error\r\n",*(n+i));
            exit(1);
        }
        status = pthread_create(&id_aa, NULL, (void *)aa, nums);
        if (status != 0){
            printf("create aa pthread error\r\n",*(n+i));
            exit(1);
        }
        status = pthread_create(&id_sum, NULL, (void *)sum, nums);
        if (status != 0){
            printf("create sum pthread error\r\n",*(n+i));
            exit(1);
        }
        pthread_join(id_ga,(void **)&ret_ga);
        pthread_join(id_aa,(void **)&ret_aa);
        pthread_join(id_sum,(void **)&ret_sum);
        ans_ga[*n] = ret_ga;
        ans_aa[*n] = ret_aa;
        ans_sum[*n] = ret_sum;
        count = sprintf(temp,"Worker Child Pthread Number = %d : \t Geometric Average = %f \t Arithmetic Average = %f \t Sum = %f \n",*n,ret_ga,ret_aa,ret_sum);
        offset += pwrite(out_fd,temp,sizeof(char)*count,offset);
    }

    double max(double *nums_double){
        double ret = nums_double[0];
        for(int i =1;i<16;i++){
            if (ret<nums_double[i])
                ret = nums_double[i];
        }
        return ret;
    }

    int main(int argc, char **argv){
        if (argc < 2) {
            printf("arg error. press ./***.exe Prj2InpSect26.txt [output.txt]\n");
            exit(-1);
        }
        
        char *file_name = "output.txt";
        if (argc >=3) file_name = argv[2];
        
        FILE *fd = fopen(argv[1],"r");
        if (fd == NULL){
            printf("open file error\n");
            exit(1);
        }
        char ch[MAX_LINE];
        int i = 0;
        while (1){
            fgets(ch,MAX_LINE,fd);
            if (feof(fd)) break;
            numbers[i++] = atoi(ch);

        }
        fclose(fd);
        umask(0);
        out_fd = creat(file_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (out_fd == -1){
            printf("create file error");
            exit(1);
        }

        pthread_t ids[16];
        int status;
        int n[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        for (int i=0;i<16;i++){
            status = pthread_create(&ids[i], NULL, (void *)child_thread, n+i);
            if (status != 0){
                printf("create pthread %d error\r\n",*(n+i));
                exit(1);
            }
        }
        for (int i=0;i<16;i++){
            pthread_join(ids[i], NULL);
        }

        
        count = sprintf(temp,"Main program thread: Max of the Geometric Averages = %f \n",max(ans_ga));
        offset += pwrite(out_fd,temp,sizeof(char)*count,offset);

        count = sprintf(temp,"Main program thread: Max of the Arithmetic Averages = %f \n",max(ans_aa));
        offset += pwrite(out_fd,temp,sizeof(char)*count,offset);

        count = sprintf(temp,"Main program thread: Max of the Sums = %f \n",max(ans_sum));
        offset += pwrite(out_fd,temp,sizeof(char)*count,offset);

        count = sprintf(temp,"Main program thread: Terminating. \n");
        offset += pwrite(out_fd,temp,sizeof(char)*count,offset);

        close(out_fd);
        return 0;
    }

