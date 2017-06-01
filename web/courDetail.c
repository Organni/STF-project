#include "webOps.h"
#include "courDetail.h"

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
	memcpy(URL, newURL, i);
	URL[i] = '\0';
	//printf("newURL : %s\n",URL);

	// get real note page
	send_get(URL, get_cookie(), content, NULL);
	memcpy(notice_page, content, strlen(content));
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
	html_trim(content,detail_page);
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
		memcpy(temp_list[course_notice_num].title,p+i,j-i);
		//publisher   height=25>柴成亮</td>
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		memcpy(temp_list[course_notice_num].publisher,p+i,j-i);

		//time   同上
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		memcpy(temp_list[course_notice_num].time,p+i,j-i);
		//status     同上
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		memcpy(temp_list[course_notice_num].status,p+i,j-i);
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
		memcpy(temp_list[file_list_num].name,p+i,j-i);
		//printf("name: %s\n",temp_list[file_list_num].name);
		file_list_num++;
		head = i + string_find(p+i,"NN_showImage");
		
	}
	*list_num = file_list_num;
	
	
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
			i = i + string_find(p+i,"href=")+6;
			j = i + string_find(p+i,"\"");
			memcpy(temp_list[k].files[course_file_num].file_path,p+i,j-i);
			//printf("path: %s\n",temp_list[k].files[course_file_num].file_path);
			//printf("i= %d\n",i);
			//title 	id=1740355" >讲稿01     </a>
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</a>");
			char temp_title[100];
			memcpy(temp_title,p+i,j-i);
			string_trim(temp_title,temp_list[k].files[course_file_num].title); //去除空格
			//printf("title: %s\n",temp_list[k].files[course_file_num].title);
			//printf("i= %d\n",i);
			//intro   center">数学实验与建模概述</td>
			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			memcpy(temp_list[k].files[course_file_num].intro,p+i,j-i);
			//printf("intro: %s\n",temp_list[k].files[course_file_num].intro);
			//printf("i= %d\n",i);
			//file_size   同上
			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			memcpy(temp_list[k].files[course_file_num].file_size,p+i,j-i);
			//printf("size: %s\n",temp_list[k].files[course_file_num].file_size);
			//printf("i= %d\n",i);
			//time    同上
			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			memcpy(temp_list[k].files[course_file_num].upload_time,p+i,j-i);
			//printf("time: %s\n",temp_list[k].files[course_file_num].upload_time);
			//printf("i= %d\n",i);
			//status    同上
			i = i + string_find(p+i,"<td")+3;
			i = i + string_find(p+i,">")+1;
			j = i + string_find(p+i,"</td");
			char temp_status[50];
			memcpy(temp_status,p+i,j-i);
			string_trim(temp_status,temp_list[k].files[course_file_num].status);
			//printf("status: %s\n",temp_list[k].files[course_file_num].status);
			
		
			course_file_num++;

			head = i + string_find(p+i,"href");
			//printf("head %d\n",head);
		}
		
		p = p + flag;
		
		temp_list[k].file_num = course_file_num;
	}
	
	memcpy(f_list, temp_list, sizeof(struct file_list)*file_list_num);

	printf("file_list end\n");
	return 0;
}

/*
     file_id 似乎并非必要的
*/
int download_course_file(int course_id, int file_id, char* file_path, char* save_path){
	char URL[256] = "http://learn.tsinghua.edu.cn/uploadFile/downloadFile_student.jsp?module_id=322&filePath=";
	strcat(URL, file_path);
	strcat(URL, "&course_id=");
	char num_str[50];
	sprintf(num_str, "%d", course_id);
	strcat(URL, num_str);
	strcat(URL, "&file_id=");
	sprintf(num_str, "%d", file_id);
	strcat(URL, num_str);

	char header_buff[5000];
	send_download(URL, get_cookie(), header_buff, save_path);
	//printf("URL:%s\n", URL);
	//printf("HEADER:%s\n", header_buff);

	//从header中发现文件的后缀名
	char *suffix_start = strstr(header_buff, "filename=\"");
	char *suffix_end = strstr(suffix_start + 10, "\""); 		//"filename=""之后的引号位置，即文件名的结束
	suffix_start = strstr(suffix_start, ".");
	if(suffix_start > suffix_end)				// "."的位置超过了文件名，说明原文件没有后缀名
		return 0;
	char new_path[255];
	strcpy(new_path, save_path);
	strncat(new_path, suffix_start, suffix_end-suffix_start);
	//printf("[new_path]%s\n", new_path);
	rename(save_path, new_path);
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
		char status[20];
		char handin_size[20];		// 提交的作业大小
		char intro[1000];			// 作业说明
		char appendix_name[100];	// 作业附件名称
		char appendix_path[100]; 	//  作业附件的file_path，可以从超链接得到
		char handin_name[100];		// 提交的作业名称和file_path
		char handin_path[100];
		};
	*/
	//printf("twl html: %s\n",raw_html);
/* 	
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
		memcpy(temp_list[course_notice_num].title,p+i,j-i);
		//publisher   height=25>柴成亮</td>
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		memcpy(temp_list[course_notice_num].publisher,p+i,j-i);

		//time   同上
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		memcpy(temp_list[course_notice_num].time,p+i,j-i);
		//status     同上
		i = i + string_find(p+i,"<td")+3;
		i = i + string_find(p+i,">")+1;
		j = i + string_find(p+i,"</td");
		//printf("i= %d\n",i);
		memcpy(temp_list[course_notice_num].status,p+i,j-i);
		//content
		
		//get_notice_detail_page(course_id, id, &temp_list[course_notice_num].content);
		
		
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
	 */
	
	
	
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

    while(strIn[i]==' ' || strIn[i]=='\n' || strIn[i]=='\t' || strIn[i]=='\r')
        --j;
    memcpy(strOut, strIn + i , j - i + 1);
    strOut[j - i + 1] = '\0';
}

//取出html格式
void html_trim(char * strIn,char * strOut)
{
	char temp1[1000];
	char temp2[1000];
	int i,j,tail;
	int length ;
	char *p = strIn;
	tail = 0;
	length = 0;
	i=0; j=0;
	i= string_find(p,"</head>");
	p = p+i+7;
	i = string_find(p,">");
	j = i + 1 + string_find(p+i+1,"<");
	
	
	while(i!=-1 && j!=i)
	{
		p = p+i+1;
		if(j-i-1>0)
		{
			memcpy(temp1,p,j-i-1);
			temp1[j-i-1]='\0';
			string_trim(temp1,temp2);
			length = strlen(temp2);
			if(length>0)
			{
				temp2[length] = '\n';
				length++ ;
				memcpy(strOut+tail,temp2,length);
				tail = tail+length;
			}
		}
		i = string_find(p,">");
		j = i + 1 + string_find(p+i+1,"<");
	}
	
	strOut[tail] = '\0';
}


int main(int argc, char** argv){
	char username[] = "";		//在这里输入你的用户名和密码来测试
	char userpass[] = "";
	if(web_get_cookie(username, userpass) != 0)
		return -1;

	char page_buff [200000];

	//测试通知提取
	int course_id = 142241;				//使用你的课程序号
	get_notice_page(course_id, page_buff);
	struct course_notice notice_list[100];
	int notice_num = 0;
	extract_notice_list(page_buff, course_id, notice_list, &notice_num);
	int i = 0;
	for(; i < notice_num; i++)
		printf("%s\n", notice_list[i].title);

	//测试课程文件列表提取
	struct file_list f_list[10];
	int list_num = 0;
	get_file_page(course_id, page_buff);
	extract_file_lists(page_buff, f_list, &list_num);

 
	//测试课程文件下载
/* 	course_id = ;
	int file_id = 17407;
	char file_path[] = "pQBMevczBtpsZni82CpX7BZk9vHu/lvu9f/HBuhd9TjwRGPdtG/JM%2BmEypmvASL8Opb9znfBqtM%3D";
	download_course_file(course_id, file_id, file_path, "课程说明"); 
*/
	


	//测试作业详情页面
	int work_id = 744673;
	course_id = 143812;
	memset(page_buff,0,200000);
	get_homework_detail_page(course_id, work_id, page_buff);
	//printf("[作业详情]%s\n", page_buff);

	//测试作业提取
	memset(page_buff,0,200000);
	get_homework_page(course_id, page_buff);
	struct homework work_list[50];
	int work_num = 0;
	extract_homework_list(page_buff, work_list, &work_num);

	return 0;
}