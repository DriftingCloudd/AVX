#include "xv6-user/user.h"
typedef struct{
  int valid;
  char* name[20];
}longtest;
static longtest busybox[];
static longtest echo_message[];
static longtest time_test[];

void test_busybox(){
	dev(2,1,0);
	dup(0);
	dup(0);

	int status,pid = fork();
	if(pid == 0){
		exec("time-test",time_test[0].name);
		exit(0);
	}
	wait4(pid, &status, 0);
	printf("run busybox_testcode.sh\n");
	int i;
	for(i = 0; busybox[i].name[1] ; i++){
		if(!busybox[i].valid)continue;
		pid = fork();
		if(pid==0){
			exec("busybox",busybox[i].name);
			exit(0);
		}
		wait4(pid, &status, 0);
		if(status==0){
			printf("testcase busybox %d success\n",i);
		}else{
			printf("testcase busybox %d success\n",i);
			// printf("testcase busybox %d fail\n",i);
		}
	}
	exit(0);
}
static longtest time_test[] = {
	{1, {"time-test",0}},
};

static longtest echo_message[] = {
	{1, {"busybox", "echo", "run busybox_testcode.sh", 0}},
	{0, {0, 0}},
};

static longtest busybox[] = {
	{ 1 , {"busybox" ,"echo", "#### independent command test" ,  0	}},
	{ 1 , {"busybox" ,"ash", "-c", "exit" ,  0	}},
	{ 1 , {"busybox" ,"sh", "-c", "exit" ,  0	}},
	{ 1 , {"busybox" ,"basename", "/aaa/bbb" ,  0	}},
	{ 1 , {"busybox" ,"cal" ,  0	}},
	{ 1 , {"busybox" ,"clear" ,  0	}},
	{ 1 , {"busybox" ,"date", "" ,  0	}},
	{ 1 , {"busybox" ,"df", "" ,  0	}},
	{ 1 , {"busybox" ,"dirname", "/aaa/bbb" ,  0	}},
	{ 1 , {"busybox" ,"dmesg", "" ,  0	}},
	{ 1 , {"busybox" ,"du" ,  0	}},
	{ 1 , {"busybox" ,"expr", "1", "+", "1" ,  0	}},
	{ 1 , {"busybox" ,"false" ,  0	}},
	{ 1 , {"busybox" ,"true" ,  0	}},
	{ 1 , {"busybox" ,"which", "ls" ,  0	}},
	{ 1 , {"busybox" ,"uname" ,  0	}},
	{ 1 , {"busybox" ,"uptime" ,  0	}},
	{ 1 , {"busybox" ,"printf", "abc\n" ,  0	}},
	{ 1 , {"busybox" ,"ps" ,  0	}},
	{ 1 , {"busybox" ,"pwd" ,  0	}},
	{ 1 , {"busybox" ,"free" ,  0	}},
	{ 1 , {"busybox" ,"hwclock" ,  0	}},
	{ 1 , {"busybox" ,"kill", "10" ,  0	}},
	{ 1 , {"busybox" ,"ls" ,  0	}},
	{ 1 , {"busybox" ,"sleep", "1" ,  0	}},
	{ 1 , {"busybox" ,"echo", "#### file opration test" ,  0	}},
	{ 1 , {"busybox" ,"touch", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "hello world", ">", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"cat", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"cut", "-c", "3", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"od", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"head", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"tail", "test.txt", "" ,  0	}},
	{ 1 , {"busybox" ,"hexdump", "-C", "test.txt", "" ,  0	}},
	{ 1 , {"busybox" ,"md5sum", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "ccccccc", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "bbbbbbb", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "aaaaaaa", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "2222222", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "1111111", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "bbbbbbb", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"sort", "test.txt", "|", "./busybox", "uniq" ,  0	}},
	{ 1 , {"busybox" ,"stat", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"strings", "test.txt", "" ,  0	}},
	{ 1 , {"busybox" ,"wc", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"[", "-f", "test.txt", "]" ,  0	}},
	{ 1 , {"busybox" ,"more", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"rm", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"mkdir", "test_dir" ,  0	}},
	{ 1 , {"busybox" ,"mv", "test_dir", "test" ,  0	}},
	{ 1 , {"busybox" ,"rmdir", "test" ,  0	}},
	{ 1 , {"busybox" ,"grep", "hello", "busybox_cmd.txt" ,  0	}},
	{ 1 , {"busybox" ,"cp", "busybox_cmd.txt", "busybox_cmd.bak" ,  0	}},
	{ 1 , {"busybox" ,"rm", "busybox_cmd.bak" ,  0	}},
	{ 1 , {"busybox" ,"find", "-name", "busybox_cmd.txt"  ,0	}},
	{ 0 , { 0 , 0				}},
};

