//文件名 jh_base_1.cpp 平衡车机器人底座
//编译  g++ -o jh_base_1 jh_base_1.cpp -lhiredis -lm
//机器人地盘左右轮编码器为每转90脉冲,轮直径190毫米,轮间距330毫米
//每脉冲距离为: PI*190/90=6.63
#include <linux/joystick.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
class jh_base{
    private:
	static const float PULSE_EQU = M_PI* 0.190 / 90;	//每脉冲距离 单位米
	static const float WHEELSPACING = 0.33;		//轮间距 单位米
	int comFile;	//打开的串口文件号fd
	//redis服务器参数
	char redisIp[30];
	int redisPort;
	//速度控制参数:线速度\角速度
	float speedl,speeda;
	//向机器人设置的左右轮速度,从speedl,speeda转换而来
	int speedSetL,speedSetR;
	//发送状态:
	int sendStatus;	//0自由状态,1发送左速度,2发送右速度,3.读数据
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
	//把线速度/角速度转换为左右轮脉冲数
	void speedTransform(){
		//计算左右轮速度
		float sl,sr;
		sr = speedl + speeda * WHEELSPACING/2;
		sl = speedl - speeda * WHEELSPACING/2;
		//转换为0.1秒的脉冲数
		speedSetL = sl / PULSE_EQU/10;
		speedSetR = sr / PULSE_EQU/10;
		sendStatus =1;	//设置进入发送状态
		//printf("%f  %f  %d  %d  \n",sl,sr,speedSetL,speedSetR);
	}
	//设置串口BPS
	void set_port_bps(int fd,int bps){
		struct termios options;
		int paramBms;
		switch(bps){
			case 300:paramBms=B300;break;
			case 600:paramBms=B600;break;
			case 1200:paramBms=B1200;break;
			case 2400:paramBms=B2400;break;
			case 4800:paramBms=B4800;break;
			case 9600:paramBms=B9600;break;
			case 19200:paramBms=B19200;break;
			case 38400:paramBms=B38400;break;
			case 57600:paramBms=B57600;break;
			case 115200:paramBms=B115200;break;
			defatlt:paramBms=B9600;break;
		}
	
		tcgetattr(fd, &options);
		cfsetispeed(&options, paramBms);
		cfsetospeed(&options, paramBms);
		options.c_cflag |= (CLOCAL | CREAD);
		options.c_cflag &= ~PARENB;	//设在n 一行
		options.c_cflag &= ~CSTOPB;	//设置一位停止位
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;		//设在CS8或者CS7

		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//不经处理直接发送。
		tcsetattr(fd, TCSANOW, &options);
	}
	//打开串口
	int open_port(int bps)
	{
		int fd;//O_NOCTTY不成为控制终端 O_NDELAY不关心DCD状态 O_NDELAY|O_NONBLOCK
		int flags;	
		fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY |O_NDELAY);
		if (fd == -1){
			return -1;
		}else{
			set_port_bps(fd,bps);
			flags = fcntl(fd, F_GETFL, 0);   //获取文件的flags值
			fcntl(fd, F_SETFL, flags | O_NONBLOCK); //设置成非阻塞模式
			return (fd);
		}
	}
    public:
	float rx,ry,rth;//当前位置
	jh_base(int a){
		base_init();
		speedl=0;speeda=0;
		rx=0;ry=0;rth=0;
		sendStatus=0;	//串口发送状态
		while(true){	//打开串口
			comFile = open_port(9600);
			if(comFile==-1){
				printf("串口打开失败\n");
				sleep(3);
			}else{
				break;
			}
		}
	}
	~jh_base(){
		if(comFile!=-1)close(comFile);
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
			speedTransform();
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
	//串口通讯命令 20毫秒调用一次
	void serialTx(void){
		char bufRcv[200];
		char bufSend[200];
		if(comFile ==-1)return;
		char rcvLen =read(comFile,bufRcv,200);
		if(rcvLen>0){
			bufRcv[rcvLen]=0;
			printf("rcv: %s \n",bufRcv);
		}
		switch(sendStatus){
			case 1:sprintf(bufSend,"{\"L\",\"%d\"}",speedSetL);sendStatus++;break;
			case 2:sprintf(bufSend,"{\"R\",\"%d\"}",speedSetR);sendStatus++;break;
			//case 0:
			case 3:sprintf(bufSend,"D");sendStatus=0;break;
			default:return;
		}
		int i1 =0;
		while(bufSend[i1]!=0)i1++;
		write(comFile,bufSend,i1);
		printf("sed: %s \n",bufSend);
	}
};
int main(){
	jh_base jhbase(0);
	printf("jh_base_1 is started \n");
	while(1){
		//jhbase.sim_operation();
		redisContext* conn=jhbase.pub_redisConnect();
		jhbase.pub_get_cmd(conn);
		jhbase.pub_robot_grid(conn);
		jhbase.pub_redisClose(conn);
		jhbase.serialTx();
		usleep(30000);
	}
}
