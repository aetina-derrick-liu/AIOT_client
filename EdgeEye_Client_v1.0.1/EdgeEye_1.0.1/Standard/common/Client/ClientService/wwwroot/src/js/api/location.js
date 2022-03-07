var settings = {
		"url": "SettingAPI/GetDeviceLocation",
		"method": "GET",
		"cache": false
	}

$(document).ready(function(){
	$.ajax(settings).done(function (response) {
		var obj = $.parseJSON(response);
		document.getElementById("LongitudeId").value = obj[0]["Longitude"];
		document.getElementById("LatitudeId").value = obj[0]["Latitude"];
	});
});

function submit_click() 
{	
	var submit_settings = {
		"async": true,
		"crossDomain": true,
		"url": "SettingAPI/SetDeviceLocation",
		"method": "POST",
		"headers": {
			"BranchId": "1",
			"Longitude": document.getElementById("LongitudeId").value,
			"Latitude": document.getElementById("LatitudeId").value,
		}
	}

	$.ajax(submit_settings).done(function (response){
		document.getElementById("popmsg").style = "";
	});
};