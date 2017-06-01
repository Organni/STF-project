#ifndef COUR_DETAIL
#define COUR_DETAIL 

#include <stdlib.h>
#include <string.h>
#include "webOps.h"

struct course_notice{
	int notice_id;
	char title[100];		
	char publisher[20];	
	char time[20];
	char status[20];
	char content[5000];
};

struct course_file{
	int file_id;		// 非必要
	char title[100];
	char intro[255];
	char file_size[20];
	char upload_time[20];
	char status[50];
	char file_path[255];	// 查看文件标题对应的超链接可以得到
};

struct file_list
{
	char name[20];
	int file_num;
	struct course_file files[100];
};

struct homework{
	int id;						//超链接里的第一个id， 不是rec_id
	char title[100];
	char start_time[20];
	char end_time[20];
	char status[50];
	char handin_size[20];		// 提交的作业大小
	char intro[1000];			// 作业说明
	char appendix_name[100];	// 作业附件名称
	char appendix_path[100]; 	//  作业附件的file_path，可以从超链接得到
	char handin_content[1000];
	char handin_name[100];		// 提交的作业名称和file_path
	char handin_path[100];
};

int get_notice_page(int course_id, char* notice_page);

int get_notice_detail_page(int course_id, int notice_id, char *detail_page);

int extract_notice_list(char* notice_list_page, int course_id,struct course_notice *notice_list, int* notice_num);

int download_course_file(int course_id, int file_id, char* file_path, char* save_path);

int get_homework_page(int course_id, char* page_buff);

void string_trim(char* strIn,char* strOut);

void html_trim(char* strIn,char* strOut);

int get_file_page(int course_id, char*page_buff);

int extract_file_lists(char* raw_html, struct file_list *f_list, int* list_num);

int get_homework_page(int course_id, char* page_buff);

int get_homework_detail_page(int course_id, int work_id,char* page_buff);

int extract_homework_list(char* raw_html, struct  homework *work_list, int* list_num);

#endif