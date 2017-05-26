//jh_joy.cpp
//编译  g++ -o jh_joy jh_joy.cpp -lhiredis -lm
#include <linux/joystick.h>
#include <unistd.h>
//#include <math.h>
#include <fcntl.h>
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
class jh_joy{
	private:
		int joy_fd;
		fd_set set;
		struct timeval tv;
		js_event event;
		int status1;	//按键状态
		//遥控状态速度组合
		float conspeed[32]={0.5, 0, 0.5, -0.3,  0,  0,  0.5,  0.3,
				0.3, 0, 0.3, -0.3,  0,  0,  0.3,  0.3,
				0.2,0,	0.2,-0.3,	0,0,	0.2,0.3,
				0,0,	0,0,	-0.3,0,	0,0};	//倒档
		char redisIp[30];
		int redisPort;
		void redis_init(char *ip,int port){
			strcpy(redisIp,ip);
			redisPort=port;
		}
		redisContext* pub_redisConnect(){
			redisContext* conn =redisConnect(redisIp,redisPort);
			return conn;
		}
		void pub_redisClose(redisContext* conn){
			redisFree(conn);
		}
	public:
		jh_joy(int a){
			status1=0;
			redis_init("127.0.0.1",6379);
			while(1){
				joy_fd = open("/dev/input/js0", O_RDONLY);
				if (joy_fd != -1){
					close(joy_fd);
					joy_fd = open("/dev/input/js0", O_RDONLY);
        			}
				if (joy_fd != -1)return;
				printf("Couldn't open joystick\n");
				sleep(1);
			}
		}
		~jh_joy(){close(joy_fd);}
		void joyrun(){
			char strCmd[200];
			FD_ZERO(&set);
        		FD_SET(joy_fd, &set);
			int select_out = select(joy_fd+1, &set, NULL, NULL, &tv);
			if (select_out != -1)return;
			if (FD_ISSET(joy_fd, &set)){
			if (read(joy_fd, &event, sizeof(js_event)) == -1)return;//操纵杆很可能关闭。肯定会发生。
			switch(event.type)
			{
				case JS_EVENT_BUTTON | JS_EVENT_INIT:	//软件打开时触发,每个按键都返回一个0
					//printf("joy_node1: time=%u,  type=%Xh, number=%d, value=%d\n",
					//		event.time, event.type, event.number, event.value);
					break;
				case JS_EVENT_AXIS | JS_EVENT_INIT:	//软件打开时触发,每个轴都返回一个0
					//printf("joy_node2: time=%u,  type=%Xh, number=%d, value=%d\n",
					//		event.time, event.type, event.number, event.value);
					break;
				case JS_EVENT_BUTTON:	//按键触发按下时(event.value=1)释放时(event.value=0)
					if(event.value !=0){
						if(event.type !=1)break;
						if(event.number>=4 && event.number<=7){status1=event.number-4;break;}
						if(event.number>=0 && event.number<=3){
							//printf(":speedl= %f, speeda=%f\n",conspeed[status1*8+event.number*2],
							//	conspeed[status1*8+event.number*2+1]);
							//****************************************		
							redisContext* conn=pub_redisConnect(); //打开链接
							sprintf(strCmd,"hset jh_cmd_base speedl %f",conspeed[status1*8+event.number*2]);
							redisReply* r1 = (redisReply*)redisCommand(conn,strCmd);
							freeReplyObject(r1);
							sprintf(strCmd,"hset jh_cmd_base speeda %f",conspeed[status1*8+event.number*2+1]);
							redisReply* r2 = (redisReply*)redisCommand(conn,strCmd);
							freeReplyObject(r2);
							pub_redisClose(conn);	//关闭链接
							//****************************************
						}
					}
					break;
				case JS_EVENT_AXIS:
					if(event.value !=0){
						//printf("joy_node4: time=%u,  type=%Xh, number=%d, value=%d\n",
						//event.time, event.type, event.number, event.value);
					}
					break;
          			default:
            				//printf("joy_node5: time=%u,  type=%Xh, number=%d, value=%d\n",
					//		event.time, event.type, event.number, event.value);
            				break;
			}}
		}	
};
int main(){
	jh_joy joy1(1);
	while(1){
		joy1.joyrun();
		usleep(10000);
	}
	return 0;
}
