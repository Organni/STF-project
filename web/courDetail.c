#include "webOps.h"
#include "courDetail.h"

extern FILE* log_file;

char *submit_form_elements[] = {"old_filename", "errorURL", "returnURL", "newfilename", "post_id", "post_rec_id",
							     "post_homewk_link", "file_unique_flag", "url_post", "css_name", "tmpl_name", "course_id", "module_id"};

void myReplace(char* a)
{
	while(*a != '\0')
	{
		if(*a == '/')
			*a = '-';
		a ++;
	}
}

int get_notice_page(int course_id, char* notice_page){
	char URL[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/public/bbs/getnoteid_student.jsp?course_id=";
	char num_str[50];
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);

	//char header[500];
	char content[50000];
	memset(content, 0, 50000);

	send_get(URL, get_cookie(), content, NULL);

	//printf("HEADER:%s\n",header);
	//printf("CONTENT:%s\n",content);

	// get real URL
	char *newURL = strstr(content, "http");
	int i = strstr(newURL,"\"") - newURL;
	strncpy(URL, newURL, i);
	URL[i] = '\0';
	//printf("newURL : %s\n",URL);

	// get real note page
	send_get(URL, get_cookie(), content, NULL);
	strncpy(notice_page, content, strlen(content));
	//printf("CONTENT:%s\n",content);
	return 0;
}

int get_notice_detail_page(int course_id, int notice_id, char *detail_page){
	char URL[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/public/bbs/note_reply.jsp?bbs_type=课程公告&id=";
	char num_str[50];
	char content[50000];
	memset(content,0,sizeof(content));
	sprintf(num_str, "%d", notice_id);
	strcat(URL, num_str);
	strcat(URL, "&course_id=" );
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);
	//printf("%s\n", URL);
	send_get(URL, get_cookie(),content, NULL);
	//printf("after get before trim \n");
	html_trim(content,detail_page);
	string_trim(detail_page, detail_page);
	//printf("CONTENT:%s\n",detail_page);
	return 0;
}

/*
    将网页中的通知列表提取出来
    <ram_html>  字符串形式的网页源代码，为点击课程公告后的网络学堂首页中iframe内的内容
     <course_id>  	课程序号， 你会用到它来获取课程公告的正文
     <notice_list>      课程公告列表，其结构参考头文件
      <inotice_num>   课程数量
      返回值 ： 成功时为0, 否则为其他的数值
*/
int extract_notice_list(char* raw_html, int course_id, struct course_notice *notice_list, int* notice_num){

	/*
	当你拿到通知的id后，使用下面的代码获取含有正文的网页,并进一步从中提取正文
	char content_page[50000];
	get_notice_detail_page(course_id, notice_id, &content_page);
	
		YOUR  CODE  TO FILL THE LIST
		struct course_notice{
		int notice_id;
		char title[100];		
		char publisher[20];	
		char time[20];
		char status[20];
		char content[2000];
		};
	*/
	//printf("twl html: %s\n",raw_html);
	struct course_notice  temp_list[50];
	memset(&temp_list, 0, sizeof(struct course_notice)*50);
	int course_notice_num = 0; //本来应该用 notice_num比较好，但是已经被用了，
	
	int head = 0;
	int i=0,j=0;
	//int tail = 0;
	char * p = raw_html;
	head = string_find(p,"课程公告"); //以“课程公告”作为公告标识
	while(head>=i)
	{
		p = p+head;
		//id   课程公告&id=2026620&course_id=143187'>
		i = string_find(p,"&id=")+4;
		//printf("i= %d\n",i);
		int id = 0;
		while(p[i]!='&')//遇到‘&’表示id结束，可增加是否是数字的判断
		{
			id = id*10 + p[i] - '0';
			i++ ;
		}
		temp_list[course_notice_num].notice_id = id;
		//course_id
		i = i + string_find(p+i,"course_id")+10;
		int course_id = 0;
		while(p[i]!='\'')//遇到‘'’表示id结束，可增加是否是数字的判断
		{
			course_id = course_id*10 + p[i] - '0';
			i++ ;
		}
		
		//title 	course_id=143187'><font color=red>大作业说明2</font></a> || course_id=143187'>实验三已发布</a>
		i = i + string_find(p+i,">")+1;
		if( p[i]=='<' )
			i = i+string_find(p+i,">")+1;
		//printf("i= %d\n",i);
		j = i + string_find(p+i,"</");
		strncpy(temp_list[course_notice_num].title,p+i,j-i);
		temp_list[course_notice_num].title[j-i] = '\0';
		myReplace(temp_list[course_notice_num].title);

		//publisher   height=25>柴成亮</td>
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		strncpy(temp_list[course_notice_num].publisher,p+i,j-i);
		temp_list[course_notice_num].publisher[j-i] = '\0';

		//time   同上
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		strncpy(temp_list[course_notice_num].time,p+i,j-i);
		temp_list[course_notice_num].time[j-i] = '\0';
		//status     同上
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		strncpy(temp_list[course_notice_num].status,p+i,j-i);
		temp_list[course_notice_num].status[j-i] = '\0';
		//content
		
		get_notice_detail_page(course_id, id, temp_list[course_notice_num].content);
		
		
		//printf("num: %d\n",course_notice_num);
		//printf("id: %d\n",temp_list[course_notice_num].notice_id);
		//printf("title: %s\n",temp_list[course_notice_num].title);
		//printf("publisher: %s\n",temp_list[course_notice_num].publisher);
		//printf("time: %s\n",temp_list[course_notice_num].time);
		//printf("status: %s\n",temp_list[course_notice_num].status);
		//printf("content: %s\n",temp_list[course_notice_num].content);
		
		
		course_notice_num++;
		//tail += head;
		//printf("tail %d\n",tail);
		head = i + string_find(p+i,"课程公告");
		//printf("head %d\n",head);

	}
	

	
	memcpy(notice_list, temp_list, sizeof(struct course_notice)*course_notice_num);
	*notice_num = course_notice_num;
	
	//printf("detail content unsolved !return\n\n\n");
	
	return 0;
}

int get_file_page(int course_id, char*page_buff){
	char URL[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/download.jsp?course_id=";
	char num_str[50];
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);

	//char header[500];

	send_get(URL, get_cookie(), page_buff, NULL);

	//printf("HEADER:%s\n",header);
	//printf("CONTENT:%s\n",page_buff);

	return 0;	
}

/*
    将网页中的文件列表提取出来
    <ram_html>  字符串形式的网页源代码，为点击课程文件后的网络学堂首页中iframe内的内容
     <f_list>       文件列表数组，每一个元素对应一个标签下的文件列表，具体结构请查询头文件
      <list_num>   文件列表数量
      返回值 ： 成功时为0, 否则为其他的数值
*/
int extract_file_lists(char* raw_html, struct file_list *f_list, int* list_num){

/*
	struct file_list
	{
		char name[20];
		int file_num;
		struct course_file files[100];
	};
*/

	//printf("begin file list\n");
	struct file_list  temp_list[10];
	memset(&temp_list, 0, sizeof(struct file_list)*10);
	int file_list_num = 0; 
	
	int head = 0;
	int i=0,j=0;
	//int tail = 0;
	char * p = raw_html;
	for( i=0;i<3;i++ )
	{
		head = string_find(p,"NN_showImage"); //以“NN_showImage”作为公告标识
		p = p+head+12;
	}
	head = string_find(p,"NN_showImage");	
	i=0;
	while(head>=i)	//获取列表标题、文件列表数
	{
		p = p+head+12;
		//name   NN_showImage(3,2)">参考资料</td>
		i = string_find(p,")\">")+3;
		j = i + string_find(p+i,"</td");
		if(j-i>19)	printf("name too long");
		strncpy(temp_list[file_list_num].name,p+i,j-i);
		//printf("name: %s\n",temp_list[file_list_num].name);
		file_list_num++;
		head = i + string_find(p+i,"NN_showImage");
		
	}
	*list_num = file_list_num;
	
	/*for(int i = 0; i < file_list_num; i++)
		fprintf(log_file, "[extract_file_lists   ]   %s\n", temp_list[i].name);
	fflush(log_file);*/

	int k=0;
	for(k=0;k<file_list_num;k++) //对每一个文件列表获取内容
	{
		/*
		struct course_file{
			int file_id;		// 非必要
			char title[100];
			char intro[255];
			char file_size[20];
			char upload_time[20];
			char status[50];
			char file_path[100];	// 查看文件标题对应的超链接可以得到
		};
		*/
	
		int course_file_num = 0; 
		int flag = 0;
		i=0;
		head = string_find(p,"<table"); //以“table”作为列表开始标识
		head = head + string_find(p+head,"状态")+2;
		head = head + string_find(p+head,"href"); 
		flag = head + string_find(p+head,"</table");
		
		//printf("%d\n",k);
		//printf("flag: %d\n",flag);
		char temp_title[200];
		while(head>=i && head<flag )
		{
			p = p+head+5;
			flag = flag - head -5;
			i=0;
			int t=0;
			//file_id   
			temp_list[k].files[course_file_num].file_id = course_file_num+1;
			//printf("num: %d\n",course_file_num);
			//file_path
			i = i + string_find(p+i,"&filePath=")+strlen("&filePath=");
			j = i + string_find(p+i,"&course_id=");
			strncpy(temp_list[k].files[course_file_num].file_path,p+i,j-i);
			//printf("path: %s\n",temp_list[k].files[course_file_num].file_path);
			//printf("i= %d\n",i);
			//title 	id=1740355" >讲稿01     </a>
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</a>");
			memset(temp_title,0,sizeof(temp_title));
			strncpy(temp_title,p+i,j-i);
			temp_title[j-i] = '\0';
			string_trim(temp_title,temp_list[k].files[course_file_num].title); //去除空格
			//fprintf(log_file, "[in while loop]%s\n", temp_list[k].files[course_file_num].title);
			//printf("title: %s\n",temp_list[k].files[course_file_num].title);
			//printf("i= %d\n",i);
			//intro   center">数学实验与建模概述</td>
			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			strncpy(temp_list[k].files[course_file_num].intro,p+i,j-i);
			//printf("intro: %s\n",temp_list[k].files[course_file_num].intro);
			//printf("i= %d\n",i);
			//file_size   同上
			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			strncpy(temp_list[k].files[course_file_num].file_size,p+i,j-i);
			//printf("size: %s\n",temp_list[k].files[course_file_num].file_size);
			//printf("i= %d\n",i);
			//time    同上
			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			strncpy(temp_list[k].files[course_file_num].upload_time,p+i,j-i);
			//printf("time: %s\n",temp_list[k].files[course_file_num].upload_time);
			//printf("i= %d\n",i);
			//status    同上

			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			//fprintf(log_file, "%s %d %d\n", "there is ok?",i,j);fflush(log_file);
			char temp_status[200];
			strncpy(temp_status,p+i,j-i);
			temp_status[j-i] = '\0';
			//fprintf(log_file, "[in the while loop]%s\n", temp_status);fflush(log_file);
			string_trim(temp_status,temp_list[k].files[course_file_num].status);
			//printf("status: %s\n",temp_list[k].files[course_file_num].status);
			
		
			course_file_num++;

			head = i + string_find(p+i,"href");
			//printf("head %d\n",head);
		}
		
		p = p + flag;
		
		temp_list[k].file_num = course_file_num;
		//fprintf(log_file, "[in the loop]%d %d\n", k, course_file_num);fflush(log_file);
	}

	//fprintf(log_file, "%s\n", "come out the for loop");fflush(log_file);
	
	memcpy(f_list, temp_list, sizeof(struct file_list)*file_list_num);

	//printf("file_list end\n");
	return 0;
}

/*
     file_id 似乎并非必要的
*/
int download_course_file(int course_id, int file_id, char* file_path, char* save_path){
	fprintf(log_file, "[DOWNLOAD]\ncid : %d\n fid : %d\n f_path : %s\ns_path : %s\n", course_id, file_id,file_path, save_path);
	fflush(log_file);
	char URL[512] = "http://learn.tsinghua.edu.cn/uploadFile/downloadFile_student.jsp?module_id=322&filePath=";
	strcat(URL, file_path);
	strcat(URL, "&course_id=");
	char num_str[50];
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);
	strcat(URL, "&file_id=");
	sprintf(num_str, "%d", file_id);
	strcat(URL, num_str);

	char header_buff[5000];
	memset(header_buff,0,5000);
	int rst = send_download(URL, get_cookie(), header_buff, save_path);
	fprintf(log_file, "[POST]%d\n", rst);
	fflush(log_file);
	//printf("URL:%s\n", URL);
	//printf("HEADER:%s\n", header_buff);
	fprintf(log_file, "[HEADER]%s\n", header_buff);
	fflush(log_file);
	//从header中发现文件的后缀名
	char *suffix_start = strstr(header_buff, "filename=\"");
	char *suffix_end = strstr(suffix_start + 10, "\""); 		//"filename=""之后的引号位置，即文件名的结束
	suffix_start = strstr(suffix_start, ".");
	if(suffix_start > suffix_end)				// "."的位置超过了文件名，说明原文件没有后缀名
		return 0;
	char new_path[255];
	memset(new_path,0,255);
	strcpy(new_path, save_path);
	//fprintf(log_file,"[new_path]%sI bet there is a ...%s???\n", new_path, save_path);
	//fflush(log_file);
	strncat(new_path, suffix_start, suffix_end-suffix_start);
	rename(save_path, new_path);
	strcpy(save_path, new_path);
	return 0;
}

int get_homework_page(int course_id, char* page_buff){
	char URL[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/hom_wk_brw.jsp?course_id=";
	char num_str[50];
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);

	//char header[500];

	send_get(URL, get_cookie(), page_buff, NULL);

	//printf("HEADER:%s\n",header);
	//printf("CONTENT:%s\n",content);

	return 0;	
}

int get_homework_detail_page(int course_id, int work_id,char* page_buff){
	char URL[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/hom_wk_detail.jsp?id=";
	char num_str[50];
	sprintf(num_str, "%d", work_id);
	strcat(URL, num_str);
	strcat(URL, "&course_id=");
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);

	//char header[500];

	send_get(URL, get_cookie(), page_buff, NULL);

	//printf("HEADER:%s\n",header);
	//printf("CONTENT:%s\n",content);
	//printf("page url: %d,%d\n",course_id,work_id);
	//printf("page: %s\n",page_buff);
	return 0;	
}

/*
    将网页中的作业列表提取出来
    <ram_html>  字符串形式的网页源代码，为点击课程作业后的网络学堂首页中iframe内的内容
     <work_list>      作业数组，每一个元素对应一个作业，具体结构请查询头文件
     <course_id>  	课程序号， 你会用到它来获取作业的说明以及附件信息
      <list_num>   作业数量
      返回值 ： 成功时为0, 否则为其他的数值
*/
int extract_homework_list(char* raw_html, struct  homework *work_list, int* list_num){

	/*
		当你获得一个作业的序号后，使用以下代码获得作业详情的网页，并从中获取作业的说明，附件等
		char page_buff[5000];
		get_homework_detail_page(icourse_id, work_id, page_buff)；
	
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
		char handin_name[100];		// 提交的作业名称和file_path
		char handin_path[100];
		};
	*/
	//printf("twl html: %s\n",raw_html);
 	//fprintf(log_file, "%s\n", "come into extract_homework_list");fflush(log_file);
	struct homework temp_list[50];
	memset(&temp_list, 0, sizeof(struct homework)*50);
	int homework_num = 0; 
	
	int head = 0;
	int i=0,j=0;
	char * p = raw_html;
	for( i=0;i<3;i++ )
	{
		head = string_find(p,"<table"); //以第三个“<table”作为作业列表开始标识
		p = p+head+6;
	}
	i=0;
	head = string_find(p,"<tr"); //以“<tr”作为作业标识
	while(head>=i)
	{
		
		p = p+head+3;
		//id    href="hom_wk_detail.jsp?id=740753&course_id=142241&rec_id=null"
		i = string_find(p,"?id=")+4;
		if(i==3)	break ;
		//printf("i= %d\n",i);
		int id = 0;
		while(p[i]!='&')//遇到‘&’表示id结束，可增加是否是数字的判断
		{
			id = id*10 + p[i] - '0';
			i++ ;
		}
		temp_list[homework_num].id = id;
		//printf("id: %d\n",temp_list[homework_num].id);
		//course_id
		i = string_find(p,"rse_id=")+7;
		//printf("i= %d\n",i);
		int course_id = 0;
		while(p[i]!='&')//遇到‘&’表示id结束，可增加是否是数字的判断
		{
			course_id = course_id*10 + p[i] - '0';
			i++ ;
		}
		//printf("course_id: %d\n",course_id);
		//title 	rec_id=null">插值与数值积分</a></td>
		i = i + string_find(p+i,">")+1;
		//if( p[i]=='<' )
			//i = i+string_find(p+i,">")+1;
		//printf("i= %d\n",i);
		j = i + string_find(p+i,"</a");
		strncpy(temp_list[homework_num].title,p+i,j-i);
		temp_list[homework_num].title[j-i] = '\0';
		string_trim(temp_list[homework_num].title, temp_list[homework_num].title);
		//fprintf(log_file, "[in the while loop]%s\n", temp_list[homework_num].title);fflush(log_file);
		//printf("title: %s\n",temp_list[homework_num].title);
		//start_time[20];     <td width="10%">2017-02-28</td>
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		strncpy(temp_list[homework_num].start_time,p+i,j-i);
		//printf("start: %s\n",temp_list[homework_num].start_time);
		//char end_time[20];
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		strncpy(temp_list[homework_num].end_time,p+i,j-i);
		//printf("end: %s\n",temp_list[homework_num].end_time);
		//char status[50];
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		char temp_status[100];
		strncpy(temp_status,p+i,j-i);
		temp_status[j-i]='\0';
		string_trim(temp_status,temp_list[homework_num].status);
		//printf("status: %s\n",temp_list[homework_num].status);
		//char handin_size[20];		// 提交的作业大小
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		char temp_size[50];
		strncpy(temp_size,p+i,j-i);
		temp_size[j-i]='\0';
		string_trim(temp_size,temp_list[homework_num].handin_size);
		//printf("size: %s\n",temp_list[homework_num].handin_size);
		//fprintf(log_file, "%s\n", "before come in the homework page");fflush(log_file);
		//进入作业页面

		char page_buff[20000];
		//printf("before\n");
		memset(page_buff,0,sizeof(page_buff));
		get_homework_detail_page(course_id, id, page_buff);
		//fprintf(log_file, "%s\n", "get homework page is ok?");fflush(log_file);
		//printf("after\n");
		//printf("%s",page_buff);
		char * q = page_buff;
		int qi = 0;
		int qj = 0;
		//char intro[1000];			// 作业说明
		qi = string_find(q,"作业说明")+4; 
		qi = qi + string_find(q+qi,"<td")+3;
		qi = qi + string_find(q+qi,"<textarea")+9;
		qi = qi + string_find(q+qi,">")+1;
		qj = qi + string_find(q+qi,"</textarea");
		//printf("intro\n");
		strncpy(temp_list[homework_num].intro,q+qi,qj-qi);
		//fprintf(log_file, "[in the homework page]%s\n", temp_list[homework_num].intro);fflush(log_file);
		//printf("intro: %s\n",temp_list[homework_num].intro);
		//char appendix_name[100];	// 作业附件名称
		//char appendix_path[100]; 	//  作业附件的file_path，可以从超链接得到	
		qi = qi + string_find(q+qi,"作业附件")+4;
		qi = qi + string_find(q+qi,"<td")+3;
		qi = qi + string_find(q+qi,">")+1;
		qj = qi + string_find(q+qi,"</td");
		int flag1 = 0;
		int flag2 = 0;
		flag1 = qi + string_find(q+qi,"无相关文件");
		flag2 = qi + string_find(q+qi,"href");
		if ((flag2>qi-1 &&flag2<qj)&&(flag1==qi-1||flag1>qj)) //
		{
			qi = qi + string_find(q+qi,"&filePath=")+strlen("&filePath="); 
			qj = qi + string_find(q+qi,"\"");
			strncpy(temp_list[homework_num].appendix_path,q+qi,qj-qi);
			//printf("path: %s\n",temp_list[homework_num].appendix_path);
			qi = qi + string_find(q+qi,">")+1;
			qj = qi + string_find(q+qi,"</a");
			strncpy(temp_list[homework_num].appendix_name,q+qi,qj-qi);
			//printf("name: %s\n",temp_list[homework_num].appendix_name);
		}

		//fprintf(log_file, "%s\n", "there is ok?");fflush(log_file);	
		
		//char handin_content[1000];
		//char handin_name[100];		// 提交的作业名称和file_path
		//char handin_path[100];
		qi = string_find(q,"上交作业内容")+6; 
		qi = qi + string_find(q+qi,"<td")+3;
		qi = qi + string_find(q+qi,"<textarea")+9;
		qi = qi + string_find(q+qi,">")+1;
		qj = qi + string_find(q+qi,"</textarea");
		if( qi==qj )
		{
			//temp_list[homework_num].handin_content=NULL;
		}else
		{
			strncpy(temp_list[homework_num].handin_content,q+qi,qj-qi);
		}

		//fprintf(log_file, "%s\n", "there1 is ok?");fflush(log_file);

		qi = qi + string_find(q+qi,"上交作业附件")+4;
		qi = qi + string_find(q+qi,"<td")+3;
		qi = qi + string_find(q+qi,">")+1;
		qj = qi + string_find(q+qi,"</td");
		flag1 = qi + string_find(q+qi,"无相关文件");
		flag2 = qi + string_find(q+qi,"href");
		//fprintf(log_file, "%d %d %d %d\n", qi, qj, flag1, flag2);fflush(log_file);
		if ((flag2>(qi-1) &&flag2<qj)&&(flag1==(qi-1)||flag1>qj)) //
		{//fprintf(log_file, "%s\n", "come in if");fflush(log_file);
			qi = qi + string_find(q+qi,"&filePath=")+strlen("&filePath="); 
			qj = qi + string_find(q+qi,"\"");
			//fprintf(log_file, "%d %d\n", qi, qj);
			strncpy(temp_list[homework_num].handin_path,q+qi,qj-qi);
			//fprintf(log_file, "%s\n", temp_list[homework_num].handin_path);fflush(log_file);
			//printf("path: %s\n",temp_list[homework_num].handin_path);
			qi = qi + string_find(q+qi,">")+1;
			qj = qi + string_find(q+qi,"</a");
			strncpy(temp_list[homework_num].handin_name,q+qi,qj-qi);
			//printf("name: %s\n",temp_list[homework_num].handin_name);
		}
		//fprintf(log_file, "%s\n", "after come in the homework page");fflush(log_file);	
				
		homework_num++;
		//tail += head;
		////printf("tail %d\n",tail);
		head = i + string_find(p+i,"<tr");
		//printf("head %d\n",head);
		//printf("\n\n\n");
	}
	
	
	memcpy(work_list, temp_list, sizeof(struct homework)*homework_num);
	*list_num = homework_num;
	
	//printf("return\n");
	
	return 0;
}

//去除字符串中的空格、换行
void string_trim(char * strIn,char * strOut)
{
    int i, j ;

    i = 0;

    j = strlen(strIn) - 1;	

    while(strIn[i]==' ' || strIn[i]=='\n' || strIn[i]=='\t' || strIn[i]=='\r')
        ++i;

    while(strIn[j]==' ' || strIn[j]=='\n' || strIn[j]=='\t' || strIn[j]=='\r')
        --j;

    if(j - i + 1 <= 0)
    {
    	strOut[0] = '\0';
    	return;
    }

    strncpy(strOut, strIn + i , j - i + 1);
    strOut[j - i + 1] = '\0';

    char temp[j-i+1];
    memset(temp, 0, j-i+1);
	char delims[] = "&nbsp;";
	char *result = NULL;
	result = strtok(strOut, delims);
	if(result == NULL)
		return;
	while(result != NULL)
	{
		strcat(temp, result);
		result = strtok(NULL, delims);
	}
	strcpy(strOut, temp);
    /*char* off = strOut;
    while((off = strstr(off, "&nbsp;")) != NULL)
    {
    	off[0] = ' ';
    	strcpy(off + 1, off + 6);
    }*/
    /*off = strOut;
    while((off = strstr(off+1, "&lsquo;")) != NULL)
    {
    	off[0] = '\'';
    	memcpy(off, off + 7, strlen(off) - 7 + 1);
    }
    off = strOut;
    while((off = strstr(off+1, "&rsquo;")) != NULL)
    {
    	off[0] = '\'';
    	memcpy(off, off + 7, strlen(off) - 7 + 1);
    }
    off = strOut;
    while((off = strstr(off+1, "&hellip;")) != NULL)
    {
    	off[0] = ' ';
    	memcpy(off, off + 8, strlen(off) - 8 + 1);
    }*/
}

//取出html格式
void html_trim(char * strIn,char * strOut)
{
	printf("begin trim\n");
	char temp1[1000];
	char temp2[1000];
	char temp_content[50000];
	memset(temp_content,0,sizeof(temp_content));
	int i,j,tail;
	int length ;
	char *p = strIn;
	tail = 0;
	length = 0;
	i=0; j=0;
	i= string_find(p,"正文");
	p = p+i+2;
	i = string_find(p,"<td");
	p = p+i;
	j = string_find(p,"</td");
	
	printf("content_size: %d",j+2);
	strncpy(temp_content,p,j+2);
	
	
	
	p = temp_content;
	i=0;j=0;
	i = string_find(p,">");
	j = i + 1 + string_find(p+i+1,"<");
	
	
	while(i!=-1 && j!=i)
	{
		p = p+i+1;
		if(j-i-1>0)
		{
			strncpy(temp1,p,j-i-1);
			temp1[j-i-1]='\0';
			string_trim(temp1,temp2);
			length = strlen(temp2);
			if(length>0)
			{
				temp2[length] = '\n';
				length++ ;
				strncpy(strOut+tail,temp2,length);
				tail = tail+length;
			}
		}
		i = string_find(p,">");
		j = i + 1 + string_find(p+i+1,"<");
	}
	
	strOut[tail] = '\0';
}

int get_homework_submit_page(int course_id, int work_id, char* page_buff){
	char URL[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/hom_wk_submit.jsp?id=";
	char num_str[50];
	sprintf(num_str, "%d", work_id);
	strcat(URL, num_str);
	strcat(URL, "&course_id=");
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);

	//char header[500];

	send_get(URL, get_cookie(), page_buff, NULL);

	//printf("HEADER:%s\n",header);
	//printf("CONTENT:%s\n",content);
	//printf("page url: %d,%d\n",course_id,work_id);
	//printf("page: %s\n",page_buff);
	return 0;
}

/*	获取上传用的表单
	up_file_name   :  待上传文件名
	page_buff         :  文件上传页面源码
	form_buff	    :  上传表单
*/
struct curl_httppost* extract_submit_form(char *up_file_name, char *page_buff,char* local_file_name, char* form_ptr){
	struct curl_httppost *formpost=NULL;  
  	struct curl_httppost *lastptr=NULL;
	// find form
	char* ptr = strstr(page_buff, "<FORM id=\"F1\""), *next;
	if(!ptr){
		//fprintf(log_file, "[SUBMIT] no form in page_buff\n");
		return NULL;
	}
	char form_buff[500];
	memset(form_buff, 0, 500);

	// find saveDir
	ptr = strstr(ptr, "value=\"") + strlen("value=\"");
	next = strstr(ptr, "\"");
	strncpy(form_buff, ptr, next - ptr);
	form_buff[next - ptr] = 0;
	strcat(form_ptr,"saveDir=");
	strcat(form_ptr, form_buff);
	curl_formadd(&formpost,  
               &lastptr,  
               CURLFORM_COPYNAME, "saveDir",  
               CURLFORM_COPYCONTENTS, form_buff,  
               CURLFORM_END);
	ptr = next;
	// add filename
	curl_formadd(&formpost,  
               &lastptr,  
               CURLFORM_COPYNAME, "filename",  
               CURLFORM_COPYCONTENTS,up_file_name,  
               CURLFORM_END);
	strcat(form_ptr,"&filename=");
	strcat(form_ptr, up_file_name);
	int i = 0;
	for(;i < 13; i++){
		ptr = strstr(ptr, "value=") + strlen("value=\"");
		next = strstr(ptr, ">")-1;
		strncpy(form_buff, ptr, next - ptr);
		form_buff[next - ptr] = 0;
		if(i != 6){
			curl_formadd(&formpost,  
	               	&lastptr,  
	               	CURLFORM_COPYNAME, submit_form_elements[i],  
	               CURLFORM_COPYCONTENTS, form_buff,  
	               CURLFORM_END);
			strcat(form_ptr,"&");
			strcat(form_ptr, submit_form_elements[i]);
			strcat(form_ptr, "=");
			strcat(form_ptr, form_buff);
		}else{
			curl_formadd(&formpost,  
	               	&lastptr,  
	               	CURLFORM_COPYNAME, submit_form_elements[i],  
	               CURLFORM_COPYCONTENTS, up_file_name,  
	               CURLFORM_END);
			strcat(form_ptr,"&");
			strcat(form_ptr, submit_form_elements[i]);
			strcat(form_ptr, "=");
			strcat(form_ptr, up_file_name);
		}
		
		ptr = next;
	}
	curl_formadd(&formpost,  
               &lastptr,  
               CURLFORM_COPYNAME, "post_rec_homewk_detail",  
               CURLFORM_COPYCONTENTS, "",  
               CURLFORM_END);
	curl_formadd(&formpost,  
               &lastptr,  
               CURLFORM_COPYNAME, "submit",  
               CURLFORM_COPYCONTENTS, "提交",  
               CURLFORM_END);
	curl_formadd(&formpost,  
               &lastptr,  
               CURLFORM_COPYNAME, "sendfile",  
               CURLFORM_FILE, local_file_name,  
               CURLFORM_END);
	strcat(form_ptr, "&post_rec_homewk_detail=&submit=提交");
	return formpost;
}

int upload_homewk(struct curl_httppost* formpost, char* form_buff){
	char URL[256] = "http://learn.tsinghua.edu.cn/uploadFile/uploadFile.jsp";
	char URL2[256] = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/hom_wk_handin.jsp";
	char header[5000];
	char content[50000];
	memset(header, 0, 5000);
	memset(content, 0, 50000);

	int rst = send_upload(URL, get_cookie(), header, content, formpost, NULL);
	//printf("[UPLOADHEADER]%s\n", header);
	//printf("[UPLOADCONTENT]%s\n",content);
	memset(header, 0, 5000);
	memset(content, 0, 50000);
	rst = send_upload(URL2, get_cookie(), header, content, formpost, form_buff);

	//printf("[UPLOADHEADER]%s\n", header);
	//printf("[UPLOADCONTENT]%s\n",content);
	return rst;
}

/*int main(int argc, char** argv){
	char username[] = "";		//在这里输入你的用户名和密码来测试
	char userpass[] = "";
	fileInit();
	printf("[LOGGING]\n");
	if(web_get_cookie(username, userpass) != 0)
		return -1;
	printf("[LOGGED]\n");
	char page_buff [200000];
	

	//测试通知提取
	/*int course_id = 142241;				//使用你的课程序号
	get_notice_page(course_id, page_buff);
	struct course_notice notice_list[100];
	int notice_num = 0;
	extract_notice_list(page_buff, course_id, notice_list, &notice_num);
	int i = 0;
	for(; i < notice_num; i++)
		printf("%s\n", notice_list[i].title);*/

	//测试课程文件列表提取
	/*struct file_list f_list[10];
	int list_num = 0;
	get_file_page(course_id, page_buff);
	extract_file_lists(page_buff, f_list, &list_num);*/

 
	//测试课程文件下载
 /*	course_id = ;
	int file_id = 17407;
	char file_path[] = "pQBMevczBtpsZni82CpX7BZk9vHu/lvu9f/HBuhd9TjwRGPdtG/JM%2BmEypmvASL8Opb9znfBqtM%3D";
	download_course_file(course_id, file_id, file_path, "课程说明"); 

	


	//测试作业详情页面
	int work_id = 740753;
	course_id = 142241;
	memset(page_buff,0,200000);
	get_homework_detail_page(course_id, work_id, page_buff);
	//printf("[作业详情]%s\n", page_buff);

	//测试作业提取
	/*int course_id = 142241;
	memset(page_buff,0,200000);
	get_homework_page(course_id, page_buff);
	struct homework work_list[50];
	int work_num = 0;
	extract_homework_list(page_buff, work_list, &work_num);

	//测试作业上传
	
	int course_id = 142886, wk_id = 763013;
	int rst = 0;
	memset(page_buff, 0, sizeof(page_buff));
	get_homework_submit_page(course_id, wk_id, page_buff);
	char form_buff[50000];
	memset(form_buff, 0, sizeof(form_buff));
	//printf("[page]%d\n%s\n", 0,page_buff);

	struct curl_httppost *formpost = extract_submit_form( "log.txt",page_buff,"/home/jt/log.txt",form_buff);
	printf("[FORM]%d\n%s\n", rst,form_buff);

	/*FILE* file  = fopen("/home/jt/helloWeb","r");
	if(!file)
		printf("[UPLOAD]%s\n", strerror(errno));
	rst = upload_homewk(formpost,form_buff);
	printf("[UPLOAD]%d\n", rst);
	return 0;
}*/