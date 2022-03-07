function showTime(){
	var today = new Date();
	var hour=
	parseInt(today.getHours())<10 
	? '0'+today.getHours() 
	: today.getHours();

	var min=
	parseInt(today.getMinutes())<10 
	? '0'+today.getMinutes() 
	: today.getMinutes();

	var sec=
	parseInt(today.getSeconds())<10 
	? '0'+today.getSeconds() 
	: today.getSeconds();

	document.getElementById('nowtime').innerHTML=hour+':'+min+':'+sec
}
window.onload=function(){
	setInterval(showTime,1000);
	var today = new Date();
	document.getElementById('nowdate').textContent=
		today.getFullYear()+'.'+
		(today.getMonth()+1)+'.'+
		today.getDate()
		//+'.'+today.getDay()
}