//文件名 jh_base_sim.cpp 模拟机器人
//编译  g++ -o jh_base_sim jh_base_sim.cpp -lhiredis -lm
#include <linux/joystick.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <time.h>
class jh_base{
    private:
	//redis服务器参数
	char redisIp[30];
	int redisPort;
	//速度控制参数:线速度\角速度
	float speedl,speeda;
	int intSim; //为1表示为虚拟机器人，0为实际机器人
	void base_init(){
		redis_init("127.0.0.1",6379);
		pub_delCmd();
		int intSim =1;
		sim_operation();
	}
	void redis_init(char *ip,int port){
		strcpy(redisIp,ip);
		redisPort=port;
	}
	//检查有命令的删除
	void pub_delCmd(){
		char strCmd[200];
		redisContext* conn=pub_redisConnect(); //打开链接
		sprintf(strCmd,"del jh_cmd_base");
		redisReply* r1 = (redisReply*)redisCommand(conn,strCmd);
		freeReplyObject(r1);
		sprintf(strCmd,"del jh_cmd_pos");
		redisReply* r2 = (redisReply*)redisCommand(conn,strCmd);
		freeReplyObject(r2);
		pub_redisClose(conn);	//关闭链接
	}
	
    public:
	float rx,ry,rth;//当前位置
	jh_base(int a){
		base_init();
		speedl=0;speeda=0;
		rx=0;ry=0;rth=0;
	}
	//虚拟机运行
	void sim_operation(){	//机器人模拟运行
		float speed,vth;
		static struct timeval tv;
		struct timeval tv1;
		gettimeofday(&tv1,NULL);
		if(tv1.tv_sec < tv.tv_sec || (speedl==0 && speeda==0)){gettimeofday(&tv,NULL);return;}
		if(intSim ==1) return;
		//计算机器人坐标值
		double dt=tv1.tv_usec - tv.tv_usec;
		speed =speedl;vth=speeda;
		gettimeofday(&tv,NULL);
		if(dt < 0) dt += 1000000;
		dt /= 1000000;
		double delta_x=(speed*cos(rth)-0*sin(rth))*dt;
		double delta_y=(speed*sin(rth)+0*cos(rth))*dt;
		double delta_th=vth * dt;
		rx += delta_x;
		ry += delta_y;
		rth+=delta_th;
		if(rth>6.29)rth -=6.2831852;
		if(rth<-6.29)rth+=6.2831852;
	}
	redisContext* pub_redisConnect(){
		redisContext* conn =redisConnect(redisIp,redisPort);
		return conn;
	}
	void pub_redisClose(redisContext* conn){
		redisFree(conn);
	}
	//从redis读入命令
	void pub_get_cmd(redisContext* conn){
		char strCmd[200];
		sprintf(strCmd,"exists jh_cmd_base");
		redisReply* r1 = (redisReply*)redisCommand(conn,strCmd);
		if(r1->integer>0){
			sprintf(strCmd,"hget jh_cmd_base speedl");
			redisReply* ra1 = (redisReply*)redisCommand(conn,strCmd);
			sscanf(ra1->str,"%f",&speedl);
			freeReplyObject(ra1);
			sprintf(strCmd,"hget jh_cmd_base speeda");
			redisReply* ra2 = (redisReply*)redisCommand(conn,strCmd);
			sscanf(ra2->str,"%f",&speeda);
			freeReplyObject(ra2);
			sprintf(strCmd,"del jh_cmd_base");
			redisReply* ra3 = (redisReply*)redisCommand(conn,strCmd);
			freeReplyObject(ra3);
		}
		freeReplyObject(r1);
		sprintf(strCmd,"exists jh_cmd_pos");
		redisReply* r2 = (redisReply*)redisCommand(conn,strCmd);
		if(r2->integer>0){
			sprintf(strCmd,"hget jh_cmd_pos x");
			redisReply* ra1 = (redisReply*)redisCommand(conn,strCmd);
			sscanf(ra1->str,"%f",&rx);
			freeReplyObject(ra1);
			sprintf(strCmd,"hget jh_cmd_pos y");
			redisReply* ra2 = (redisReply*)redisCommand(conn,strCmd);
			sscanf(ra2->str,"%f",&ry);
			freeReplyObject(ra2);
			sprintf(strCmd,"hget jh_cmd_pos th");
			redisReply* ra3 = (redisReply*)redisCommand(conn,strCmd);
			sscanf(ra3->str,"%f",&rth);
			freeReplyObject(ra3);
			sprintf(strCmd,"del jh_cmd_pos");
			redisReply* ra4 = (redisReply*)redisCommand(conn,strCmd);
			freeReplyObject(ra4);
		}
		freeReplyObject(r2);
	}
	//上传机器人位置
	void pub_robot_grid(redisContext* conn){
		char strCmd[200];
		if(conn->err){redisFree(conn);return;}
		int i;
		for(i=0;i<3;i++){
			switch(i){
				case 0:sprintf(strCmd,"hset %s lx %f","jh_bot",rx);break;
				case 1:sprintf(strCmd,"hset %s ly %f","jh_bot",ry);break;
				case 2:sprintf(strCmd,"hset %s lth %f","jh_bot",rth);break;
			}
			redisReply* r = (redisReply*)redisCommand(conn,strCmd);
			freeReplyObject(r);
		}
	}
};
int main(){
	jh_base jhbase(0);
	printf("jh_base is started \n");
	while(1){
		jhbase.sim_operation();
		redisContext* conn=jhbase.pub_redisConnect();
		jhbase.pub_get_cmd(conn);
		jhbase.pub_robot_grid(conn);
		jhbase.pub_redisClose(conn);
		usleep(10000);
	}
}
