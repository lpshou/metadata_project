/**
 * @file namespace_api.c
 * @brief 
 * @author lpshou.liu
 * @version v1.0.0
 * @date 2013-07-26
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper.h>
#include <zookeeper_log.h>
#include "common.h"
int flag_user_exist = 0;
int flag_user_create = 0;
int flag_user_delete = 0;
void zktest_watcher_g(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
	printf("==========================================\n");
    printf("Something happened....\n");
    printf("type: %d\n", type);
    printf("state: %d\n", state);
    printf("path: %s\n", path);
    printf("watcherCtx: %s\n", (char *)watcherCtx);
	printf("==========================================\n");
}

void zktest_dump_stat(const struct Stat *stat)
{
    char tctimes[40];
    char tmtimes[40];
    time_t tctime;
    time_t tmtime;

    if (!stat) {
        fprintf(stderr,"null\n");
        return;
    }
    tctime = stat->ctime/1000;
    tmtime = stat->mtime/1000;
       
    ctime_r(&tmtime, tmtimes);
    ctime_r(&tctime, tctimes);
   	printf("==========================================\n");
   
    fprintf(stderr, "\tctime = %s\tczxid=%llx\n"
    "\tmtime=%s\tmzxid=%llx\n"
    "\tversion=%x\n\taversion=%x\n"
    "\tephemeralOwner = %llx\n",
     tctimes, stat->czxid,
     tmtimes, stat->mzxid,
    (unsigned int)stat->version, 
	(unsigned int)stat->aversion,
    stat->ephemeralOwner);
}

/**
 * @brief 判断用户是否存在的回调函数
 *
 * @param rc
 * @param stat
 * @param data
 *
 * 利用全局变量记录创建用户的返回状态
 */
void user_exist_stat_completion(int rc, const struct Stat *stat, const void *data)
{
	//fprintf(stderr, "%s: rc = %d\n", (char*)data, rc);
    //zktest_dump_stat(stat);
	if(rc == ZOK)
		flag_user_exist = SUCCEED_STATUS;
	else
		flag_user_exist = FAILED_STATUS;
}

/**
 * @brief user_create_string_completion 创建用户的回调函数
 *
 * @param rc
 * @param name
 * @param data
 * 利用全局变量记录创建用户的返回状态
 */
void user_create_string_completion(int rc, const char *name, const void *data)
{
    //fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
    //if (!rc) {
    //    fprintf(stderr, "\tname = %s\n", name);
    //}
	if(rc == ZOK)
		flag_user_create = SUCCEED_STATUS;
	else if(rc == ZNODEEXISTS)
		flag_user_create = FAILED_STATUS;
	else
		flag_user_create = OTHER_STATUS;
}

/**
 * @brief user_delete_void_completion 删除用户的回调函数
 *
 * @param rc
 * @param data
 * 利用全局变量flag_user_delete记录删除用户的返回状态
 */
void user_delete_void_completion(int rc, const void *data)
{
    fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
	//printf("rc: %d\n",rc);
	if(rc == ZOK)
		flag_user_delete = SUCCEED_STATUS;
	else
		flag_user_delete = FAILED_STATUS;
	printf("flag_user_delete: %d\n",flag_user_delete);
}


/**
 * @brief user_exist：判断用户是否存在
 *
 * @param userName：用户名
 *
 * @return 用户存在返回0 用户不存在返回-1 
 */
int user_exist(const char *userName){
	int ret;
	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	zhandle_t* zkhandle = zookeeper_init(host, zktest_watcher_g, timeout, 0, "for user exist", 0);
	if(zkhandle == NULL){
		fprintf(stderr,"Error when connecting to zookeeper server...\n");
		exit(EXIT_FAILURE);
	}
	//在用户名之前加上'/'
	int len = strlen(userName);
    char *user_name=(char *)malloc(len+2);
	sprintf(user_name,"/%s",userName);
	user_name[len+1]='\0';
	printf("name:%s\n",user_name);
	//判断用户名为userName的用户是否存在
	ret = zoo_aexists(zkhandle, user_name, 0, user_exist_stat_completion, userName);
    if (ret) {
        fprintf(stderr, "Error %d for %s\n", ret, "aexists");
        exit(EXIT_FAILURE);
    }
	sleep(1);
	if(flag_user_exist == SUCCEED_STATUS)
	{
		printf("user:%s exist!\n",userName);
		ret = 0;
	}
	else
	{
		printf("user:%s not exist!\n",userName);
		ret = -1;
	}
	free(user_name);
	// Wait for asynchronous zookeeper call done.
    // getchar();
	// Wait 2s for asynchronous zookeeper call done./
	sleep(1);
    zookeeper_close(zkhandle);
	return ret;
}



/**
 * @brief user_create 创建新用户
 *
 * @param userName
 *
 * @return  创建新用户成功返回0，用户已经存在返回-1，创建用户失败返回-2
 */
int user_create(const char *userName)
{
	int ret,len;
	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	zhandle_t* zkhandle = zookeeper_init(host, zktest_watcher_g, timeout, 0, "for user_create", 0);
	if(zkhandle == NULL){
		fprintf(stderr,"Error when connecting to zookeeper server...\n");
		exit(EXIT_FAILURE);
	}
	len = strlen(userName);
	char *user_name = (char *)malloc(len+2);
	sprintf(user_name,"/%s",userName);
	user_name[len+1]='\0';
	//printf("name:%s\n",user_name);

    ret = zoo_acreate(zkhandle, user_name, "userName", strlen("userName"), &ZOO_OPEN_ACL_UNSAFE, 0 /* ZOO_SEQUENCE */,
           user_create_string_completion, userName);

	sleep(1);

	if(flag_user_create == SUCCEED_STATUS )
		printf("create user: %s succeed!\n",userName);
	else if(flag_user_create == FAILED_STATUS)
		printf("user: %s already exist!\n",userName);
	else
		printf("create user: %s failed!\n",userName);
	free(user_name);
	// Wait for asynchronous zookeeper call done.
    // getchar();
	// Wait 2s for asynchronous zookeeper call done./
	
 	sleep(1);
	zookeeper_close(zkhandle);

	return flag_user_create;
}

/**
 * @brief 删除用户
 *
 * @param userName
 *
 * @return 删除用户成功返回0， 删除用户失败返回-1
 */
int user_delete(const char *userName)
{
	int ret,len;
	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	zhandle_t* zkhandle = zookeeper_init(host, zktest_watcher_g, timeout, 0, "for user_delete", 0);
	if(zkhandle == NULL){
		fprintf(stderr,"Error when connecting to zookeeper server...\n");
		exit(EXIT_FAILURE);
	}
	len = strlen(userName);
	char *user_name = (char *)malloc(len+2);
	sprintf(user_name,"/%s",userName);
	user_name[len+1]='\0';
	printf("name:%s\n",user_name);


    zoo_adelete(zkhandle, user_name, -1, user_delete_void_completion, "adelete");
    // Wait for asynchronous zookeeper call done.
    //getchar();
	sleep(1);
    zookeeper_close(zkhandle);
	return flag_user_delete;
}

/**
 * @brief main
 *
 * @param argc
 * @param argv[]
 *
 * @return 0
 */
int main(int argc, const char *argv[])
{
    //user_exist("abc");
    //user_exist("efg");
	//	user_create("abc");
	int k;
	k = user_delete("abc");
	printf("return: %d\n",k);
  
	//设置日志等级
//	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
//    
//	zhandle_t* zkhandle = zookeeper_init(host, zktest_watcher_g, timeout, 0, "hello zookeeper.", 0);
//    if (zkhandle == NULL) {
//        fprintf(stderr, "Error when connecting to zookeeper servers...\n");
//        exit(EXIT_FAILURE);
//    }
//
//
//    struct ACL ALL_ACL[] = {{ZOO_PERM_ALL, ZOO_ANYONE_ID_UNSAFE}} ; 
//     struct ACL_vector ALL_PERMS = {1, ALL_ACL};

//    ret = zoo_adelete(zkhandle, "/xyz", -1, zktest_void_completion, "adelete");
//    if (ret) {
//        fprintf(stderr, "Error %d for %s\n", ret, "adelete");
//        exit(EXIT_FAILURE);
//    }
//    // Wait for asynchronous zookeeper call done.
//    getchar();
//
//    zookeeper_close(zkhandle);
}

