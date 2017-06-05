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
	int download_flag;
	char save_path[255];
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
	char appendix_path[500]; 	//  作业附件的file_path，可以从超链接得到
	int appendix_download_flag;
	char appendix_save_path[255];
	char handin_content[1000];
	char handin_name[100];		// 提交的作业名称和file_path
	char handin_path[500];
	int handin_download_flag;
	char handin_save_path[255];
};

/*	获取课程公告页面
	course_id : 课程标识符
	notice_page : 存放页面源码的字符串
	返回 ; 成功时0，否则为错误码
*/
int get_notice_page(int course_id, char* notice_page);

/*	获取课程公告详情页面
	course_id : 课程标识符
	notice_id : 公告标识符
	notice_page : 存放页面源码的字符串
	返回 ; 成功时0，否则为错误码
*/
int get_notice_detail_page(int course_id, int notice_id, char *detail_page);

/*
    将网页中的通知列表提取出来
    <ram_html>  字符串形式的网页源代码，为点击课程公告后的网络学堂首页中iframe内的内容
     <course_id>  	课程序号， 你会用到它来获取课程公告的正文
     <notice_list>      课程公告列表，其结构参考头文件
      <inotice_num>   课程数量
      返回值 ： 成功时为0, 否则为其他的数值
*/
int extract_notice_list(char* notice_list_page, int course_id,struct course_notice *notice_list, int* notice_num);

/*	下载制定的文件
	course_id : 课程标识符
	file_id : 文件标识符	（无用）
	file_path ; 储存远端文件路径的字符串
	save_path: 下载到的文件的存放路径
	返回 ; 成功时0，否则为错误码
*/
int download_course_file(int course_id, int file_id, char* file_path, char* save_path);

/*	获取课程作业页面
	course_id : 课程标识符
	notice_page : 存放页面源码的字符串
	返回 ; 成功时0，否则为错误码
*/
int get_homework_page(int course_id, char* page_buff);

void string_trim(char* strIn,char* strOut);

void html_trim(char* strIn,char* strOut);

/*	获取课程文件页面
	course_id : 课程标识符
	notice_id : 公告标识符
	notice_page : 存放页面源码的字符串
	返回 ; 成功时0，否则为错误码
*/
int get_file_page(int course_id, char*page_buff);

/*
    将网页中的文件列表提取出来
    <ram_html>  字符串形式的网页源代码，为点击课程文件后的网络学堂首页中iframe内的内容
     <f_list>       文件列表数组，每一个元素对应一个标签下的文件列表，具体结构请查询头文件
      <list_num>   文件列表数量
      返回值 ： 成功时为0, 否则为其他的数值
*/
int extract_file_lists(char* raw_html, struct file_list *f_list, int* list_num);

/*	获取课程文件详情页面
	course_id : 课程标识符
	work_id : 作业标识符
	notice_page : 存放页面源码的字符串
	返回 ; 成功时0，否则为错误码
*/
int get_homework_detail_page(int course_id, int work_id,char* page_buff);

/*
    将网页中的作业列表提取出来
    <ram_html>  字符串形式的网页源代码，为点击课程作业后的网络学堂首页中iframe内的内容
     <work_list>      作业数组，每一个元素对应一个作业，具体结构请查询头文件
     <course_id>  	课程序号， 你会用到它来获取作业的说明以及附件信息
      <list_num>   作业数量
      返回值 ： 成功时为0, 否则为其他的数值
*/
int extract_homework_list(char* raw_html, struct  homework *work_list, int* list_num);

/*	获取上传用的表单以及一个普通格式的表单
	up_file_name   :  待上传文件在远端的文件名
	page_buff         :  文件上传页面源码
	local_file_name : 待上传文件在本地的文件名
	form_buff	    :  普通格式表单的存放位置
	返回 ： 一个上传专用表格的结构体
*/
struct curl_httppost* extract_submit_form(char *up_file_name, char *page_buff,char* local_file_name, char* form_ptr);

/*	使用已经生成的表格上传一个文件
	formpost : 上传专用的表格
	form_buff ： 通常形式的表格
	返回 ： 成功时0，否则为错误码
*/
int upload_homewk(struct curl_httppost* formpost, char* form_buff);
#endif