/*
 ============================================================================
 Name        : getfilename.c
 Author      : zhangly
 Version     :
 Copyright   : JSD company limited! copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

//h264 file save module
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> //for sleep


char savepath[] = "/mnt/hd1/chan/";
char timestr[128] = { 0 };
char oldtimestr[128] = { 0 };
char avi_file_name[128] = { 0 };
char mp3_file_name[128] = { 0 };
char avi_name[128] = { 0 };
char name[128] = { 0 };

void get_time_str264(char * str) {
	struct tm *newtime;
	time_t lt1;
	time(&lt1);
	newtime = localtime(&lt1);
	strftime(str, 128, "%F-%H-%M-%S", newtime);

}
FILE *p264File;
FILE *mp3File;
void* H264FileInit(unsigned char channel) {

	struct stat st = { 0 };

	if (stat("/mnt/hd1/chan", &st) == -1) {
		mkdir("/mnt/hd1/chan", 0700);
	}
	get_time_str264(avi_file_name);
	strcpy(mp3_file_name, avi_file_name);
	strcat(avi_file_name, ".h264");

	strcat(mp3_file_name, ".mp3");
	sprintf(name, "chn%d_", channel); //chn0
	strcat(avi_name, savepath); ///mnt/hd1/chan/
	strcat(avi_name, name); ///mnt/hd1/chan/chn0

	strcat(avi_name, avi_file_name);
	strcat(avi_name, mp3_file_name);
	printf("avi %s  \n", avi_name);
	printf("mp3 %s \n", mp3_file_name);

	//create main stream file
	//p264File = fopen(avi_name, "wb");
	if (!p264File) {
		printf("open file[%s] failed!\n", avi_name);
		return NULL;
	}
	return 0;
//
}
//
void get_time_str(char * str) {
	struct tm *newtime;
	time_t lt1;
	time(&lt1);
	newtime = localtime(&lt1);
	strftime(str, 128, "%F-%H-%M-%S", newtime);
}

//char tmp[];
//sprintf(tmp,"/bin/mount -t vfat %s /mnt/usb",dev);
//system(tmp);
//∆‰÷–dev «/dev/sda1°£
//./ffmpeg -i chn1_1970-01-01-09-19-52.h264 -i test.mp3 -vcodec copy -acodec copy output2.avi
void fileinit() {
	/* strcat example */


//	  Output:
//
//	  these strings are concatenated
//	  /mnt/hd1/chan/chn0_2014-07-09-15-47-57.264
//	  /mnt/hd1/chan/chn0_2014-07-09-15-47-57.mp3
}

void setchnfilename(char * str){
	struct tm *newtime;
	time_t lt1;
	time(&lt1);
	newtime = localtime(&lt1);
	strftime(str, 128, "%F-%H-%M-%S", newtime);
}
void getchnfilename(char * str,unsigned char channel,char *ntimestr,char * fileextension)
{
	char tmpstr[128]={0};
	 strcpy (tmpstr,savepath);
	 sprintf(name, "chn%d_", channel);//chn0
	 strcat (tmpstr,name);
	 strcat(tmpstr,ntimestr);
	 strcat(tmpstr, fileextension);
	 strcpy(str,tmpstr);
}

int main() {
	int i = 0;
	char str[80];
	char str264[80];
	char strmp3[80];
	char stravi[80];
	char tmpcommand[256];
	int oneshot=0;
	//getchnfilename(str, tm,timestr,".264");

	int tm = 0;
	setchnfilename(timestr);
	for(tm=0;tm<4;tm++)
	{
		getchnfilename(str, tm,timestr,".264");
		puts(str);
	}
	strcpy(oldtimestr,timestr);
	sleep(5);

	for (i = 0; i < 100; i++) {
		tm = (i % 4);
		printf("%d \n", tm);
		getchnfilename(stravi, tm,oldtimestr,".avi");
		switch (tm) {
		case 0:

			setchnfilename(timestr);
			getchnfilename(str, tm,timestr,".264");
			puts(str);
			getchnfilename(str, tm,timestr,".mp3");
			puts(str);

			getchnfilename(str264, tm,oldtimestr,".264");
			getchnfilename(strmp3, tm,oldtimestr,".mp3");



			//./ffmpeg -i chn1_1970-01-01-09-19-52.h264 -i test.mp3 -vcodec copy -acodec copy output2.avi
			tmpcommand[0] = '\0';
			sprintf(tmpcommand,"/lib/ffmpeg -i %s -i  %s -vcodec copy -acodec %s",str264,strmp3,stravi);
			puts(tmpcommand);

			break;
		case 1:
		case 2:
		case 3:
			getchnfilename(str, tm,timestr,".264");
			getchnfilename(str264, tm,oldtimestr,".264");
			tmpcommand[0] = '\0';
			sprintf(tmpcommand,"/lib/ffmpeg -i %s -i  %s -vcodec copy -acodec %s",str264,strmp3,stravi);
			puts(tmpcommand);
			puts(str);
			if(tm==3)
				strcpy(oldtimestr,timestr);
			break;
		default:
			break;
		}
		sleep(2);

	}
	return 0;

}


