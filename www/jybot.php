<?php
	$redisAddr='127.0.0.1';
	$rediaPort=6379;	
	switch($_REQUEST['type']){
		case 4:	$redis = new Redis();
			$redis->connect($redisAddr,$rediaPort);
			//"jh_bml_val":{"mileage":-661,"speed":-89.633842}
			$jh_bml_val = "\"jh_bml_val\":{\"mileage\":".$redis->hget("jh_bml","mileage").
				",\"speed\":".$redis->hget("jh_bml","speed")."}";
			//"jh_bmr_val":{"mileage":-1458,"speed":6.937651}
			$jh_bmr_val = "\"jh_bmr_val\":{\"mileage\":".$redis->hget("jh_bmr","mileage").
				",\"speed\":".$redis->hget("jh_bmr","speed")."}";
			echo "{".$jh_bml_val.",".$jh_bmr_val."}";
			break;
		case 5: $redis = new Redis();
			$redis->connect($redisAddr,$rediaPort);
			//"jh_bml_conf":{"mode":"1","pa":"37","pb":"35"}
			$jh_bml_conf="\"jh_bml_conf\":{\"mode\":\"".$redis->hget("jh_bml","mode").
				"\",\"pa\":\"".$redis->hget("jh_bml","pa").
				"\",\"pb\":\"".$redis->hget("jh_bml","pb")."\"}";
			//"jh_bmr_conf":{"mode":"1","pa":"38","pb":"40"}
			$jh_bmr_conf="\"jh_bmr_conf\":{\"mode\":\"".$redis->hget("jh_bmr","mode").
				"\",\"pa\":\"".$redis->hget("jh_bmr","pa").
				"\",\"pb\":\"".$redis->hget("jh_bmr","pb")."\"}";
			//"jh_motol_conf":{"pa":"33","pb":"31","pwm":"29"}
			$jh_motol_conf="\"jh_motol_conf\":{\"pa\":\"".$redis->hget("jh_ml","pa").
				"\",\"pb\":\"".$redis->hget("jh_ml","pb").
				"\",\"pwm\":\"".$redis->hget("jh_ml","pwm")."\"}";
			//"jh_motor_conf":{"pa":"33","pb":"31","pwm":"29"}
			$jh_motor_conf="\"jh_motor_conf\":{\"pa\":\"".$redis->hget("jh_mr","pa").
				"\",\"pb\":\"".$redis->hget("jh_mr","pb").
				"\",\"pwm\":\"".$redis->hget("jh_mr","pwm")."\"}";
			
			//"jh_bot_conf":{"wheelspacing":"0.450000","unitdistance":"0.000248",
			//	"acc_lim_th":"3.200000","acc_lim_x":"2.500000",
			//	"acc_lim_y":"2.500000","max_vel_x":"0.651000",
			//	"min_vel_x":"0.100000","max_rotation_vel":"1.000000",
			//	"min_inplace_vel":"0.400000","escape_vel":"-0.100000",
			//	"xy_goal_tolerance":"0.100000","yaw_goal_tolerance":"0.050000"}
			$jh_bot_conf="\"jh_bot_conf\":{\"wheelspacing\":\"".$redis->hget("jh_bot","wheelspacing").
				"\",\"unitdistance\":\"".$redis->hget("jh_bot","unitdistance").
				"\",\"acc_lim_th\":\"".$redis->hget("jh_bot","acc_lim_th").
				"\",\"acc_lim_x\":\"".$redis->hget("jh_bot","acc_lim_x").
				"\",\"acc_lim_y\":\"".$redis->hget("jh_bot","acc_lim_y").
				"\",\"max_vel_x\":\"".$redis->hget("jh_bot","max_vel_x").
				"\",\"min_vel_x\":\"".$redis->hget("jh_bot","min_vel_x").
				"\",\"max_rotation_vel\":\"".$redis->hget("jh_bot","max_rotation_vel").
				"\",\"min_inplace_vel\":\"".$redis->hget("jh_bot","min_inplace_vel").
				"\",\"escape_vel\":\"".$redis->hget("jh_bot","escape_vel").
				"\",\"xy_goal_tolerance\":\"".$redis->hget("jh_bot","xy_goal_tolerance").
				"\",\"yaw_goal_tolerance\":\"".$redis->hget("jh_bot","yaw_goal_tolerance")."\"}";
			echo "{".$jh_bml_conf.",".$jh_bmr_conf.",".$jh_motol_conf.",".$jh_motor_conf.
				",".$jh_bot_conf."}";
			break;
		case 6:	//设置左右编码器方向 仅mod3有效
			$redis = new Redis();
			$redis->connect($redisAddr,$rediaPort);
			$dirL = $_REQUEST['jh_bml_dir'];
			$dirR = $_REQUEST['jh_bmr_dir'];
			$redis->set("jh_bml_dir",$dirL);
			$redis->set("jh_bmr_dir",$dirR);
			echo "{\"ret\":\"ok\"}";
			break;
		case 7:	//设置运行速度
			$speedl = $_REQUEST['speedl'];
			$speeda=$_REQUEST['speeda'];
			$redis = new Redis();
			$redis->connect($redisAddr,$rediaPort);
			$redis->hset("jh_cmd_base","speedl",$speedl);
			$redis->hset("jh_cmd_base","speeda",$speeda);
			echo "{\"ret\":\"ok\"}";
			break;
		case 8:	//获取机器人当前位置{"lx":"12","ly":"13","lth":"0.12"}
			$redis = new Redis();
			$redis->connect($redisAddr,$rediaPort);
			$strRet = "{\"lx\":".$redis->hget("jh_bot","lx").",\"ly\":"
				.$redis->hget("jh_bot","ly").",\"lth\":"
				.$redis->hget("jh_bot","lth")."}";
			echo $strRet;
			break;
		case 9://设置机器人当前位置
			$posX = $_REQUEST['x'];
			$posY = $_REQUEST['y'];
			$posTh = $_REQUEST['th'];
			$redis = new Redis();
			$redis->connect($redisAddr,$rediaPort);
			$redis->hset("jh_cmd_pos","x",$posX);
			$redis->hset("jh_cmd_pos","y",$posY);
			$redis->hset("jh_cmd_pos","th",$posTh);
			echo "{\"ret\":\"ok\"}";
			break;
		default:
			echo "这是上海惊鸿机器人有限公司机器人系统API";
			break;
	}
?>
