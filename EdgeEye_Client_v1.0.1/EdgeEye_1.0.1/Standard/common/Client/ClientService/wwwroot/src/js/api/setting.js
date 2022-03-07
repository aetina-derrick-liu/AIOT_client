import setting_device from "../../setting-device.html";
import setting_location from "../../setting-location.html";
import setting_threshold from "../../setting-threshold.html";

var option_text = ['=', '&lt;', '&gt;', '&ne;'];

$(document).ready(function(){
	
});

$(document).on("binding-setting-btn", function(){
	$(".dev-btn").bind("click", TriggerDeviceSetting);
	$(".loc-btn").bind("click", TriggerLocationSetting);
	$(".thre-btn").bind("click", TriggerThresholdSetting);
});

$(document).on("reload-setting-device", function()
{
	document.getElementById("button-dev").setAttribute("class", "fa fa-hdd-o active");
	document.getElementById("button-loc").setAttribute("class", "fa fa-location-arrow");
	document.getElementById("button-thre").setAttribute("class", "fa fa-line-chart");
	document.getElementById("setting-icon").setAttribute("class", "fa fa-hdd-o fa-5x");
	document.getElementById("setting-title").innerHTML = '<b>Device</b>';
	document.getElementById("setting-content").innerHTML = 
							'<p>' +
								'You can modify device setting at this page.' +
							'</p>';
	document.getElementById("setting-main").innerHTML = setting_device;
	LoadDeviceSetting();
	$(".submit-btn").bind("click", SubmitDeviceSetting);
	
});

$(document).on("reload-setting-location", function()
{
	document.getElementById("button-dev").setAttribute("class", "fa fa-hdd-o");
	document.getElementById("button-loc").setAttribute("class", "fa fa-location-arrow active");
	document.getElementById("button-thre").setAttribute("class", "fa fa-line-chart");
	document.getElementById("setting-icon").setAttribute("class", "fa fa-location-arrow fa-5x");
	document.getElementById("setting-title").innerHTML = '<b>Location</b>';
	document.getElementById("setting-content").innerHTML = 
							'<p>' +
								'You can modify device location at this page.' +
							'</p>';
	document.getElementById("setting-main").innerHTML = setting_location;
	LoadLocationSetting();
	$(".submit-btn").bind("click", SubmitLocationSetting);
});

$(document).on("reload-setting-threshold", function()
{
	document.getElementById("button-dev").setAttribute("class", "fa fa-hdd-o");
	document.getElementById("button-loc").setAttribute("class", "fa fa-location-arrow");
	document.getElementById("button-thre").setAttribute("class", "fa fa-line-chart active");
	document.getElementById("setting-icon").setAttribute("class", "fa fa-line-chart fa-5x");
	document.getElementById("setting-title").innerHTML = '<b>Threshold</b>';
	document.getElementById("setting-content").innerHTML = 
							'<p>' +
								'You can modify threshold setting at this page.' +
							'</p>';
	document.getElementById("setting-main").innerHTML = setting_threshold;
	LoadThresholdSetting();
	$(".submit-btn").bind("click", SubmitThresholdSetting);
});

function TriggerDeviceSetting()
{
	$(document).trigger("reload-setting-device");
}

function TriggerLocationSetting()
{
	$(document).trigger("reload-setting-location");
}

function TriggerThresholdSetting()
{
	$(document).trigger("reload-setting-threshold");
}

function LoadDeviceSetting()
{
	var settings = {
		"url": "SettingAPI/GetDeviceSetting",
		"method": "GET",
		"cache": false
		}

	$.ajax(settings).done(function (response) {
		var obj = $.parseJSON(response);
		document.getElementById("devName").value = obj["DeviceName"];
		document.getElementById("aliastext").value = obj["Alias"];
		document.getElementById("ipaddr").value = obj["ServerIP"];
		document.getElementById("upintervaltext").value = obj["UploadInterval"];
		//document.getElementById("datalimittext").value = obj["DataLimitCount"];
		//document.getElementById("resendinterval").value = obj["EventResendInterval"];
	});
}

function SubmitDeviceSetting()
{
	var submit_settings = {
	"async": true,
	"crossDomain": true,
	"url": "SettingAPI/SetDeviceSetting",
	"method": "POST",
	"headers": {
			"Alias": document.getElementById("aliastext").value,
			"ServerIp": document.getElementById("ipaddr").value,
			"UploadInterval": document.getElementById("upintervaltext").value,
			//"DataLimitCount": document.getElementById("datalimittext").value,
			//"EventResendInterval": document.getElementById("resendinterval").value
		}
	}

	$.ajax(submit_settings).done(function (response){
		document.getElementById("popmsg").style = "";
		$.cookie("upinterval", document.getElementById("upintervaltext").value);
	});
};

function LoadLocationSetting()
{
	var settings = {
		"url": "SettingAPI/GetDeviceLocation",
		"method": "GET",
		"cache": false
	}

	$.ajax(settings).done(function (response) {
		var obj = $.parseJSON(response);
		document.getElementById("LongitudeId").value = obj[0]["Longitude"];
		document.getElementById("LatitudeId").value = obj[0]["Latitude"];
	});
};

function SubmitLocationSetting()
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

function LoadThresholdSetting()
{
	var threshold = {
		"url": "SettingAPI/GetThresholdSetting",
		"method": "GET",
		"cache": false
	}

	$.ajax(threshold).done(function (response) {
		var obj = $.parseJSON(response);
		var item_div = document.getElementById("threshold-item");
		var length = obj.length;
		for(var i = 0; i < length; i++)
		{
				var root_div = document.createElement("div");
				//Checkbox
				var checkbox = document.createElement("input");
				checkbox.setAttribute("type", "checkbox");
				checkbox.setAttribute("id", "box" + i);
				checkbox.checked = obj[i]["Enable"];
				root_div.appendChild(checkbox);

				//Enable label
				var label_enable = document.createElement("label");
				label_enable.appendChild(document.createTextNode("Enable"));
				label_enable.setAttribute("data-user", obj[i]["Id"]);
				label_enable.setAttribute("for", "box" + i);
				root_div.appendChild(label_enable);

				//Title label
				var label_title = document.createElement("label");
				label_title.appendChild(document.createTextNode(obj[i]["Name"]));
				root_div.appendChild(label_title);

				//Options
				var div_options = document.createElement("div");
				var select_options = document.createElement("select");
				option_text.forEach(function(element, idx, array){
					var func_option = document.createElement("option");
                    func_option.value = idx;
                    func_option.innerHTML = element;
                    if(obj[i]["Func"] == idx)
                    {
                        func_option.selected = "selected";
                    }
                    select_options.appendChild(func_option);
				});
				div_options.appendChild(select_options);
				div_options.setAttribute("class", "col-md-12 pad0");
				root_div.appendChild(div_options);

				//Text box
				var div_text = document.createElement("div");
				var input_text = document.createElement("input");
				input_text.value = obj[i]["Value"];
				input_text.setAttribute("class", "form-control");
				div_text.appendChild(input_text);
				var hr_text = document.createElement("hr");
				div_text.appendChild(hr_text);
				div_text.setAttribute("class", "col-md-12 pad0");
				root_div.appendChild(div_text);

				root_div.setAttribute("class", "col-md-6");
				item_div.appendChild(root_div);
		}
	});
};

function SubmitThresholdSetting()
{       
        var obj = document.getElementById("threshold-item").children;
        var i;
        for(var i = 0; i < obj.length; i++)
        {       
                var obj_child = obj[i].children;
                var obj_select = obj_child[3].children[0];
                var obj_text = obj_child[4].children[0];
                
                var settings = {
                  "async": true, 
                  "crossDomain": true,
                  "url": "SettingAPI/SetThreshold",
                  "method": "POST",
                  "headers": {
                    "Id": parseInt(obj_child[1].getAttribute("data-user")),
                    "Func": parseInt(obj_select.selectedIndex),
                    "Enable": (obj_child[0].checked?1:0),
                    "Value": parseFloat(obj_text.value),
                  },
                }
                
                $.ajax(settings).done(function (response) { 
                  document.getElementById("popmsg").style = "";
                });
        }
};
