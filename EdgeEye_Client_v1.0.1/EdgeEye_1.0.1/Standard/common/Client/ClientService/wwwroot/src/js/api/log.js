var settings = {
	"url": "LogAPI/GetDataLog?count=100",
	"method": "GET",
	"cache": false
}

function formatDate(date) {
	var day = date.getDate();
	var monthIndex = date.getMonth() + 1;
	var year = date.getFullYear();
	var hour = date.getHours();
	var min = date.getMinutes();

	return year + '-' + monthIndex + '-' + day + ' ' + hour + ':' + min;
}

$(document).ready(function () {
	//$(document).trigger("reload-logdata");
});


$(document).on("reload-logdata", function () {

	var event_table = $('#log-table').DataTable({
		"info": false,
		"columnDefs": [
			{
				"targets": -1,
				"data": null,
				"render": function (data, type, row, meta) {

					if (data[3]) {
						return `<button class="btn btn-mini btn-dark" disabled="disabled" style="background: #fff;color: #5E676C;border: 1px solid #5E676C;">Solve</button>`;
					}
					else {
						return `<button class="btn btn-mini btn-dark">Solve</button>`;
					}
				}
			}]
	});



	$.ajax(settings).done(function (response) {
		try {
			var obj = $.parseJSON(response);
			var length = obj.length;
			for (var i = 0; i < length; i++) {
				event_table.row.add(
					[
						obj[i]["Id"],
						formatDate(new Date(obj[i]["Time"] * 1000)),
						obj[i]["Message"],
						obj[i]["Checked"]
					]
				).draw(false);
			}
		}
		catch (x) {

		}

	});

	$('#log-table tbody').on('click', 'button', function (e) {

		var data = event_table.row($(this).parents('tr')).data();
		$.ajax({
			"url": "LogAPI/CheckLog",
			"method": "PUT",
			"headers": {
				"LogId": data[0]
			}
		}).done(function (response) {
			console.log('Do something...');
			e.target.style = "background: #fff;color: #5E676C;border: 1px solid #5E676C";
			e.target.disabled = true;
			$(document).trigger("myCustomEvent");
		});
	});
});
