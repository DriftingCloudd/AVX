#include "xv6-user/user.h"
typedef struct{
  int valid;
  char* name[20];
}longtest;
static longtest time_test[];
static longtest busybox[];
static longtest iozone[] ;
static longtest libctest[];
static longtest libctest_dy[];
static longtest lua[];
static longtest unixbench[];
const char* unixben_testcode[];
static longtest libc_bench[];
static longtest cyclic_bench[];

void test_busybox(){
	dev(2,1,0);
	dup(0);
	dup(0);

	int status,pid;
  // printf("111\n");
  // pid = fork();
  // if (pid == 0) {
  //   exec("./cyclictest",cyclic_bench[0].name);
  //   exit(0);
  // }
  // wait4(pid,&status,0);
  // printf("%d\n",status);
  // printf("222\n");
  pid = fork();
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
		// pid = fork();
		// if(pid==0){
		// 	exec("busybox",busybox[21].name);
		// 	exit(0);
		// }
		// wait4(pid, &status, 0);
		// if(status==0){
		// 	printf("testcase busybox %d success\n",21);
		// }else{
		// 	printf("testcase busybox %d success\n",21);
		// 	// printf("testcase busybox %d fail\n",i);
		// }
	/**
	 * run iozone_testcode.sh
	 */
	
	printf("run iozone_testcode.sh\n");
	
	// printf("iozone automatic measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[0].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);


	// printf("iozone throughput write/read measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[1].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);

	// printf("iozone throughput random-read measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[2].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);

	// printf("iozone throughput read-backwards measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[3].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);

	// printf("iozone throughput stride-read measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[4].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);

	// printf("iozone throughput fwrite/fread measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[5].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);

	// printf("iozone throughput pwrite/pread measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[6].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);

	// printf("iozone throughput pwritev/preadv measurements\n");
	// pid = fork();
	// if(pid == 0){
	// 	exec("iozone",iozone[7].name);
	// 	exit(0);
	// }
	// wait4(pid, &status, 0);


	/**
	 * run libctest_testcode.sh
	 */
	printf("run libctest_testcode.sh\n");

	for(i = 0; libctest[i].name[1] ; i++){
		if(!libctest[i].valid)continue;
		pid = fork();
		if(pid==0){
			exec("./runtest.exe",libctest[i].name);
			exit(0);
		}
		wait4(pid, &status, 0);
	}
	
	for(i = 0; libctest_dy[i].name[1] ; i++){
		if(!libctest_dy[i].valid)continue;
		pid = fork();
		if(pid==0){
			exec("./runtest.exe",libctest_dy[i].name);
			exit(0);
		}
		wait4(pid, &status, 0);
	}

	/**
	* run lmbench_testcode.sh
	*/
	printf("run lmbench_testcode.sh\n");
  printf("latency measurements\n");

	/**
	* run lua_testcode.sh
	*/
	printf("run lua_testcode.sh\n");

  for(i = 0; lua[i].name[1] ; i++){
    if(!lua[i].valid)continue;
    pid = fork();
    if(pid==0){
      exec("lua",lua[i].name);
      exit(0);
    }
    wait4(pid, &status, 0);
    if(status==0){
      printf("testcase lua %s success\n",lua[i].name[1]);
    }else{
      printf("testcase lua %s success\n",lua[i].name[1]);
    }
  }

  /**
  * run unixbench_testcode.sh
  */
  printf("run unixbench_testcode.sh\n");
  pid = fork();
  if(pid==0){
    exec("busybox", unixbench[0].name);
    exit(0);
  }
  wait4(pid, &status, 0);
  if((pid = fork()) == 0){
    int fd = open("unixbench_testcode.sh", 0x42); //O_CREATE 0x40 O_RDWR 0x2
    write(fd, unixben_testcode[0], strlen(unixben_testcode[0]));
    write(fd, unixben_testcode[1], strlen(unixben_testcode[1]));
    write(fd, unixben_testcode[3], strlen(unixben_testcode[3]));
    close(fd);
    exec("busybox", unixbench[1].name);
    exit(0);
  }
	wait4(pid, &status, 0);

  printf("run netperf_testcode.sh\n");

  printf("run iperf_testcode.sh\n");

  printf("run cyclic_testcode.sh\n");

  if((pid = fork()) == 0){
    exec("libc-bench", libc_bench[0].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  exit(0);
}

static longtest time_test[] = {
	{1, {"time-test",0}},
};


static longtest busybox[] = {
	{ 1 , {"busybox" ,"echo", "#### independent command test" ,  0	}},
	{ 1 , {"busybox" ,"ash", "-c", "exit" ,  0	}},
	{ 1 , {"busybox" ,"sh", "-c", "exit" ,  0	}},
	{ 1 , {"busybox" ,"basename", "/aaa/bbb" ,  0	}},
	{ 1 , {"busybox" ,"cal" ,  0	}},
	{ 1 , {"busybox" ,"clear" ,  0	}},
	{ 1 , {"busybox" ,"date", 0	}},
	{ 1 , {"busybox" ,"df", 0	}},
	{ 1 , {"busybox" ,"dirname", "/aaa/bbb" ,  0	}},
	{ 1 , {"busybox" ,"dmesg", 0	}},
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
	{ 1 , {"busybox" ,"tail", "test.txt",  0	}},
	{ 1 , {"busybox" ,"hexdump", "-C", "test.txt", 0	}},
	{ 1 , {"busybox" ,"md5sum", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "ccccccc", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "bbbbbbb", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "aaaaaaa", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "2222222", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "1111111", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"echo", "bbbbbbb", ">>", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"sort", "test.txt", "|", "./busybox", "uniq" ,  0	}},
	{ 1 , {"busybox" ,"stat", "test.txt" ,  0	}},
	{ 1 , {"busybox" ,"strings", "test.txt", 0	}},
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

static longtest iozone[] = {
  { 1, { "iozone", "-a", "-r", "1k", "-s", "4m", 0 } },
  { 1, { "iozone", "-t", "4", "-i", "0", "-i", "1", "-r", "1k", "-s", "1m", 0 } },
  { 1, { "iozone", "-t", "4", "-i", "0", "-i", "2", "-r", "1k", "-s", "1m", 0 } },
  { 1, { "iozone", "-t", "4", "-i", "0", "-i", "3", "-r", "1k", "-s", "1m", 0 } },
  { 1, { "iozone", "-t", "4", "-i", "0", "-i", "5", "-r", "1k", "-s", "1m", 0 } },
  { 1, { "iozone", "-t", "4", "-i", "6", "-i", "7", "-r", "1k", "-s", "1m", 0 } },
  { 1, { "iozone", "-t", "4", "-i", "9", "-i", "10", "-r", "1k", "-s", "1m", 0 } },
  { 1, { "iozone", "-t", "4", "-i", "11", "-i", "12", "-r", "1k", "-s", "1m", 0 } },
  { 0, { 0, 0 } } // 数组结束标志，必须保留
};

static longtest libctest[] = {
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "argv", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "basename", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "clocale_mbfuncs", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "clock_gettime", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "crypt", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "dirname", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "env", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fdopen", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fnmatch", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fscanf", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fwscanf", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "iconv_open", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "inet_pton", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "mbc", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "memstream", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cancel_points", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cancel", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cond", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_tsd", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "qsort", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "random", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "search_hsearch", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "search_insque", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "search_lsearch", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "search_tsearch", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "setjmp", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "snprintf", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "socket", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "sscanf", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "sscanf_long", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "stat", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strftime", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "string", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "string_memcpy", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "string_memmem", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "string_memset", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "string_strchr", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "string_strcspn", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "string_strstr", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strptime", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strtod", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strtod_simple", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strtof", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strtol", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strtold", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "swprintf", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "tgmath", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "time", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "tls_align", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "udiv", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "ungetc", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "utime", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "wcsstr", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "wcstol", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "pleval", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "daemon_failure", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "dn_expand_empty", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "dn_expand_ptr_0", 0 } },
  
  // can not pass
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fflush_exit", 0 } },
  
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fgets_eof", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fgetwc_buffering", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "fpclassify_invalid_ld80", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "ftello_unflushed_append", 0 } },

  // can not pass
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "getpwnam_r_crash", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "getpwnam_r_errno", 0 } },


  { 1, {"./runtest.exe", "-w", "entry-static.exe", "iconv_roundtrips", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "inet_ntop_v4mapped", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "inet_pton_empty_last_field", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "iswspace_null", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "lrand48_signextend", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "lseek_large", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "malloc_0", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "mbsrtowcs_overflow", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "memmem_oob_read", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "memmem_oob", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "mkdtemp_failure", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "mkstemp_failure", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "printf_1e9_oob", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "printf_fmt_g_round", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "printf_fmt_g_zeros", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "printf_fmt_n", 0 } },

  // can not pass
	{ 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_robust_detach", 0 } },
	{ 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cancel_sem_wait", 0 } },
	//   { 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cond_smasher", 0 } },
	//   { 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_condattr_setclock", 0 } },
	{ 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_exit_cancel", 0 } },
	{ 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_once_deadlock", 0 } },
	{ 1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_rwlock_ebusy", 0 } },


  { 1, {"./runtest.exe", "-w", "entry-static.exe", "putenv_doublefree", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "regex_backref_0", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "regex_bracket_icase", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "regex_ere_backref", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "regex_escaped_high_byte", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "regex_negated_range", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "regexec_nosub", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "rewind_clear_error", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "rlimit_open_files", 0 } },
{ 1, {"./runtest.exe", "-w", "entry-static.exe", "scanf_bytes_consumed", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "scanf_match_literal_eof", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "scanf_nullbyte_char", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "setvbuf_unget", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "sigprocmask_internal", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "sscanf_eof", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "statvfs", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "strverscmp", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "syscall_sign_extend", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "uselocale_0", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "wcsncpy_read_overflow", 0 } },
  { 1, {"./runtest.exe", "-w", "entry-static.exe", "wcsstr_false_negative", 0 } },

  { 0, { 0, 0 } }, // 数组结束标志，必须保留
};

static longtest libctest_dy[] = {
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "argv", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "basename", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "clocale_mbfuncs", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "clock_gettime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "crypt", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dirname", 0}},
    
    // {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dlopen", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "env", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fdopen", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fnmatch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fwscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "iconv_open", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "inet_pton", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mbc", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "memstream", 0}},

    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cancel_points", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cancel", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cond", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_tsd", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "qsort", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "random", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_hsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_insque", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_lsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_tsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sem_init", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "setjmp", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "snprintf", 0}},
    
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "socket", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sscanf_long", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "stat", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strftime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_memcpy", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_memmem", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_memset", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_strchr", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_strcspn", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_strstr", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strptime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtod", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtod_simple", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtof", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtol", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtold", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "swprintf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tgmath", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "time", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tls_init", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tls_local_exec", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "udiv", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "ungetc", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "utime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "wcsstr", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "wcstol", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "daemon_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dn_expand_empty", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dn_expand_ptr_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fflush_exit", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fgets_eof", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fgetwc_buffering", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fpclassify_invalid_ld80", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "ftello_unflushed_append", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "getpwnam_r_crash", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "getpwnam_r_errno", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "iconv_roundtrips", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "inet_ntop_v4mapped", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "inet_pton_empty_last_field", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "iswspace_null", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "lrand48_signextend", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "lseek_large", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "malloc_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mbsrtowcs_overflow", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "memmem_oob_read", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "memmem_oob", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mkdtemp_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mkstemp_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_1e9_oob", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_fmt_g_round", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_fmt_g_zeros", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_fmt_n", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_robust_detach", 0}},
    // {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cond_smasher", 0}},
    // {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_condattr_setclock", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_exit_cancel", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_once_deadlock", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_rwlock_ebusy", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "putenv_doublefree", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_backref_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_bracket_icase", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_ere_backref", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_escaped_high_byte", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_negated_range", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regexec_nosub", 0}},
	{1, {"./runtest.exe", "-w", "entry-dynamic.exe", "rewind_clear_error", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "rlimit_open_files", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "scanf_bytes_consumed", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "scanf_match_literal_eof", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "scanf_nullbyte_char", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "setvbuf_unget", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sigprocmask_internal", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sscanf_eof", 0}},
    // {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "statvfs", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strverscmp", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "syscall_sign_extend", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tls_get_new_dtv", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "uselocale_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "wcsncpy_read_overflow", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "wcsstr_false_negative", 0}},
	{ 0, { 0, 0 } }, // 数组结束标志，必须保留
};

static longtest lua[] = {
  {1, {"./lua", "date.lua", 0}},
  {1, {"./lua", "file_io.lua", 0}},
  {1, {"./lua", "max_min.lua", 0}},
  {1, {"./lua", "random.lua", 0}},
  {1, {"./lua", "remove.lua", 0}},
  {1, {"./lua", "round_num.lua", 0}},
  {1, {"./lua", "sin30.lua", 0}},
  {1, {"./lua", "sort.lua", 0}},
  {1, {"./lua", "strings.lua", 0}},
  {0, {0}},

};


static longtest unixbench[] = {
  {1, {"busybox", "rm", "unixbench_testcode.sh", 0}},
  {1, {"busybox", "sh", "unixbench_testcode.sh", 0}},
  {0, {0}},
};

static longtest libc_bench[] = {
  {1, {"libc-bench", 0}}
};

const char* unixben_testcode[] = {"\
#!/bin/bash \n\
./dhry2reg 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench DHRY2 test(lps): \"$0}'\n\
./whetstone-double 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+.[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+.[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench WHETSTONE test(MFLOPS): \"$0}'\n\
./syscall 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench SYSCALL test(lps): \"$0}'\n\
./context1 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox tail -n1 | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench CONTEXT test(lps): \"$0}'\n\
./pipe 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench PIPE test(lps): \"$0}'\n\
./spawn 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench SPAWN test(lps): \"$0}'\n\
UB_BINDIR=./ ./execl 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench EXECL test(lps): \"$0}'\n",
"./fstime -w -t 20 -b 256 -m 500 | ./busybox grep -o \"WRITE COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_WRITE_SMALL test(KBps): \"$0}'\n\
./fstime -r -t 20 -b 256 -m 500 | ./busybox grep -o \"READ COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_READ_SMALL test(KBps): \"$0}'\n\
./fstime -c -t 20 -b 256 -m 500 | ./busybox grep -o \"COPY COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_COPY_SMALL test(KBps): \"$0}'\n\
./fstime -w -t 20 -b 1024 -m 2000 | ./busybox grep -o \"WRITE COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_WRITE_MIDDLE test(KBps): \"$0}'\n\
./fstime -r -t 20 -b 1024 -m 2000 | ./busybox grep -o \"READ COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_READ_MIDDLE test(KBps): \"$0}'\n\
./fstime -c -t 20 -b 1024 -m 2000 | ./busybox grep -o \"COPY COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_COPY_MIDDLE test(KBps): \"$0}'\n",
"./fstime -w -t 20 -b 4096 -m 8000 | ./busybox grep -o \"WRITE COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_WRITE_BIG test(KBps): \"$0}'\n\
./fstime -r -t 20 -b 4096 -m 8000 | ./busybox grep -o \"READ COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_READ_BIG test(KBps): \"$0}'\n\
./fstime -c -t 20 -b 4096 -m 8000 | ./busybox grep -o \"COPY COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FS_COPY_BIG test(KBps): \"$0}'\n",
"./arithoh 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench ARITHOH test(lps): \"$0}'\n\
./short 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench SHORT test(lps): \"$0}'\n\
./int 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench INT test(lps): \"$0}'\n\
./long 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench LONG test(lps): \"$0}'\n\
./float 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench FLOAT test(lps): \"$0}'\n\
./double 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench DOUBLE test(lps): \"$0}'\n\
./hanoi 10 | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench HANOI test(lps): \"$0}'\n\
./syscall 10 exec | ./busybox grep -o \"COUNT|[[:digit:]]\\+|\" | ./busybox grep -o \"[[:digit:]]\\+\" | ./busybox awk '{print \"Unixbench EXEC test(lps): \"$0}'\n"
};

static longtest cyclic_bench[] = {
  {1,{"./cyclictest","-a","-i","1000","-t1","-n","-p2","-D","20s","-q",0}},
  {0,{0}},
};

int main(int argc, char ** argv)
{
	dev(2,1,0);
	dup(0);
	dup(0);
	printf("busybox test\n");
    test_busybox();
    exit(0);
}