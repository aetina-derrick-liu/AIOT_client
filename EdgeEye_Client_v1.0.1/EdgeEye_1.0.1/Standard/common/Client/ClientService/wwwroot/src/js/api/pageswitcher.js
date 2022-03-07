import "../style";
import dashboard from "../../dashboard.html";
import logs from "../../logs.html";
import setting from "../../setting.html";
import logo_icap from "../../images/logo-edgeeye-nav.png";
import logo_icap_nav from "../../images/logo-edgeeye.png";
import logo_inno from "../../images/logo-aetina.png";
import logo_home from "../../images/icons/home-active.png";
import logo_remote from "../../images/icons/remote-active.png";
import logo_device from "../../images/icons/device-active.png";
require("./dashboard");
require("./log");
require("./setting");

function formatDate(date) {
	var day = date.getDate();
	var monthIndex = date.getMonth() + 1;
	var year = date.getFullYear();
	var hour = date.getHours();
	var min = date.getMinutes();

	return year + '-' + monthIndex + '-' + day + ' ' + hour + ':' + min;
}

$(document).on("myCustomEvent", function () {
	$.ajax({
		type: 'GET',
		url: 'LogAPI/GetUnreadLogCount',
		cache: false,
		success: function (response) {
			if (parseInt(response) > 9) {
				document.getElementById("badgeitem").innerHTML = "9+";
			}
			else {
				document.getElementById("badgeitem").innerHTML = response;
			}
		},
		error: function (response) {
			//if(response.status == 504)
			{
				$(document).trigger("service-unavailable");
			}
		}
	});

	$.ajax({
		type: 'GET',
		url: 'LogAPI/GetUnreadLogBaner',
		cache: false,
		success: function (response) {
			try {
				var obj = $.parseJSON(response);
				var unread_ul = document.getElementById("unreadevent");
				var length = obj.length;
				while (unread_ul.firstChild) {
					unread_ul.removeChild(unread_ul.firstChild);
				};
				for (var i = 0; i < length; i++) {
					var root_li = document.createElement("li");
					var child_a = document.createElement("a");
					child_a.setAttribute("class", "tet01 lineH15 logs-btn");
					child_a.setAttribute("style", "cursor: pointer;");
					$(child_a).bind("click", SwitchToLog);
					var msg_span = document.createElement("span");
					msg_span.setAttribute("class", "span-left font14 t02 pad0 fontMW");
					msg_span.appendChild(document.createTextNode(obj[i]["Message"]));
					child_a.appendChild(msg_span);


					var time_span = document.createElement("span");
					time_span.setAttribute("class", "span-right font12 pad0");
					var time_icon = document.createElement("i");
					time_icon.setAttribute("class", "fa fa-clock-o");
					time_span.appendChild(time_icon);
					time_span.appendChild(document.createTextNode(formatDate(new Date(obj[i]["Time"] * 1000))));
					child_a.appendChild(time_span);

					root_li.appendChild(child_a);
					unread_ul.appendChild(root_li);
				}
			}
			catch (e) {
				var unread_ul = document.getElementById("unreadevent");
				while (unread_ul.firstChild) {
					unread_ul.removeChild(unread_ul.firstChild);
				};
				var root_p = document.createElement("p");
				root_p.setAttribute("class", "text-center");
				root_p.textContent = "No new event.";
				unread_ul.appendChild(root_p);
			}
		},
		error: function (response) {
			//if(response.status == 504)
			{
				$(document).trigger("service-unavailable");
			}
		}
	});
});

$(document).ready(function () {
	$.cookie("current_page", "dashboard");
	document.getElementById("logo_icap").src = logo_icap;
	document.getElementById("logo_icap_nav").src = logo_icap_nav;
	document.getElementById("logo_inno").src = logo_inno;
	document.getElementById("logo_home").src = logo_home;
	document.getElementById("logo_remote").src = logo_remote;
	document.getElementById("logo_device").src = logo_device;
	document.getElementById("main").innerHTML = dashboard;
	$(document).trigger("myCustomEvent");
	startRefresh();
	$(".home-btn").bind("click", SwitchToDashboard);
	$(".logs-btn").bind("click", SwitchToLog);
	$(".setting-btn").bind("click", SwitchToSettingDev);
	$("#setting-device").bind("click", SwitchToSettingDev);
	$("#setting-loc").bind("click", SwitchToSettingLoc);
	$("#setting-threshold").bind("click", SwitchToSettingThreshold);
	SwitchToDashboard();
});

function SwitchToDashboard() {
	$.cookie("current_page", "dashboard");
	document.getElementById("main").innerHTML = dashboard;
	$(document).trigger("reload-dashboard");
}

function SwitchToLog() {
	$.cookie("current_page", "log");
	document.getElementById("main").innerHTML = logs;
	$(document).trigger("reload-logdata");
}

function SwitchToSettingDev() {
	$.cookie("current_page", "settingdev");
	document.getElementById("main").innerHTML = setting;
	$(document).trigger("reload-setting-device");
	$(document).trigger("binding-setting-btn");
}

function SwitchToSettingLoc() {
	$.cookie("current_page", "settingloc");
	document.getElementById("main").innerHTML = setting;
	$(document).trigger("reload-setting-location");
	$(document).trigger("binding-setting-btn");
}

function SwitchToSettingThreshold() {
	$.cookie("current_page", "settingthr");
	document.getElementById("main").innerHTML = setting;
	$(document).trigger("reload-setting-threshold");
	$(document).trigger("binding-setting-btn");
}

function startRefresh() {
	var current_page = $.cookie("current_page");
	if (current_page != "serviceunavailable") {
		setTimeout(startRefresh, 30000);
		$(document).trigger("myCustomEvent");
	}
}