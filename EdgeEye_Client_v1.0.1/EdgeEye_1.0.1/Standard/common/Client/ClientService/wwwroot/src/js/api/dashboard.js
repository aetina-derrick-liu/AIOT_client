import "../jquery-jvectormap-world-mill";
require("../color_map");

var temp_chart;
var stor_chart;
var timer;

var settings = {
	"url": "DashboardAPI/Get",
	"method": "GET",
	"cache": false
}

function renderTemperature(DataTime, loading) {
	var DateTimeObject = [];
	var DataSeries = [];
	var max = 0, min = 500;

	Chart.defaults.global.defaultFontColor = "rgba(255,255,255,0.3)";

	for(var i = 0; i < DataTime.length; i++)
	{
		DateTimeObject[i] = DataTime[i] * 1000;
	}

	loading.forEach(function(element, idx, array)
	{
		DataSeries[idx] = {
			label: 'Storage' + idx,
			borderColor: color_def[array.length - 1][idx],
			pointColor: color_def[array.length - 1][idx],
			data: element
		};
		element.forEach(sub_element => {
				if(max < sub_element)
			{
				max = sub_element;
			}
			if(min > sub_element)
			{
				min = sub_element;
			}
		});
	});

	var config = {
		type: 'line',
		data: {
			labels: DateTimeObject,
			datasets: DataSeries,
		},
		options: {
			responsive: true,
			maintainAspectRatio: false,
			legend: {
				display: true,
			},
			scales: {
				xAxes: [{
					type: 'time',
					time: {
						displayFormats: {
							second: 'HH:mm:ss',
							minute: 'MM-DD HH:mm',
							hour: 'MM-DD HH:mm',
							day: 'MM-DD HH:mm',
							month: 'MM-DD HH:mm',
							quater: 'MM-DD HH:mm',
							year: 'MM-DD HH:mm'
						}
					},
					position: 'bottom',
					scaleLabel: {
						display: false,
						labelString: 'Time'
					}
				}],
				yAxes: [{
					type: 'linear',
					position: 'bottom',
					scaleLabel: {
						display: false,
						labelString: 'Temperature (°C)'
					},
					ticks: {
						min: min - 0.5,
						max: max + 0.5
					}
				}]
			}
		},
		plugins: [
			{
				afterDatasetsDraw: function(chart) {
					var ctx = chart.ctx;
					chart.data.datasets.forEach(function(dataset, i) {
						var meta = chart.getDatasetMeta(i);
						if (!meta.hidden) {
							meta.data.forEach(function(element, index) {
								// Draw the text in black, with the specified font
								ctx.fillStyle = 'rgb(255, 255, 255)';
								var fontSize = 12;
								var fontStyle = 'normal';
								var fontFamily = 'Helvetica Neue';
								ctx.font = Chart.helpers.fontString(fontSize, fontStyle, fontFamily);
								// Just naively convert to string for now
								var dataString = dataset.data[index].toString();
								// Make sure alignment settings are correct
								ctx.textAlign = 'center';
								ctx.textBaseline = 'middle';
								var padding = 5;
								var position = element.tooltipPosition();
								ctx.fillText(dataString, position.x, position.y - (fontSize / 2) - padding);
							});
						}
					});
				}
			}
		]
	};

	$("#Temperature").empty();

	var chart_dom = document.getElementById('Temperature');

	var ctx = document.createElement("canvas");

	temp_chart = new Chart(ctx, config);

	chart_dom.appendChild(ctx);
}

function updateTemperature(DataTime, loading)
{
	var DateTimeObject = [];
	var DataSeries = [];
	var max = 0, min = 500;

	Chart.defaults.global.defaultFontColor = "rgba(255,255,255,0.3)";

	for(var i = 0; i < DataTime.length; i++)
	{
		DateTimeObject[i] = DataTime[i] * 1000;
	}

	loading.forEach(function(element, idx, array)
	{
		DataSeries[idx] = {
			label: 'Storage' + idx,
			borderColor: color_def[array.length - 1][idx],
			pointColor: color_def[array.length - 1][idx],
			data: element
		};
		element.forEach(sub_element => {
				if(max < sub_element)
			{
				max = sub_element;
			}
			if(min > sub_element)
			{
				min = sub_element;
			}
		});
	});

	temp_chart.data.datasets = DataSeries;
	temp_chart.data.labels = DateTimeObject;
	temp_chart.options.animation.duration = 0;
	temp_chart.options.scales.yAxes[0].ticks.max = max + 0.5;
	temp_chart.options.scales.yAxes[0].ticks.min = min - 0.5;
	temp_chart.update();
}

function renderLifespan(days) 
{
	var DataLabels = [];
	var LifespanLabel = [];

	if(days.length > 0 && days[0] != undefined)
	{
		for(var i = 0; i < days.length; i++)
		{
			DataLabels[i] = "Storage " + i;
			if(days[i])
			{
				LifespanLabel[i] = days[i].toString();
			}
			else
			{
				LifespanLabel[i] = "n/a";
			}
		}

		var config = {
			type: 'bar',
			data: {
				labels: DataLabels,
				datasets: [
					{
						label: "Estimated days",
						backgroundColor: color_def[days.length - 1],
						data: days
					}
				]
			},
			options: {
				legend: { display: false },
				maintainAspectRatio: false,
				title: {
					display: false
				},
				scales:{
					xAxes: [{
						scaleLabel: {
							display: false,
							labelString: 'Estimated days'
						},
						ticks: {min: 0}
					}],
					yAxes: [{
						barThickness : 20
					}]
				}
			},
			plugins: [
				{
					afterDatasetsDraw: function(chart) {
						var ctx = chart.ctx;
						chart.data.datasets.forEach(function(dataset, i) {
							var meta = chart.getDatasetMeta(i);
							if (!meta.hidden) {
								meta.data.forEach(function(element, index) {
									// Draw the text in black, with the specified font
									ctx.fillStyle = 'rgb(255, 255, 255)';
									var fontSize = 12;
									var fontStyle = 'normal';
									var fontFamily = 'Helvetica Neue';
									ctx.font = Chart.helpers.fontString(fontSize, fontStyle, fontFamily);
									// Just naively convert to string for now
									var dataString = LifespanLabel[index];
									// Make sure alignment settings are correct
									ctx.textAlign = 'center';
									ctx.textBaseline = 'middle';
									var padding = 5;
									var position = element.tooltipPosition();
									ctx.fillText(dataString, position.x, position.y - (fontSize / 2) - padding);
								});
							}
						});
					}
				}
			]
		};

		$("#Lifespan").empty();

		var chart_dom = document.getElementById('Lifespan');

		var ctx = document.createElement("canvas");

		stor_chart = new Chart(ctx, config);

		chart_dom.appendChild(ctx);
	}
	else
	{
		$("#Lifespan").empty();

		var chart_dom = document.getElementById('Lifespan');

		var ctx = document.createElement("div");
		ctx.textContent = "There is no contain any lifespan data.";
		ctx.setAttribute("class", "barNoData font26");

		chart_dom.appendChild(ctx);
	}
}

function updateLifespan(days)
{
	var DataLabels = [];
	var LifespanLabel = [];

	if(days.length > 0 && days[0] != undefined)
	{
		if(stor_chart == null)
		{
			renderLifespan(days)
		}
		else
		{
			for(var i = 0; i < days.length; i++)
			{
				DataLabels[i] = "Storage " + i;
				if(days[i])
				{
					LifespanLabel[i] = days[i].toString();
				}
				else
				{
					LifespanLabel[i] = "n/a";
				}
			}
			stor_chart.data.labels = DataLabels;
			stor_chart.data.datasets[0].backgroundColor = color_def[days.length - 1];
			stor_chart.data.datasets[0].data = days;
			stor_chart.options.animation.duration = 0;
			stor_chart.update();
		}
	}
}

$(document).on("reload-dashboard", function()
{
	var settings = {
		"url": "SettingAPI/GetDeviceSetting",
		"method": "GET",
		"cache": false
		}

	$.ajax(settings).done(function (response) {
		var obj = $.parseJSON(response);
		var upload_interval = obj["UploadInterval"];
		$.cookie("upinterval", upload_interval);
		ReloadDashboard();
		timer = setTimeout(startRefresh, upload_interval * 1000);
	});
});

function startRefresh() {
	var current_page = $.cookie("current_page");
	var upload_interval = $.cookie("upinterval");
	if(current_page == "dashboard")
	{
		timer = setTimeout(startRefresh, upload_interval * 1000);
		//$(document).trigger("reload-dashboard-auto");
		reloadDashboard();
	}
	else
	{
		clearTimeout(timer);
	}
}

function ReloadDashboard()
{
	$.ajax(settings).done(function (response) {
		var showName;
		var obj = $.parseJSON(response);

		if(obj["ServiceStatus"] == 0)
		{
			document.getElementById("status-led").setAttribute("class", "led-success");
		}
		else if(obj["ServiceStatus"] == 4)
		{
			document.getElementById("status-led").setAttribute("class", "led-danger");
		}
		else
		{
			document.getElementById("status-led").setAttribute("class", "led-warning");
		}
		document.getElementById("serverip").innerHTML = obj["ServerIP"];

		if(obj["Alias"].length > 0)
		{
			document.getElementById("DevNameLabel").innerHTML = "Device Alias";
			document.getElementById("name").setAttribute("class", "font18");
			document.getElementById("name").innerHTML = '<b>' + obj["Alias"] + '</b>';
			showName = obj["Alias"];
		}
		else
		{
			document.getElementById("DevNameLabel").innerHTML = "Device Name";
			document.getElementById("name").setAttribute("class", "font18");
			document.getElementById("name").innerHTML = '<b>' + obj["Name"] + '</b>';
			showName = obj["Name"];
		}	
		
		document.getElementById("CPULoading").innerHTML = Math.floor(obj["CPULoading"] * 10) / 10;
		document.getElementById("MEMLoading").innerHTML = Math.floor(obj["MEMLoading"] * 10) / 10;

		var temp_obj = [,];
		var life_obj = []

		var stor_info_table = document.getElementById("storage-info-table");

		$("#storage-info-table").empty();
		
		var tr_obj_title = document.createElement("tr");
		tr_obj_title.setAttribute("class", "tabcon active");
		var td_index = document.createElement("td");
		td_index.innerHTML = '<b>Serial Number</b>';
		tr_obj_title.appendChild(td_index);
		var td_temp = document.createElement("td");
		td_temp.innerHTML = '<b>Temp</b>';
		tr_obj_title.appendChild(td_temp);
		var td_health = document.createElement("td");
		td_health.innerHTML = '<b>Health</b>';
		tr_obj_title.appendChild(td_health);
		var td_pecycle = document.createElement("td");
		td_pecycle.innerHTML = '<b>Wear Cycle</b>';
		tr_obj_title.appendChild(td_pecycle);
		var td_lifespan = document.createElement("td");
		td_lifespan.innerHTML = '<b>Lifespan</b>';
		tr_obj_title.appendChild(td_lifespan);
		stor_info_table.appendChild(tr_obj_title);

		for(var i = 0; i < obj["StorList"].length; i++)
		{
			var temp_data_length = obj["StorList"][i]["Temperature"].length;
			var life_data_length = obj["StorList"][i]["Lifespan"]["Days"].length;
			temp_obj[i] = obj["StorList"][i]["Temperature"];
			life_obj[i] = obj["StorList"][i]["Lifespan"]["Days"][life_data_length - 1];

			var tr_obj = document.createElement("tr");
			tr_obj.setAttribute("class", "tabcon active");
			var td_index = document.createElement("td");
			td_index.innerHTML = obj["StorList"][i]["SN"];
			tr_obj.appendChild(td_index);
			var td_temp = document.createElement("td");
			td_temp.innerHTML = obj["StorList"][i]["Temperature"][temp_data_length - 1];
			tr_obj.appendChild(td_temp);
			var td_health = document.createElement("td");
			td_health.innerHTML = obj["StorList"][i]["Health"];
			tr_obj.appendChild(td_health);
			var td_pecycle = document.createElement("td");
			td_pecycle.innerHTML = obj["StorList"][i]["AvgEraseCount"] + "/" + obj["StorList"][i]["PECycle"];
			tr_obj.appendChild(td_pecycle);
			var td_lifespan = document.createElement("td");
			if(obj["StorList"][i]["Lifespan"]["Days"][life_data_length - 1] > 0)
			{
				td_lifespan.innerHTML = obj["StorList"][i]["Lifespan"]["Days"][life_data_length - 1];
			}
			else
			{
				td_lifespan.innerHTML = "n/a";
			}
			tr_obj.appendChild(td_lifespan);
			stor_info_table.appendChild(tr_obj);
		}

		renderTemperature(obj["RawDataTime"], temp_obj);

		renderLifespan(life_obj);

		var map = $('#world-map').vectorMap({
			map: 'world_mill',
			markerStyle: {
		      initial: {
		        fill: '#00FF00',
		        stroke: '#383f47'
		      }
		    },
			normalizeFunction: 'polynomial',
			markers: [
      			{
					latLng: [
						obj["Location"][0]["Latitude"], 
						obj["Location"][0]["Longitude"]
					], 
					name: showName
				}
      		]
		});
	});
}

//$(document).on("reload-dashboard-auto", function()
function reloadDashboard()
{
	$.ajax(settings).done(function (response) {
		var showName;
		var obj = $.parseJSON(response);

		if(obj["ServiceStatus"] == 0)
		{
			document.getElementById("status-led").setAttribute("class", "led-success");
		}
		else if(obj["ServiceStatus"] == 4)
		{
			document.getElementById("status-led").setAttribute("class", "led-danger");
		}
		else
		{
			document.getElementById("status-led").setAttribute("class", "led-warning");
		}
		document.getElementById("serverip").innerHTML = obj["ServerIP"];

		if(obj["Alias"].length > 0)
		{
			document.getElementById("DevNameLabel").innerHTML = "Device Alias";
			document.getElementById("name").innerHTML = '<b>' + obj["Alias"] + '</b>';
			showName = obj["Alias"];
		}
		else
		{
			document.getElementById("DevNameLabel").innerHTML = "Device Name";
			document.getElementById("name").innerHTML = '<b>' + obj["Name"] + '</b>';
			showName = obj["Name"];
		}	
		
		document.getElementById("CPULoading").innerHTML = Math.floor(obj["CPULoading"] * 10) / 10;
		document.getElementById("MEMLoading").innerHTML = Math.floor(obj["MEMLoading"] * 10) / 10;

		var temp_obj = [,];
		var life_obj = []

		var stor_info_table = document.getElementById("storage-info-table");

		$("#storage-info-table").empty();
		
		var tr_obj_title = document.createElement("tr");
		tr_obj_title.setAttribute("class", "tabcon active");
		var td_index = document.createElement("td");
		td_index.innerHTML = '<b>Index</b>';
		tr_obj_title.appendChild(td_index);
		var td_temp = document.createElement("td");
		td_temp.innerHTML = '<b>Temp</b>';
		tr_obj_title.appendChild(td_temp);
		var td_health = document.createElement("td");
		td_health.innerHTML = '<b>Health</b>';
		tr_obj_title.appendChild(td_health);
		var td_pecycle = document.createElement("td");
		td_pecycle.innerHTML = '<b>SSD Wear Cycle</b>';
		tr_obj_title.appendChild(td_pecycle);
		var td_lifespan = document.createElement("td");
		td_lifespan.innerHTML = '<b>Lifespan</b>';
		tr_obj_title.appendChild(td_lifespan);
		stor_info_table.appendChild(tr_obj_title);

		for(var i = 0; i < obj["StorList"].length; i++)
		{
			var temp_data_length = obj["StorList"][i]["Temperature"].length;
			var life_data_length = obj["StorList"][i]["Lifespan"]["Days"].length;
			temp_obj[i] = obj["StorList"][i]["Temperature"];
			life_obj[i] = obj["StorList"][i]["Lifespan"]["Days"][life_data_length - 1];

			var tr_obj = document.createElement("tr");
			tr_obj.setAttribute("class", "tabcon active");
			var td_index = document.createElement("td");
			td_index.innerHTML = obj["StorList"][i]["SN"];
			tr_obj.appendChild(td_index);
			var td_temp = document.createElement("td");
			td_temp.innerHTML = obj["StorList"][i]["Temperature"][temp_data_length - 1];
			tr_obj.appendChild(td_temp);
			var td_health = document.createElement("td");
			td_health.innerHTML = obj["StorList"][i]["Health"];
			tr_obj.appendChild(td_health);
			var td_pecycle = document.createElement("td");
			td_pecycle.innerHTML = obj["StorList"][i]["AvgEraseCount"] + "/" + obj["StorList"][i]["PECycle"];
			tr_obj.appendChild(td_pecycle);
			var td_lifespan = document.createElement("td");
			if(obj["StorList"][i]["Lifespan"]["Days"][life_data_length - 1] > 0)
			{
				td_lifespan.innerHTML = obj["StorList"][i]["Lifespan"]["Days"][life_data_length - 1];
			}
			else
			{
				td_lifespan.innerHTML = "n/a";
			}
			tr_obj.appendChild(td_lifespan);
			stor_info_table.appendChild(tr_obj);
		}

		updateTemperature(obj["RawDataTime"], temp_obj);

		updateLifespan(life_obj);
	});
}//);