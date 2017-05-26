function getCurrTime(){
	var day="";
	var month="";
	var ampm="上午 ";
	var ampmhour="";
	var ampmminute="";
	var ampmsecond=""
	var myweekday="";
	var year="";
	mydate=new Date();
	myweekday=mydate.getDay();
	mymonth=mydate.getMonth()+1;
	myday= mydate.getDate();
	myyear= mydate.getYear();
	ampmhour = mydate.getHours();
	ampmminute=mydate.getMinutes();
	ampmsecond=mydate.getSeconds();
	year=(myyear > 200) ? myyear : 1900 + myyear;
	if(myweekday == 0)	weekday=" 星期日 ";
	else if(myweekday == 1)weekday=" 星期一 ";
	else if(myweekday == 2)weekday=" 星期二 ";
	else if(myweekday == 3)weekday=" 星期三 ";
	else if(myweekday == 4)weekday=" 星期四 ";
	else if(myweekday == 5)weekday=" 星期五 ";
	else if(myweekday == 6)weekday=" 星期六 ";
	if(ampmhour<8)ampm = "凌晨 ";
	else if(ampmhour<11)ampm = "上午 ";
	else if(ampmhour <13)ampm = "中午 ";
	else if(ampmhour <17){ampm = "下午 ";	ampmhour = ampmhour - 12;}
	else if(ampmhour <20){ampm = "晚上 ";ampmhour = ampmhour - 12;}
	else{ampm = "夜里 ";ampmhour = ampmhour - 12;}	
	//var retstr='现在是 ' +  year+'-'+mymonth+'-'+myday+' '+weekday + ' ';
	var retstr='现在是 ' +  year+'-'+mymonth+'-'+myday + ' ';
	retstr = retstr + ampm + ampmhour + ':'+ ampmminute + ':' + ampmsecond;
	return retstr;
	}