<?php
	function myTest() {
		static $x=0;
		echo $x;
		$x ++;
	}	
	switch($_REQUEST['type']){
		case 1:
			//$_REQUEST  $_POST  $_GET
			echo "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
			echo "我的第一段 PHP 脚本！"."<br />";
			echo $_SERVER['PHP_SELF']."<br />";
			echo $_SERVER['REMOTE_ADDR']."<br />";
			echo $_SERVER['REMOTE_PORT'];
			echo "2017/4/30";
			break;
		case 2:	$add1 = $_REQUEST['dat1'];
			$add2 = $_REQUEST['dat2'];
			$add3 = $add1 + $add2;
			echo $add3;
			break;
		case 3:	$add1 = $_REQUEST['dat1'];
			$add2 = $_REQUEST['dat2'];
			$add3 = $add1 - $add2;
			echo $add3;
			break;
		case 4:myTest();
			myTest();myTest();myTest();
			break;
	}
?>
